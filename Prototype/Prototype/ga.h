#ifndef GA_H
#define GA_H

#include <vector>
#include <time.h>

#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"

#include "simulation.h"
#include "neuralnetwork.h"
#include "object.h"
#include "common.h"

using namespace std;

struct GAParams
{
    GAParams() : GApopulation(0), simulationPopulation(0), searchSpaceMax(1.0f), searchSpaceMin(-1.0f), maxGenerations(1), simulationCycles(1), nnInputs(1), nnHiddens(0), nnOutputs(1), maxFitness(1){}
    unsigned int GApopulation;
    unsigned int simulationPopulation;
    float searchSpaceMax;
    float searchSpaceMin;
    unsigned int maxGenerations;
    unsigned int simulationCycles;
    unsigned int nnInputs;
    unsigned int nnHiddens;
    unsigned int nnOutputs;
    vector2 modelMoveSpaceMin;
    vector2 modelMoveSpaceMax;
    vector2 modelInitSpaceMin;
    vector2 modelInitSpaceMax;
    vector2 vMax;
    vector2 vMin;
    vector4 modelColour;
    float maxFitness;
};


class GA
{
public:
    GA(GAParams parameters);
    GA(const GA& other);
    GA& operator=(const GA& other);
    ~GA(){}

    NeuralNetwork train(unsigned int& initializationSeed, vector2 goal);
    void setParameters(GAParams parameters){mParameters = parameters;};

private:
    GA(){}
    unsigned int getNumWeights(unsigned int numInput, unsigned int numHidden, unsigned int numOutput);
    vector<Object*> initializeModels(unsigned int initializationSeed);
    void cleanupModels(vector<Object*> models);
    vector<NeuralNetwork> initializePopulation();
    vector<NeuralNetwork> getBest(vector<NeuralNetwork> population, unsigned int amount);

private:
    GAParams mParameters;
    Simulation sim;
};

#endif