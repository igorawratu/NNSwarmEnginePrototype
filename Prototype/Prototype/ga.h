#ifndef GA_H
#define GA_H

#include <vector>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <algorithm>

#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"

#include "simulation.h"
#include "neuralnetwork.h"
#include "chromosome.h"

using namespace std;

enum Crossover{GAUSSIAN_CO, MULTIPOINT_CO, SIMPLEX_CO, SINGLEPOINT_CO, TWOPOINT_CO, SIMULATEDBINARY_CO};

struct GAParams
{
    GAParams() : GApopulation(0), searchSpaceMax(1.f), searchSpaceMin(-1.f), maxGenerations(1), elitismCount(0), mutationProb(0.0f), epsilon(1.f){}
    unsigned int GApopulation;
    float searchSpaceMax;
    float searchSpaceMin;
    unsigned int maxGenerations;
    unsigned int elitismCount;
    float mutationProb;
    float epsilon;
    Crossover crossoverType;
    vector<NeuralNetworkParameter> nnParameters;
};


class GA
{
public:
    GA(GAParams parameters);
    GA(const GA& other);
    GA& operator=(const GA& other);
    ~GA(){}

    vector<NeuralNetwork> train(Simulation* simulation);
    vector<NeuralNetwork> competePopulation(Simulation* simulation);
    void setParameters(GAParams parameters){mParameters = parameters;};

private:
    /* INIT */
    vector<Chromosome> initializePopulation(Simulation* simulation, int chromosomeGroup);

    /* MUTATION */
    void mutate(vector<Chromosome>& population);
    
    /* CROSSOVER RELATED FUNCTIONS */
    vector<Chromosome> crossover(vector<Chromosome> population, float maxFitness, int chromosomeGroup);

    //crossover helper
    vector<Chromosome> getParents(vector<Chromosome> population, unsigned int numParents);
    Chromosome selectParent(vector<Chromosome> population, unsigned int& rank);

    //crossover types
    vector<Chromosome> gaussianCrossover(vector<Chromosome> population, float maxFitness, vector<NeuralNetworkParameter> parameters);
    vector<Chromosome> multipointCrossover(vector<Chromosome> population, float maxFitness, vector<NeuralNetworkParameter> parameters);
    vector<Chromosome> simplexCrossover(vector<Chromosome> population, float maxFitness, vector<NeuralNetworkParameter> parameters);
    vector<Chromosome> simulatedbinaryCrossover(vector<Chromosome> population, float maxFitness, vector<NeuralNetworkParameter> parameters);
    vector<Chromosome> singlepointCrossover(vector<Chromosome> population, float maxFitness, vector<NeuralNetworkParameter> parameters);
    vector<Chromosome> twopointCrossover(vector<Chromosome> population, float maxFitness, vector<NeuralNetworkParameter> parameters);

    /* MISC */

    GA(){}
    vector<unsigned int> getNumWeights();
    vector<Chromosome> getFirst(vector<Chromosome> population, unsigned int amount);
    void quicksort(vector<Chromosome>& elements, int left, int right);
    void conformWeights(vector<Chromosome>& population);
    void evaluatePopulation(vector<Chromosome>& population, Simulation* simulation);
    void evaluateCompetitivePopulation(vector<vector<Chromosome>>& population, Simulation* simulation);

private:
    GAParams mParameters;
};

#endif