#include "ga.h"

GA::GA(const GA& other)
{
    mParameters = other.mParameters;
}

GA& GA::operator=(const GA& other)
{
    mParameters = other.mParameters;
    return *this;
}

GA::GA(GAParams parameters)
{
    mParameters = parameters;
}

vector<unsigned int> GA::getNumWeights()
{
    vector<unsigned int> counters;
    for(unsigned int k = 0; k < mParameters.nnParameters.size(); k++)
        counters.push_back((mParameters.nnParameters[k].hiddenNodes > 0) ? mParameters.nnParameters[k].hiddenNodes * 
        (mParameters.nnParameters[k].inputNodes + mParameters.nnParameters[k].outputNodes + 1) + mParameters.nnParameters[k].outputNodes
        : mParameters.nnParameters[k].outputNodes * (mParameters.nnParameters[k].inputNodes + 1));

    return counters;
}

vector<Chromosome> GA::initializePopulation(Simulation* simulation)
{
    vector<Chromosome> population;

    boost::mt19937 rng(rand());
    boost::uniform_real<float> initWeightDist(mParameters.searchSpaceMin, mParameters.searchSpaceMax);
    boost::variate_generator<boost::mt19937, boost::uniform_real<float>> genInitPos(rng, initWeightDist);

    vector<unsigned int> numWeights = getNumWeights();

    for(unsigned int k = 0; k < mParameters.GApopulation; k++)
    {
        vector<vector<float>> currChromosomeWeights;
        for(unsigned int l = 0; l < numWeights.size(); l++)
        {
            vector<float> currnetweights;
            for(unsigned int i = 0; i < numWeights[l]; i++)
                currnetweights.push_back(genInitPos());
            currChromosomeWeights.push_back(currnetweights);
        }

        Chromosome currChromosome(mParameters.nnParameters);
        currChromosome.mWeights = currChromosomeWeights;
        currChromosome.mFitness = simulation->parameters.maxFitness;

        population.push_back(currChromosome);
    }

    return population;
}

//selects parent according to rank
//assumes a sorted population
Chromosome GA::selectParent(vector<Chromosome> population, unsigned int& rank)
{
    unsigned int max = 0;
    for(unsigned int k = 1; k <= population.size(); k++)
        max += k;

    boost::mt19937 rankRng(rand());
    boost::uniform_int<> rankDist(1, max);
    boost::variate_generator<boost::mt19937, boost::uniform_int<>> genSelection(rankRng, rankDist);

    int selection = genSelection();

    unsigned int k = -1;
    while(selection > 0)
        selection -= population.size() - ++k;

    assert(k >= 0);
    assert(k < population.size());

    rank = k;
    return population[k];
}

vector<Chromosome> GA::getParents(vector<Chromosome> population, unsigned int numParents)
{
    assert(numParents <= population.size());
    vector<Chromosome> parents;

    while(parents.size() < numParents)
    {
        unsigned int rank;
        parents.push_back(selectParent(population, rank));
        population.erase(population.begin() + rank);
    }
    assert(parents.size() == numParents);
    return parents;
}

vector<Chromosome> GA::gaussianCrossover(vector<Chromosome> population, float maxFitness)
{
    vector<Chromosome> output;

    vector<Chromosome> parents = getParents(population, 2);

    Chromosome child(mParameters.nnParameters);
    
    boost::mt19937 rng(rand());
    
    vector<vector<float>> weights;
    for(unsigned int k = 0; k < parents[0].mWeights.size(); k++)
    {
        vector<float> currNetWeights;
        for(unsigned int i = 0; i < parents[0].mWeights[k].size(); i++)
        {
            boost::normal_distribution<> normDist((parents[0].mWeights[k][i] + parents[1].mWeights[k][i]) / 2 , fabs(parents[0].mWeights[k][i] - parents[1].mWeights[k][i]));
	        boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > genCrossover(rng, normDist);
            
            float val = genCrossover();
            currNetWeights.push_back(val);
        }
        weights.push_back(currNetWeights);
    }

    child.mWeights = weights;
    child.mFitness = maxFitness;

    output.push_back(child);
    return output;
}

vector<Chromosome> GA::multipointCrossover(vector<Chromosome> population, float maxFitness)
{
    vector<Chromosome> output;

    vector<Chromosome> parents = getParents(population, 2);

    Chromosome child(mParameters.nnParameters);
    
    boost::mt19937 rng(rand());
    boost::uniform_int<> multipointDist(0, 1);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<> > genCrossover(rng, multipointDist);
    
    vector<vector<float>> weights;
    for(unsigned int k = 0; k < parents[0].mWeights.size(); k++)
    {
        vector<float> currNetWeights;
        for(unsigned int i = 0; i < parents[0].mWeights[k].size(); i++)
            currNetWeights.push_back(genCrossover() == 0? parents[0].mWeights[k][i] : parents[1].mWeights[k][i]);
        weights.push_back(currNetWeights);
    }

    child.mWeights = weights;
    child.mFitness = maxFitness;

    output.push_back(child);

    return output;
}

