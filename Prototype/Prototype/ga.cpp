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

NeuralNetwork GA::crossover(vector<NeuralNetwork> population)
{
    unsigned int p1rank;
    NeuralNetwork parent1 = selectParent(population, p1rank);
    unsigned int p2rank = p1rank;

    NeuralNetwork parent2(mParameters.nnInputs, mParameters.nnOutputs, mParameters.nnHiddens);
    while(p2rank == p1rank)
        parent2 = selectParent(population, p2rank);
    NeuralNetwork child(mParameters.nnInputs, mParameters.nnOutputs, mParameters.nnHiddens);
    
    boost::mt19937 mutation(rand());
    
    vector<float> weights;
    for(int k = 0; k < parent1.getWeights().size(); k++)
    {
        boost::normal_distribution<> normDist((parent1.getWeights()[k] + parent2.getWeights()[k]) / 2 , fabs(parent1.getWeights()[k] - parent2.getWeights()[k]));
	    boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > genCrossover(mutation, normDist);
        
        float val = genCrossover();
        weights.push_back(val);
    }

    child.setWeights(weights);
    child.setFitness(mParameters.maxFitness);

    return child;
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

NeuralNetwork GA::mutate(NeuralNetwork network, vector<float> deviations)
{
    assert(mParameters.mutationProb <= 1.0f && mParameters.mutationProb >= 0.0f);

    boost::mt19937 mutation(rand());
    boost::uniform_real<float> mutationProbDist(0, 1);
    boost::variate_generator<boost::mt19937, boost::uniform_real<float>> genMutationProb(mutation, mutationProbDist);

    vector<float> weights = network.getWeights();
    for(int k = 0; k < weights.size(); k++)
    {
        boost::normal_distribution<> normDist(0, deviations[k]);
	    boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > genMutation(mutation, normDist);

        if(genMutationProb() <= mParameters.mutationProb)
            weights[k] += genMutation();
    }

    network.setWeights(weights);

    return network;
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
            newPopulation.push_back(crossover(population));

        population = newPopulation;
        
        vector<vector<float>> deviationMatrix;

        for(int i = 0; i < population.size(); i++)
        {
            vector<float> deviations;

            for(int l = 0; l < population[i].getWeights().size(); l++)
                deviations.push_back(calculateStandardDeviation(population, population[i], l));

            deviationMatrix.push_back(deviations);
        }

        for(int i = 0; i < deviationMatrix.size(); i++)
            population[i] = mutate(population[i], deviationMatrix[i]);

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