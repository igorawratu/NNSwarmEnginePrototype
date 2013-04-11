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
        population.push_back(currNetwork);
    }

    return population;
}

NeuralNetwork GA::crossover(vector<NeuralNetwork> population)
{
}

float NeuralNetwork::calculateStandardDeviation(vector<NeuralNetwork> population, NeuralNetwork current, unsigned int position)
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

NeuralNetwork NeuralNetwork::mutate(NeuralNetwork network, vector<float> deviations)
{
    assert(mParameters.mutationProb <= 1.0f && mutationProb >= 0.0f);

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

    vector<Object*> objects = initializeModels(initializationSeed);
    assert(objects.size() > 0);

    for(unsigned int k = 0; k < mParameters.maxGenerations; k++)
    {
        for(unsigned int i = 0; i < mParameters.GApopulation; i++)
        {
            float fitness = sim.run(mParameters.simulationCycles, population[i], objects, goal);
            population[i].setFitness(fitness);
            if(fitness == 0.0f)
            {
                cleanupModels(objects);
                return population[i];
            }
        }

        vector<NeuralNetwork> newPopulation = getBest(population, mParameters.elitismCount);
        while(newPopulation.size() < population.size())
            newPopulation.push_back(crossover(population));

        population = newPopulation;
        
        for(int i = 0; i < population.size(); i++)
        {
            vector<float> deviations;

            for(int l = 0; l < population[i].getWeights().size(); l++)
                deviations.push_back(calculateStandardDeviation(population, population[i], l);

            population[i] = mutate(population[i], deviations);
        }
    }

    cleanupModels(objects);

    return getBest(population, 1)[0];
}

vector<NeuralNetwork> GA::getBest(vector<NeuralNetwork> population, unsigned int amount)
{
    vector<NeuralNetwork> output;

    quicksort(population);

    unsigned int stop = (amount > population.size()) ? population.size() : amount;
    for(int k = 0; k < stop; k++)
        output.push_back(population[k]);

    return output;
}

void GA::quicksort(vector<NeuralNetwork>& elements, int left, int right)
{
	int i = left;
	int j = right;

	NeuralNetwork pivot = particles[(left+ right) / 2];
	do
	{
		while (elements[i]->getFitness() > pivot->getFitness())
			i++;
		while (elements[j]->getFitness() < pivot->getFitness())
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