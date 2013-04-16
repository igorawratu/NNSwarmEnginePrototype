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

unsigned int GA::getNumWeights(unsigned int numInput, unsigned int numHidden, unsigned int numOutput)
{
    return (numHidden > 0) ? numHidden * (numInput + numOutput + 1) + numOutput : numOutput * (numInput + 1);
}

vector<Object*> GA::initializeModels(unsigned int initializationSeed)
{
    vector<Object*> objects;

    boost::mt19937 rngx(initializationSeed);
    boost::mt19937 rngy(initializationSeed * 2);
    boost::uniform_real<float> xDist(mParameters.modelInitSpaceMin.x, mParameters.modelInitSpaceMax.x);
    boost::uniform_real<float> yDist(mParameters.modelInitSpaceMin.y, mParameters.modelInitSpaceMax.y);
    boost::variate_generator<boost::mt19937, boost::uniform_real<float>> genx(rngx, xDist);
    boost::variate_generator<boost::mt19937, boost::uniform_real<float>> geny(rngy, yDist);

    for(unsigned int k = 0; k < mParameters.simulationPopulation; k++)
    {
        vector2 pos;
        pos.x = genx();
        pos.y = geny();
        objects.push_back(new Object(pos, mParameters.modelColour, mParameters.vMax, mParameters.vMin, mParameters.modelMoveSpaceMax, mParameters.modelMoveSpaceMin, false));
    }

    return objects;
}

void GA::cleanupModels(vector<Object*> models)
{
    for(unsigned int k = 0; k < models.size(); k++)
    {
        delete models[k];
        models[k] = 0;
    }
}

vector<NeuralNetwork> GA::initializePopulation()
{
    vector<NeuralNetwork> population;

    boost::mt19937 rng(rand());
    boost::uniform_real<float> initWeightDist(mParameters.searchSpaceMin, mParameters.searchSpaceMax);
    boost::variate_generator<boost::mt19937, boost::uniform_real<float>> genInitPos(rng, initWeightDist);

    unsigned int numWeights = getNumWeights(mParameters.nnInputs, mParameters.nnHiddens, mParameters.nnOutputs);

    for(unsigned int k = 0; k < mParameters.GApopulation; k++)
    {
        vector<float> currNetworkWeights;
        for(unsigned int i = 0; i < numWeights; i++)
            currNetworkWeights.push_back(genInitPos());
        NeuralNetwork currNetwork(mParameters.nnInputs, mParameters.nnOutputs, mParameters.nnHiddens);
        if(!currNetwork.setWeights(currNetworkWeights))
        {
            cout << "unable to set weights for a neural net, check calculations" << endl;
            return population;
        }
        currNetwork.setFitness(mParameters.maxFitness);
        population.push_back(currNetwork);
    }

    return population;
}