vector<Chromosome> GA::simplexCrossover(vector<Chromosome> population, float maxFitness)
{
    vector<Chromosome> output;
    vector<Chromosome> parents = getParents(population, 4);

    Chromosome child(mParameters.nnParameters);

    quicksort(parents, 0, parents.size() - 1);

    vector<vector<float>> weights;
    for(unsigned int k = 0; k < parents[0].mWeights.size(); k++)
    {
        vector<float> com;
        vector<float> childWeights;
        
        for(unsigned int l = 0; l < parents[0].mWeights[k].size(); l++)
        {
            com.push_back(0.0f);
            for(unsigned int i = 0; i < parents.size() - 1; i++)
                com[l] += parents[i].mWeights[k][l];

            com[l] /= (parents.size() - 1);
            childWeights.push_back(com[l] + (parents[0].mWeights[k][l] - parents[parents.size() - 1].mWeights[k][l]));
        }

        weights.push_back(childWeights);
    }
    
    child.mWeights = weights;
    child.mFitness = maxFitness;

    output.push_back(child);
    return output;
}

vector<Chromosome> GA::singlepointCrossover(vector<Chromosome> population, float maxFitness)
{
    vector<Chromosome> output;

    vector<Chromosome> parents = getParents(population, 2);

    Chromosome child(mParameters.nnParameters);
    
    vector<vector<float>> weights;

    for(unsigned int k = 0; k < parents[0].mWeights.size(); k++)
    {
        vector<float> currNetWeights, p1, p2;
        unsigned int pos = rand() % parents[0].mWeights[k].size();

        p1 = parents[0].mWeights[k];
        p2 = parents[1].mWeights[k];

        currNetWeights.insert(currNetWeights.end(), p1.begin(), p1.begin() + pos);
        currNetWeights.insert(currNetWeights.end(), p2.begin() + pos, p2.end());

        weights.push_back(currNetWeights);
    }

    child.mWeights = weights;
    child.mFitness = maxFitness;

    output.push_back(child);

    return output;
}

vector<Chromosome> GA::twopointCrossover(vector<Chromosome> population, float maxFitness)
{
    vector<Chromosome> output;

    vector<Chromosome> parents = getParents(population, 2);

    Chromosome child(mParameters.nnParameters);
    
    vector<vector<float>> weights;

    for(unsigned int k = 0; k < parents[0].mWeights.size(); k++)
    {
        vector<float> currNetWeights, p1, p2;
        unsigned int pos1 = rand() % parents[0].mWeights[k].size();
        unsigned int pos2 = (rand() % (parents[0].mWeights[k].size() - pos1)) + pos1;

        p1 = parents[0].mWeights[k];
        p2 = parents[1].mWeights[k];

        currNetWeights.insert(currNetWeights.end(), p1.begin(), p1.begin() + pos1);
        currNetWeights.insert(currNetWeights.end(), p2.begin() + pos1, p2.begin() + pos2);
        currNetWeights.insert(currNetWeights.end(), p2.begin() + pos2, p2.end());

        weights.push_back(currNetWeights);
    }

    child.mWeights = weights;
    child.mFitness = maxFitness;

    output.push_back(child);

    return output;
}

vector<Chromosome> GA::simulatedbinaryCrossover(vector<Chromosome> population, float maxFitness)
{
    vector<Chromosome> output;
    
    vector<Chromosome> parents = getParents(population, 2);
    Chromosome child1(mParameters.nnParameters); 
    Chromosome child2(mParameters.nnParameters); 
    child1.mFitness = child2.mFitness = maxFitness;

    boost::mt19937 rng(rand());
    boost::uniform_real<float> unidist(0, 1);
	boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > genCrossover(rng, unidist);
    
    vector<vector<float>> weights1, weights2;

    for(unsigned int k = 0; k < parents[0].mWeights.size(); k++)
    {
        vector<float> currWeights1, currWeights2;
        for(unsigned int i = 0; i < parents[0].mWeights[k].size(); k++)
        {
            float random = genCrossover();
            float offset = random > 0.5f? pow(2 * random, 0.5f) : pow(1 / (2 * (1-random)), 0.5f);

            currWeights1.push_back(((1 + offset) * parents[0].mWeights[k][i] + (1 - offset) * parents[1].mWeights[k][i])/2);
            currWeights2.push_back(((1 - offset) * parents[0].mWeights[k][i] + (1 + offset) * parents[1].mWeights[k][i])/2);
        }
        weights1.push_back(currWeights1);
        weights2.push_back(currWeights2);
    }
        
    child1.mWeights = weights1;
    child2.mWeights = weights2;

    output.push_back(child1);
    output.push_back(child2);

    return output;
}

