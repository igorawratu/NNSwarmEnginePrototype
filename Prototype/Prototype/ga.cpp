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

vector<Chromosome> GA::initializePopulation(SimulationParams simParams, int chromosomeGroup)
{
    vector<Chromosome> population;

    boost::mt19937 rng(rand());
    boost::uniform_real<float> initWeightDist(mParameters.searchSpaceMin, mParameters.searchSpaceMax);
    boost::variate_generator<boost::mt19937, boost::uniform_real<float>> genInitPos(rng, initWeightDist);

    vector<unsigned int> numWeights = getNumWeights();

    if(chromosomeGroup < 0)
    {
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
            currChromosome.mFitness = simParams.maxFitness;

            population.push_back(currChromosome);
        }
    }
    else
    {
        for(unsigned int k = 0; k < mParameters.GApopulation; k++)
        {
            vector<vector<float>> currChromosomeWeights;
            
            vector<float> currnetweights;
            for(unsigned int i = 0; i < numWeights[chromosomeGroup]; i++)
                currnetweights.push_back(genInitPos());
            
            currChromosomeWeights.push_back(currnetweights);

            vector<NeuralNetworkParameter> parameters;
            parameters.push_back(mParameters.nnParameters[chromosomeGroup]);

            Chromosome currChromosome(parameters);
            currChromosome.mWeights = currChromosomeWeights;
            currChromosome.mFitness = simParams.maxFitness;

            population.push_back(currChromosome);
        }
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

vector<Chromosome> GA::gaussianCrossover(vector<Chromosome> population, float maxFitness, vector<NeuralNetworkParameter> parameters)
{
    vector<Chromosome> output;

    vector<Chromosome> parents = getParents(population, 2);

    Chromosome child(parameters);
    
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

vector<Chromosome> GA::multipointCrossover(vector<Chromosome> population, float maxFitness, vector<NeuralNetworkParameter> parameters)
{
    vector<Chromosome> output;

    vector<Chromosome> parents = getParents(population, 2);

    Chromosome child(parameters);
    
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

vector<Chromosome> GA::simplexCrossover(vector<Chromosome> population, float maxFitness, vector<NeuralNetworkParameter> parameters)
{
    vector<Chromosome> output;
    vector<Chromosome> parents = getParents(population, 4);

    Chromosome child(parameters);

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

vector<Chromosome> GA::singlepointCrossover(vector<Chromosome> population, float maxFitness, vector<NeuralNetworkParameter> parameters)
{
    vector<Chromosome> output;

    vector<Chromosome> parents = getParents(population, 2);

    Chromosome child(parameters);
    
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

vector<Chromosome> GA::twopointCrossover(vector<Chromosome> population, float maxFitness, vector<NeuralNetworkParameter> parameters)
{
    vector<Chromosome> output;

    vector<Chromosome> parents = getParents(population, 2);

    Chromosome child(parameters);
    
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

vector<Chromosome> GA::simulatedbinaryCrossover(vector<Chromosome> population, float maxFitness, vector<NeuralNetworkParameter> parameters)
{
    vector<Chromosome> output;
    
    vector<Chromosome> parents = getParents(population, 2);
    Chromosome child1(parameters); 
    Chromosome child2(parameters); 
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

vector<Chromosome> GA::crossover(vector<Chromosome> population, float maxFitness, int chromosomeGroup)
{
    vector<Chromosome> output;

    vector<NeuralNetworkParameter> parameters;
    if(chromosomeGroup < 0)
        parameters = mParameters.nnParameters;
    else parameters.push_back(mParameters.nnParameters[chromosomeGroup]);

    switch (mParameters.crossoverType)
    {
        case GAUSSIAN_CO: 
            output = gaussianCrossover(population, maxFitness, parameters);
            break;
        case MULTIPOINT_CO: 
            output = multipointCrossover(population, maxFitness, parameters);
            break;
        case SIMPLEX_CO:
            output = simplexCrossover(population, maxFitness, parameters);
            break;
        case SINGLEPOINT_CO:
            output = singlepointCrossover(population, maxFitness, parameters);
            break;
        case TWOPOINT_CO:
            output = twopointCrossover(population, maxFitness, parameters);
            break;
        case SIMULATEDBINARY_CO:
            output = simulatedbinaryCrossover(population, maxFitness, parameters);
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

vector<NeuralNetwork> GA::train(SimulationParams simParams)
{
    assert(mParameters.GApopulation > mParameters.elitismCount); 

    cout << "Initialising" << endl;
    vector<Chromosome> population = initializePopulation(simParams, -1);
    assert(population.size() > 0);

    cout << "Starting training: " << endl;

    for(unsigned int k = 0; k < mParameters.maxGenerations; k++)
    {
        cout << "Generation " << k << endl;

        evaluatePopulation(population, simParams);

        if(population[0].mFitness <= mParameters.epsilon)
            return population[0].getBrains();
        
        vector<Chromosome> newPopulation = getFirst(population, mParameters.elitismCount);
        while(newPopulation.size() < population.size())
        {
            vector<Chromosome> offspring = crossover(population, simParams.maxFitness, -1);
            newPopulation.insert(newPopulation.end(), offspring.begin(), offspring.end());
        }

        while(newPopulation.size() > population.size())
            newPopulation.pop_back();

        population = newPopulation;

        mutate(population);

        conformWeights(population);
    }
    evaluatePopulation(population, simParams);

    return population[0].getBrains();
}

vector<Chromosome> GA::getFirst(vector<Chromosome> population, unsigned int amount)
{
    vector<Chromosome> output;

    unsigned int stop = (amount > population.size()) ? population.size() : amount;
    output.insert(output.end(), population.begin(), population.begin() + stop);

    return output;
}

void GA::evaluateCompetitivePopulation(vector<vector<Chromosome>>& population, SimulationParams simParams, bool fullSim)
{
    //assume two for now
    assert(population.size() == 2);
    cout << "evaluating... " << endl;
    if(!fullSim)
    {
        for(int i = 0; i < population[0].size(); i++)
        {
            vector<Chromosome> competeSetOne = getParents(population[1], mParameters.GApopulation/5);
            for(int k = 0; k < competeSetOne.size(); k++)
            {
                CompetitiveSimulation sim(simParams, false);
                vector<NeuralNetwork> brains;
                brains.push_back(population[0][i].getBrains()[0]);
                brains.push_back(competeSetOne[k].getBrains()[0]);
                sim.fullRun(brains);
                float winner = sim.getWinner();
                if(winner == 0)
                    population[0][i].mFitness -= 1;
                sim.shutdown();
            }
        }
        for(int i = 0; i < population[1].size(); i++)
        {
            vector<Chromosome> competeSetTwo = getParents(population[0], mParameters.GApopulation/5);
            for(int k = 0; k < competeSetTwo.size(); k++)
            {
                CompetitiveSimulation sim(simParams, false);
                vector<NeuralNetwork> brains;
                brains.push_back(competeSetTwo[k].getBrains()[0]);
                brains.push_back(population[1][i].getBrains()[0]);
                sim.fullRun(brains);
                float winner = sim.getWinner();
                if(winner == 1)
                    population[1][i].mFitness -= 1;
                sim.shutdown();
            }
        }
    }
    else
    {
        for(int i = 0; i < population[0].size(); i++)
        {
            for(int k = 0; k < population[1].size(); k++)
            {
                //cout << i << " " << k << endl;
                CompetitiveSimulation sim(simParams, false);
                vector<NeuralNetwork> brains;
                brains.push_back(population[0][i].getBrains()[0]);
                brains.push_back(population[1][k].getBrains()[0]);
                sim.fullRun(brains);
                float winner = sim.getWinner();
                if(winner == 0)
                    population[0][i].mFitness -= 1;
                else population[1][k].mFitness -=1;

                sim.shutdown();
            }
        }
    }
    quicksort(population[0], 0, population[0].size() - 1);
    quicksort(population[1], 0, population[1].size() - 1);

    for(int k = 0; k < population.size(); k++)
        for(int i = 0; i < population[k].size(); i++)
            cout << "Chromsome " << i << " of population " << k << " with fitness " << population[k][i].mFitness << endl;
    
}

void GA::evaluateModularPopulation(vector<vector<Chromosome>>& population, SimulationParams simParams)
{
    assert(population.size() == 2);

    for(int i = 0; i < population[0].size(); i++)
    {
        float fitnessVal = 0;
        vector<Chromosome> competeSetOne = getParents(population[1], 5);
        for(int k = 0; k < competeSetOne.size(); k++)
        {
            CompetitiveSimulation sim(simParams, false);
            vector<NeuralNetwork> brains;
            brains.push_back(population[0][i].getBrains()[0]);
            brains.push_back(competeSetOne[k].getBrains()[0]);
            sim.fullRun(brains);
            float a1Fit, a2Fit;
            sim.getIndividualFitness(a1Fit, a2Fit);
            fitnessVal += a1Fit;
            sim.shutdown();
        }
        population[0][i].mFitness = fitnessVal / 5;
    }

    for(int i = 0; i < population[1].size(); i++)
    {
        float fitnessVal = 0;
        vector<Chromosome> competeSetTwo = getParents(population[0], 5);
        for(int k = 0; k < competeSetTwo.size(); k++)
        {
            CompetitiveSimulation sim(simParams, false);
            vector<NeuralNetwork> brains;
            brains.push_back(competeSetTwo[k].getBrains()[0]);
            brains.push_back(population[1][i].getBrains()[0]);
            sim.fullRun(brains);
            float a1Fit, a2Fit;
            sim.getIndividualFitness(a1Fit, a2Fit);
            fitnessVal += a2Fit;
            sim.shutdown();
        }
        population[1][i].mFitness = fitnessVal / 5;
    }

    quicksort(population[0], 0, population[0].size() - 1);
    quicksort(population[1], 0, population[1].size() - 1);

    for(int k = 0; k < population.size(); k++)
        for(int i = 0; i < population[k].size(); i++)
            cout << "Chromsome " << i << " of population " << k << " with fitness " << population[k][i].mFitness << endl;

}

void GA::evaluateCompetitiveSinglePopulation(vector<Chromosome>& population, SimulationParams simParams, bool fullSim)
{
    for(int k = 0; k < population.size(); k++)
    {
        vector<Chromosome> competeSet = getParents(population, fullSim ? mParameters.GApopulation : mParameters.GApopulation/5);
        for(int i = 0; i < competeSet.size(); i++)
        {
            CompetitiveSimulation sim(simParams, false);
            vector<NeuralNetwork> brains;
            brains.push_back(population[k].getBrains()[0]);
            brains.push_back(competeSet[i].getBrains()[0]);
            sim.fullRun(brains);
            float winner = sim.getWinner();
            population[k].mFitness -= (winner == 0) ? 1 : 0;
            sim.shutdown();
        }
    }

    quicksort(population, 0, population.size() - 1);
    
    for(int k = 0; k < population.size(); k++)
        cout << "Chromosome " << k << " with fitness " << population[k].mFitness << endl;
}   

vector<NeuralNetwork> GA::modularTrain(SimulationParams simParams)
{
    assert(mParameters.GApopulation > mParameters.elitismCount);
    unsigned long lastTime = time(0);

    vector<vector<Chromosome>> populations;

    for(int k = 0; k < mParameters.nnParameters.size(); k++)
        populations.push_back(initializePopulation(simParams, k));

    for(unsigned int k = 0; k < mParameters.maxGenerations; k++)
    {
        cout << "TIME ELAPSED SINCE LAST GENERATION: " << time(0) - lastTime << endl;
        lastTime = time(0);

        cout << "Generation " << k << endl;

        evaluateModularPopulation(populations, simParams);

        CompetitiveSimulation sim(simParams, false);

        vector<NeuralNetwork> brains;
        for(int k = 0; k < populations.size(); k++)
            brains.push_back(populations[k][0].getBrains()[0]);

        sim.fullRun(brains);
        float fit = sim.evaluateFitness();
        sim.shutdown();
        cout << "Best chromosome fitness for current generation: " << fit << endl;
            
        if(fit <= mParameters.epsilon)
        {
            cout << "Fitness below epsilon" << endl;
            vector<NeuralNetwork> output;
            for(int k = 0; k < populations.size(); k++)
                output.push_back(populations[k][0].getBrains()[0]);
            return output;
        }
        
        for(int i = 0; i < mParameters.nnParameters.size(); i++)
        {
            vector<Chromosome> newPopulation = getFirst(populations[i], mParameters.elitismCount);
            for(int l = 0; l < newPopulation.size(); l++)
                newPopulation[l].mFitness = simParams.maxFitness;
            while(newPopulation.size() < populations[i].size())
            {
                vector<Chromosome> offspring = crossover(populations[i], simParams.maxFitness, i);
                newPopulation.insert(newPopulation.end(), offspring.begin(), offspring.end());
            }

            while(newPopulation.size() > populations[i].size())
                newPopulation.pop_back();

            populations[i] = newPopulation;

            mutate(populations[i]);

            conformWeights(populations[i]);
        }
    }

    evaluateModularPopulation(populations, simParams);
    vector<NeuralNetwork> output;
    for(int k = 0; k < populations.size(); k++)
        output.push_back(populations[k][0].getBrains()[0]);

    return output;
}

vector<NeuralNetwork> GA::competePopulation(SimulationParams simParams)
{   
    assert(mParameters.GApopulation > mParameters.elitismCount); 

    unsigned long lastTime = time(0);

    vector<vector<Chromosome>> populations;
    for(int k = 0; k < mParameters.nnParameters.size(); k++)
        populations.push_back(initializePopulation(simParams, k));

    for(unsigned int k = 0; k < mParameters.maxGenerations; k++)
    {
        cout << "TIME ELAPSED SINCE LAST GENERATION: " << time(0) - lastTime << endl;
        lastTime = time(0);

        cout << "Generation " << k << endl;

        evaluateCompetitivePopulation(populations, simParams, mParameters.fullSample);

        CompetitiveSimulation sim(simParams, false);
        vector<NeuralNetwork> brains;
        brains.push_back(populations[0][0].getBrains()[0]);
        brains.push_back(populations[1][0].getBrains()[0]);
        sim.fullRun(brains);
        float fit = sim.evaluateFitness();
        sim.shutdown();
        cout << "Best chromosome fitness for current generation: " << fit << endl;
            
        if(fit <= mParameters.epsilon)
        {
            cout << "Fitness below epsilon" << endl;
            vector<NeuralNetwork> output;
            for(int k = 0; k < populations.size(); k++)
                output.push_back(populations[k][0].getBrains()[0]);
            return output;
        }
        
        for(int i = 0; i < mParameters.nnParameters.size(); i++)
        {
            vector<Chromosome> newPopulation = getFirst(populations[i], mParameters.elitismCount);
            for(int l = 0; l < newPopulation.size(); l++)
                newPopulation[l].mFitness = simParams.maxFitness;
            while(newPopulation.size() < populations[i].size())
            {
                vector<Chromosome> offspring = crossover(populations[i], simParams.maxFitness, i);
                newPopulation.insert(newPopulation.end(), offspring.begin(), offspring.end());
            }

            while(newPopulation.size() > populations[i].size())
                newPopulation.pop_back();

            populations[i] = newPopulation;

            mutate(populations[i]);

            conformWeights(populations[i]);
        }
        
    }
    evaluateCompetitivePopulation(populations, simParams, mParameters.fullSample);
    vector<NeuralNetwork> output;
    for(int k = 0; k < populations.size(); k++)
        output.push_back(populations[k][0].getBrains()[0]);

    return output;
}



vector<NeuralNetwork> GA::competeSinglePopulation(SimulationParams simParams)
{   
    assert(mParameters.GApopulation > mParameters.elitismCount);
    assert(mParameters.nnParameters.size() == 1);

    unsigned long lastTime = time(0);

    vector<Chromosome> population = initializePopulation(simParams, -1);

    for(unsigned int k = 0; k < mParameters.maxGenerations; k++)
    {
        cout << "TIME ELAPSED SINCE LAST GENERATION: " << time(0) - lastTime << endl;
        lastTime = time(0);

        cout << "Generation " << k << endl;

        evaluateCompetitiveSinglePopulation(population, simParams, mParameters.fullSample);

        CompetitiveSimulation sim(simParams, false);
        vector<NeuralNetwork> brains;
        brains.push_back(population[0].getBrains()[0]);
        brains.push_back(population[1].getBrains()[0]);
        sim.fullRun(brains);
        float fit = sim.evaluateFitness();
        sim.shutdown();
        cout << "Best chromosome fitness for current generation: " << fit << endl;
            
        if(fit <= mParameters.epsilon)
        {
            cout << "Fitness below epsilon" << endl;
            vector<NeuralNetwork> output;
            output.push_back(population[0].getBrains()[0]);
            output.push_back(population[1].getBrains()[0]);
            return output;
        }
        
        vector<Chromosome> newPopulation = getFirst(population, mParameters.elitismCount);
        for(int l = 0; l < newPopulation.size(); l++)
            newPopulation[l].mFitness = simParams.maxFitness;
        while(newPopulation.size() < population.size())
        {
            vector<Chromosome> offspring = crossover(population, simParams.maxFitness, 0);
            newPopulation.insert(newPopulation.end(), offspring.begin(), offspring.end());
        }

        while(newPopulation.size() > population.size())
            newPopulation.pop_back();

        population = newPopulation;

        mutate(population);

        conformWeights(population);
        
    }
    evaluateCompetitiveSinglePopulation(population, simParams, mParameters.fullSample);
    
    vector<NeuralNetwork> output;
    output.push_back(population[0].getBrains()[0]);
    output.push_back(population[1].getBrains()[0]);

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

void GA::evaluatePopulation(vector<Chromosome>& population, SimulationParams simParams)
{
    for(unsigned int i = 0; i < mParameters.GApopulation; i++)
    {
        CompetitiveSimulation sim(simParams, false);
        vector<NeuralNetwork> brains = population[i].getBrains();
        
        sim.fullRun(brains);
        
        float fitness = sim.evaluateFitness();
        cout << "Chromosome: " << i << " with fitness: " << fitness << endl;
        
        population[i].mFitness = fitness;
        
        if(fitness <= mParameters.epsilon)
            break;

        sim.shutdown();

    }

    quicksort(population, 0, population.size() - 1);
}