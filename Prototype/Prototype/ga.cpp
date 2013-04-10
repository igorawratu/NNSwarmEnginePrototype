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

NeuralNetwork NeuralNetwork::mutate(NeuralNetwork)
{

}

NeuralNetwork GA::train(unsigned int& initializationSeed, vector2 goal)
{
    assert(mParameters.GApopulation > mParameters.elitismCount); 

    initializationSeed = time(0);
    vector<NeuralNetwork> population = initializePopulation();
    assert(population.size() > 0);

    vector<Object*> objects = initializeModels(initializationSeed);
    assert(objects.size() > 0);

    boost::mt19937 rng(rand());
    boost::uniform_int<> crossoverSelectionDist(0, population.size() - 1);

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
            population[i] = mutate(population[i]);
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