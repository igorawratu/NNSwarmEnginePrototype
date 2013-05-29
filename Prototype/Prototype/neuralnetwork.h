#ifndef NEURALNETWORK_H
#define NEURALNETWORK_H

#include <vector>
#include <iostream>
#include <math.h>
#include <assert.h>
#include <fstream>

using namespace std;

const float e = 2.71828f;

class Chromosome;

struct NeuralNetworkParameter
{
    unsigned int inputNodes;
    unsigned int hiddenNodes;
    unsigned int outputNodes;
};

class NeuralNetwork
{
friend class Chromosome;

public:
    NeuralNetwork(NeuralNetworkParameter parameters);
    NeuralNetwork(const NeuralNetwork& other);
    NeuralNetwork& operator=(const NeuralNetwork& other);
    ~NeuralNetwork(){}

    vector<float> evaluate(vector<float> input, bool& status);
    void print(ofstream& writer)
    {
        for(int k = 0; k < mWeights.size(); k++)
        {
            writer << mWeights[k];
            if(k < mWeights.size() - 1)
                writer << ", ";
            else writer << endl;
        }
    }

    void print()
    {
        for(int k = 0; k < mWeights.size(); k++)
        {
            cout << mWeights[k];
            if(k < mWeights.size() - 1)
                cout << ", ";
            else cout << endl;
        }
    }

    void setWeights(vector<float> weights){mWeights = weights;}

private:
    NeuralNetwork(){}
    float activationFunc(vector<float> inputs, vector<float> weightVec);
    vector<float> evaluate(vector<float> input, vector<float>& weights, bool& status);

private:
    vector<float> mWeights;
    unsigned int mWeightCount;
    NeuralNetworkParameter mParameters;
};

#endif