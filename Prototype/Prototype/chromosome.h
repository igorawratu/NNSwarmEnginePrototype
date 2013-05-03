#ifndef CHROMOSOME_H
#define CHROMOSOME_H

#include <vector>
#include <iostream>
#include <fstream>

#include "neuralnetwork.h"

using namespace std;

class GA;

enum PrintMode{PRINT_TO_FILE, PRINT_TO_CONSOLE};

class Chromosome
{
friend class GA;

public:
    Chromosome(vector<NeuralNetworkParameter> parameters);
    Chromosome(const Chromosome& other);
    const Chromosome& operator=(const Chromosome& other);
    vector<NeuralNetwork> getBrains();
    void print(PrintMode printMode);

private:
    vector<vector<float>> mWeights;
    vector<NeuralNetworkParameter> mParameters;
    vector<NeuralNetwork> mBrains;
    float mFitness;
};

#endif