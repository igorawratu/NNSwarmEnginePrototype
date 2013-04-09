#ifndef NEURALNETWORK_H
#define NEURALNETWORK_H

#include <vector>
#include <iostream>

using namespace std;

class NeuralNetwork
{
public:
    NeuralNetwork(unsigned int numInputs, unsigned int numOutputs, unsigned int numHidden);
    NeuralNetwork(const NeuralNetwork& other);
    ~NeuralNetwork();

    NeuralNetwork& operator=(const NeuralNetwork& other);

    vector<float> evaluate(vector<float> input, bool& status);
    bool setWeights(vector<float> weights);
    vector<float> getWeights(){return mWeights;}

private:
    NeuralNetwork(){}
    float activationFunc(vector<float> inputs, vector<float> weightVec);

private:
    vector<float> mWeights;
    unsigned int mWeightCount, mInputCount, mOutputCount, mHiddenCount;
};

#endif