vector<Chromosome> GA::crossover(vector<Chromosome> population, float maxFitness)
{
    vector<Chromosome> output;

    switch (mParameters.crossoverType)
    {
        case GAUSSIAN_CO: 
            output = gaussianCrossover(population, maxFitness);
            break;
        case MULTIPOINT_CO: 
            output = multipointCrossover(population, maxFitness);
            break;
        case SIMPLEX_CO:
            output = simplexCrossover(population, maxFitness);
            break;
        case SINGLEPOINT_CO:
            output = singlepointCrossover(population, maxFitness);
            break;
        case TWOPOINT_CO:
            output = twopointCrossover(population, maxFitness);
            break;
        case SIMULATEDBINARY_CO:
            output = simulatedbinaryCrossover(population, maxFitness);
            break;
        default:
            break;
    }

    return output;
}

void GA::conformWeights(vector<Chromosome>& population)
{
    for(unsigned int k = 0; k < population.size(); k++)
    {
        for(unsigned int l = 0; l < population[k].mWeights.size(); l++)
        {
            for(unsigned int i = 0; i < population[k].mWeights[l].size(); i++)
            {
                if(population[k].mWeights[l][i] > mParameters.searchSpaceMax)
                    population[k].mWeights[l][i] = mParameters.searchSpaceMin + fmod(population[k].mWeights[l][i] - mParameters.searchSpaceMax, mParameters.searchSpaceMax - mParameters.searchSpaceMin);
                else if(population[k].mWeights[l][i] < mParameters.searchSpaceMin)
                    population[k].mWeights[l][i] = mParameters.searchSpaceMax - fmod(mParameters.searchSpaceMin - population[k].mWeights[l][i], mParameters.searchSpaceMax - mParameters.searchSpaceMin);
            }
        }
    }
}

void GA::mutate(vector<Chromosome>& population)
{
    assert(mParameters.mutationProb <= 1.0f && mParameters.mutationProb >= 0.0f);

    boost::mt19937 mutation(rand());

    boost::uniform_real<float> mutationProbDist(0, 1);
    boost::variate_generator<boost::mt19937, boost::uniform_real<float>> genMutationProb(mutation, mutationProbDist);

    float standardDeviation = (mParameters.searchSpaceMax - mParameters.searchSpaceMin) / 10;
    boost::normal_distribution<> normDist(0, standardDeviation);
	boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > genMutation(mutation, normDist);

    
    for(unsigned int i = mParameters.elitismCount; i < population.size(); i++)
    {
        for(unsigned int k = 0; k < population[i].mWeights.size(); k++)
        {
            for(unsigned int l = 0; l < population[i].mWeights[k].size(); l++)
            {
                if(genMutationProb() <= mParameters.mutationProb)
                    population[i].mWeights[k][l] += genMutation();
            }
        }
    }
}

vector<NeuralNetwork> GA::train(Simulation* simulation)
{
    assert(mParameters.GApopulation > mParameters.elitismCount); 

    cout << "Initialising" << endl;
    vector<Chromosome> population = initializePopulation(simulation);
    assert(population.size() > 0);

    cout << "Starting training: " << endl;

    for(unsigned int k = 0; k < mParameters.maxGenerations; k++)
    {
        cout << "Generation " << k << endl;
        evaluatePopulation(population, simulation);

        if(population[0].mFitness < mParameters.epsilon)
            return population[0].getBrains();

        vector<Chromosome> newPopulation = getFirst(population, mParameters.elitismCount);
        while(newPopulation.size() < population.size())
        {
            vector<Chromosome> offspring = crossover(population, simulation->parameters.maxFitness);
            newPopulation.insert(newPopulation.end(), offspring.begin(), offspring.end());
        }

        while(newPopulation.size() > population.size())
            newPopulation.pop_back();

        population = newPopulation;

        mutate(population);

        conformWeights(population);
    }
    evaluatePopulation(population, simulation);

    return population[0].getBrains();
}

vector<Chromosome> GA::getFirst(vector<Chromosome> population, unsigned int amount)
{
    vector<Chromosome> output;

    unsigned int stop = (amount > population.size()) ? population.size() : amount;
    output.insert(output.end(), population.begin(), population.begin() + stop);

    return output;
}

void GA::quicksort(vector<Chromosome>& elements, int left, int right)
{
	int i = left;
	int j = right;

	Chromosome pivot = elements[(left+ right) / 2];
	do
	{
		while (elements[i].mFitness < pivot.mFitness)
			i++;
		while (elements[j].mFitness > pivot.mFitness)
			j--;

		if (i <= j)
		{
			Chromosome temp = elements[i]; elements[i] = elements[j]; elements[j] = temp;
			i++; j--;
		}
	} while (i <= j);

	if (left < j)
		quicksort(elements, left, j);
	if (i < right)
		quicksort(elements, i, right);
}

void GA::evaluatePopulation(vector<Chromosome>& population, Simulation* simulation)
{
    for(unsigned int i = 0; i < mParameters.GApopulation; i++)
    {
        vector<NeuralNetwork> brains = population[i].getBrains();
        
        simulation->fullRun(brains);
        
        float fitness = simulation->evaluateFitness();
        cout << "Chromosome: " << i << " with fitness: " << fitness << endl;
        
        population[i].mFitness = fitness;
        
        simulation->reset();
        
        if(fitness < mParameters.epsilon)
            break;
    }

    quicksort(population, 0, population.size() - 1);
}