NeuralNetwork GA::selectParent(vector<NeuralNetwork> population, unsigned int& rank)
{
    unsigned int max = 0;
    for(int k = 1; k <= population.size(); k++)
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

vector<NeuralNetwork> GA::getParents(vector<NeuralNetwork> population, unsigned int numParents)
{
    assert(numParents <= population.size());
    vector<NeuralNetwork> parents;

    while(parents.size() < numParents)
    {
        unsigned int rank;
        parents.push_back(selectParent(population, rank));
        population.erase(population.begin() + rank);
    }
    assert(parents.size() == numParents);
    return parents;
}

vector<NeuralNetwork> GA::gaussianCrossover(vector<NeuralNetwork> population)
{
    vector<NeuralNetwork> output;

    vector<NeuralNetwork> parents = getParents(population, 2);

    NeuralNetwork child(mParameters.nnInputs, mParameters.nnOutputs, mParameters.nnHiddens);
    
    boost::mt19937 rng(rand());
    
    vector<float> weights;
    for(int k = 0; k < parents[0].getWeights().size(); k++)
    {
        boost::normal_distribution<> normDist((parents[0].getWeights()[k] + parents[1].getWeights()[k]) / 2 , fabs(parents[0].getWeights()[k] - parents[1].getWeights()[k]));
	    boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > genCrossover(rng, normDist);
        
        float val = genCrossover();
        weights.push_back(val);
    }

    child.setWeights(weights);
    child.setFitness(mParameters.maxFitness);

    output.push_back(child);
    return output;
}

vector<NeuralNetwork> GA::multipointCrossover(vector<NeuralNetwork> population)
{
    vector<NeuralNetwork> output;

    vector<NeuralNetwork> parents = getParents(population, 2);

    NeuralNetwork child(mParameters.nnInputs, mParameters.nnOutputs, mParameters.nnHiddens);
    
    boost::mt19937 rng(rand());
    
    vector<float> weights;
    for(int k = 0; k < parents[0].getWeights().size(); k++)
    {
        boost::uniform_int<> multipointDist(0, 1);
	    boost::variate_generator<boost::mt19937&, boost::uniform_int<> > genCrossover(rng, multipointDist);

        weights.push_back(genCrossover() == 0? parents[0].getWeights()[k] : parents[1].getWeights()[k]);
    }

    child.setWeights(weights);
    child.setFitness(mParameters.maxFitness);

    output.push_back(child);

    return output;
}

vector<NeuralNetwork> GA::simplexCrossover(vector<NeuralNetwork> population)
{
    vector<NeuralNetwork> output;
    vector<NeuralNetwork> parents = getParents(population, 4);

    NeuralNetwork child(mParameters.nnInputs, mParameters.nnOutputs, mParameters.nnHiddens);

    quicksort(parents, 0, parents.size() - 1);
    vector<float> com;
    vector<float> childWeights;
    for(int k = 0; k < parents[0].getWeights().size(); k++)
    {
        com.push_back(0.0f);
        for(int i = 0; i < parents.size() - 1; i++)
            com[k] += parents[i].getWeights()[k];

        com[k] /= (parents.size() - 1);
        childWeights.push_back(com[k] + (parents[0].getWeights()[k] - parents[parents.size() - 1].getWeights()[k]));
    }
    
    child.setWeights(childWeights);
    child.setFitness(mParameters.maxFitness);

    output.push_back(child);
    return output;
}

vector<NeuralNetwork> GA::singlepointCrossover(vector<NeuralNetwork> population)
{
    vector<NeuralNetwork> output;

    vector<NeuralNetwork> parents = getParents(population, 2);

    NeuralNetwork child(mParameters.nnInputs, mParameters.nnOutputs, mParameters.nnHiddens);
    
    vector<float> weights, p1, p2;
    unsigned int pos = rand() % parents[0].getWeights().size();

    p1 = parents[0].getWeights();
    p2 = parents[1].getWeights();

    weights.insert(weights.end(), p1.begin(), p1.begin() + pos);
    weights.insert(weights.end(), p2.begin() + pos, p2.end());

    assert(weights.size() == parents[0].getWeights().size());

    child.setWeights(weights);
    child.setFitness(mParameters.maxFitness);

    output.push_back(child);

    return output;
}

vector<NeuralNetwork> GA::twopointCrossover(vector<NeuralNetwork> population)
{
    vector<NeuralNetwork> output;

    vector<NeuralNetwork> parents = getParents(population, 2);

    NeuralNetwork child(mParameters.nnInputs, mParameters.nnOutputs, mParameters.nnHiddens);
    
    vector<float> weights, p1, p2;
    unsigned int pos1 = rand() % parents[0].getWeights().size();
    unsigned int pos2 = (rand() % (parents[0].getWeights().size() - pos1)) + pos1;

    p1 = parents[0].getWeights();
    p2 = parents[1].getWeights();

    weights.insert(weights.end(), p1.begin(), p1.begin() + pos1);
    weights.insert(weights.end(), p2.begin() + pos1, p2.begin() + pos2);
    weights.insert(weights.end(), p2.begin() + pos2, p2.end());

    assert(weights.size() == parents[0].getWeights().size());

    child.setWeights(weights);
    child.setFitness(mParameters.maxFitness);

    output.push_back(child);

    return output;
}

vector<NeuralNetwork> GA::simulatedbinaryCrossover(vector<NeuralNetwork> population)
{
    vector<NeuralNetwork> output;
    
    vector<NeuralNetwork> parents = getParents(population, 2);
    NeuralNetwork child1(mParameters.nnInputs, mParameters.nnOutputs, mParameters.nnHiddens); child1.setFitness(mParameters.maxFitness);
    NeuralNetwork child2(mParameters.nnInputs, mParameters.nnOutputs, mParameters.nnHiddens); child2.setFitness(mParameters.maxFitness);
    vector<float> weights1, weights2;

    boost::mt19937 rng(rand());
    boost::uniform_real<float> unidist(0, 1);
	boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > genCrossover(rng, unidist);

    for(int k = 0; k < parents[0].getWeights().size(); k++)
    {
        float random = genCrossover();
        float offset = random > 0.5f? pow(2 * random, 0.5f) : pow(1 / (2 * (1-random)), 0.5f);

        weights1.push_back(((1 + offset) * parents[0].getWeights()[k] + (1 - offset) * parents[1].getWeights()[k])/2);
        weights2.push_back(((1 - offset) * parents[0].getWeights()[k] + (1 + offset) * parents[1].getWeights()[k])/2);
    }
        
    child1.setWeights(weights1);
    child2.setWeights(weights2);

    output.push_back(child1);
    output.push_back(child2);
    return output;
}

vector<NeuralNetwork> GA::crossover(vector<NeuralNetwork> population)
{
    vector<NeuralNetwork> output;

    switch (mParameters.crossoverType)
    {
        case GAUSSIAN_CO: 
            output = gaussianCrossover(population);
            break;
        case MULTIPOINT_CO: 
            output = multipointCrossover(population);
            break;
        case SIMPLEX_CO:
            output = simplexCrossover(population);
            break;
        case SINGLEPOINT_CO:
            output = singlepointCrossover(population);
            break;
        case TWOPOINT_CO:
            output = twopointCrossover(population);
            break;
        case SIMULATEDBINARY_CO:
            output = simulatedbinaryCrossover(population);
            break;
        default:
            break;
    }

    return output;
}

float GA::calculateStandardDeviation(vector<NeuralNetwork> population, NeuralNetwork current, unsigned int position)
{
    float stdv = 0.0f, mean = 0.0f;

    for(int k = 0; k < population.size(); k++)
        mean += fabs(current.getWeights()[position] - population[k].getWeights()[position]);

    mean /= current.getWeights().size();
    
    for(int k = 0; k < population.size(); k++)
    {
        float curr = fabs(current.getWeights()[position] - population[k].getWeights()[position]) - mean;
        stdv += curr * curr;
    }

    stdv /= population.size() - 1;
    
    return sqrt(stdv);
}

void GA::conformWeights(vector<NeuralNetwork>& population)
{
    for(int k = 0; k < population.size(); k++)
    {
        vector<float> weights = population[k].getWeights();

        for(int i = 0; i < weights.size(); i++)
        {
            if(weights[i] > mParameters.searchSpaceMax)
                weights[i] = mParameters.searchSpaceMin + fmod(weights[i] - mParameters.searchSpaceMax, mParameters.searchSpaceMax - mParameters.searchSpaceMin);
            else if(weights[i] < mParameters.searchSpaceMin)
                weights[i] = mParameters.searchSpaceMax - fmod(mParameters.searchSpaceMin - weights[i], mParameters.searchSpaceMax - mParameters.searchSpaceMin);
        }
        population[k].setWeights(weights);
    }
}

void GA::mutate(vector<NeuralNetwork>& population)
{
    assert(mParameters.mutationProb <= 1.0f && mParameters.mutationProb >= 0.0f);

    boost::mt19937 mutation(rand());

    boost::uniform_real<float> mutationProbDist(0, 1);
    boost::variate_generator<boost::mt19937, boost::uniform_real<float>> genMutationProb(mutation, mutationProbDist);

    float standardDeviation = (mParameters.searchSpaceMax - mParameters.searchSpaceMin) / 5;
    boost::normal_distribution<> normDist(0, standardDeviation);
	boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > genMutation(mutation, normDist);

    
    for(int i = 0; i < population.size(); i++)
        for(int k = 0; k < population[i].getWeights().size(); k++)
            if(genMutationProb() <= mParameters.mutationProb)
                population[i].getWeights()[k] += genMutation();
}

NeuralNetwork GA::train(unsigned int& initializationSeed, vector2 goal)
{
    assert(mParameters.GApopulation > mParameters.elitismCount); 

    initializationSeed = time(0);
    vector<NeuralNetwork> population = initializePopulation();
    assert(population.size() > 0);

    cout << "Starting training: " << endl;

    for(unsigned int k = 0; k < mParameters.maxGenerations; k++)
    {
        cout << "Generation " << k << endl;
        evaluatePopulation(population, goal, initializationSeed);

        if(population[0].getFitness() < mParameters.epsilon)
            return population[0];

        vector<NeuralNetwork> newPopulation = getBest(population, mParameters.elitismCount);
        while(newPopulation.size() < population.size())
        {
            vector<NeuralNetwork> offspring = crossover(population);
            newPopulation.insert(newPopulation.end(), offspring.begin(), offspring.end());
        }

        while(newPopulation.size() > population.size())
            newPopulation.pop_back();

        population = newPopulation;

        mutate(population);

        conformWeights(population);
    }
    evaluatePopulation(population, goal, initializationSeed);

    return population[0];
}

vector<NeuralNetwork> GA::getBest(vector<NeuralNetwork> population, unsigned int amount)
{
    vector<NeuralNetwork> output;

    unsigned int stop = (amount > population.size()) ? population.size() : amount;
    for(int k = 0; k < stop; k++)
        output.push_back(population[k]);

    return output;
}

void GA::quicksort(vector<NeuralNetwork>& elements, int left, int right)
{
	int i = left;
	int j = right;

	NeuralNetwork pivot = elements[(left+ right) / 2];
	do
	{
		while (elements[i].getFitness() < pivot.getFitness())
			i++;
		while (elements[j].getFitness() > pivot.getFitness())
			j--;

		if (i <= j)
		{
			NeuralNetwork temp = elements[i]; elements[i] = elements[j]; elements[j] = temp;
			i++; j--;
		}
	} while (i <= j);

	if (left < j)
		quicksort(elements, left, j);
	if (i < right)
		quicksort(elements, i, right);
}

void GA::evaluatePopulation(vector<NeuralNetwork>& population, vector2 goal, unsigned int initializationSeed)
{
    for(unsigned int i = 0; i < mParameters.GApopulation; i++)
    {
        vector<Object*> objects = initializeModels(initializationSeed);
        assert(objects.size() > 0);

        float fitness = sim.run(mParameters.simulationCycles, population[i], objects, goal);
        cout << "Chromosome: " << i << " with fitness: " << fitness << endl;
        population[i].setFitness(fitness);
        if(fitness == 0.0f)
            break;

        cout << "Weights: ";
        for(int k =0 ; k < population[i].getWeights().size(); k++)
            cout << population[i].getWeights()[k] << " ";
        cout << endl;

        cleanupModels(objects);
    }

    quicksort(population, 0, population.size() - 1);
}