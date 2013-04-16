#ifndef GA_H
#define GA_H

#include <vector>
#include <time.h>
#include <math.h>
#include <omp.h>

#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"

#include "simulation.h"
#include "neuralnetwork.h"
#include "object.h"
#include "common.h"

using namespace std;

enum Crossover{GAUSSIAN_CO, MULTIPOINT_CO, SIMPLEX_CO, SINGLEPOINT_CO, TWOPOINT_CO, SIMULATEDBINARY_CO};

struct GAParams
{
    GAParams() : GApopulation(0), simulationPopulation(0), searchSpaceMax(1.0f), searchSpaceMin(-1.0f), maxGenerations(1), simulationCycles(1), 
        nnInputs(1), nnHiddens(0), nnOutputs(1), maxFitness(1), elitismCount(0), mutationProb(0.0f){}
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
    unsigned int elitismCount;
    float mutationProb;
    float epsilon;
    Crossover crossoverType;

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
    /* INIT AND CLEANUP */
    vector<Object*> initializeModels(unsigned int initializationSeed);
    vector<NeuralNetwork> initializePopulation();
    void cleanupModels(vector<Object*> models);

    /* MUTATION */
    void mutate(vector<NeuralNetwork>& population);
    
    /* CROSSOVER RELATED FUNCTIONS */
    vector<NeuralNetwork> crossover(vector<NeuralNetwork> population);

    //crossover helper
    vector<NeuralNetwork> getParents(vector<NeuralNetwork> population, unsigned int numParents);
    NeuralNetwork selectParent(vector<NeuralNetwork> population, unsigned int& rank);

    //crossover types
    vector<NeuralNetwork> gaussianCrossover(vector<NeuralNetwork> population);
    vector<NeuralNetwork> multipointCrossover(vector<NeuralNetwork> population);
    vector<NeuralNetwork> simplexCrossover(vector<NeuralNetwork> population);
    vector<NeuralNetwork> simulatedbinaryCrossover(vector<NeuralNetwork> population);
    vector<NeuralNetwork> singlepointCrossover(vector<NeuralNetwork> population);
    vector<NeuralNetwork> twopointCrossover(vector<NeuralNetwork> population);

    /* MISC */

    GA(){}
    unsigned int getNumWeights(unsigned int numInput, unsigned int numHidden, unsigned int numOutput);
    vector<NeuralNetwork> getBest(vector<NeuralNetwork> population, unsigned int amount);
    void quicksort(vector<NeuralNetwork>& elements, int left, int right);
    float calculateStandardDeviation(vector<NeuralNetwork> population, NeuralNetwork current, unsigned int position);
    void conformWeights(vector<NeuralNetwork>& population);
    void evaluatePopulation(vector<NeuralNetwork>& population, vector2 goal, unsigned int initializationSeed);


private:
    GAParams mParameters;
    Simulation sim;
};

#endif