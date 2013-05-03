#include "neuralnetwork.h"

NeuralNetwork::NeuralNetwork(NeuralNetworkParameter parameters)
{
    if(mParameters.inputNodes > 0)
        mWeightCount = mParameters.hiddenNodes * (mParameters.inputNodes + mParameters.outputNodes + 1) + mParameters.outputNodes;
    else mWeightCount = mParameters.outputNodes * (mParameters.inputNodes + 1);

    for(int k = 0; k < mWeightCount; k++)
        mWeights.push_back(0);
}

NeuralNetwork::NeuralNetwork(const NeuralNetwork& other)
{
    mParameters = other.mParameters;
    mWeightCount = other.mWeightCount;
    mWeights = other.mWeights;
}

NeuralNetwork& NeuralNetwork::operator=(const NeuralNetwork& other)
{
    mParameters = other.mParameters;
    mWeightCount = other.mWeightCount;
    mWeights = other.mWeights;

    return *this;
}

vector<float> NeuralNetwork::evaluate(vector<float> input, bool& status)
{
    return evaluate(input, mWeights, status);
}

vector<float> NeuralNetwork::evaluate(vector<float> input, vector<float>& weights, bool& status)
{
    vector<float> output;
    if(input.size() != mParameters.inputNodes)
    {
        status = false;
        return output;
    }
        
    status = true;
    if(mParameters.hiddenNodes == 0)
    {
        for(unsigned int k = 0; k < mParameters.outputNodes; k++)
        {
            vector<float> weightVec;
            unsigned int max = (k + 1) * mParameters.inputNodes + k + 1;
            unsigned int next = k * mParameters.inputNodes + k;
            
            for(unsigned int i = next; i < max; i++)
                weightVec.push_back(weights[i]);

            output.push_back(activationFunc(input, weightVec));
        }
    }
    else
    {
        vector<float> hidden;
        for(unsigned int k = 0; k < mParameters.hiddenNodes; k++)
        {
            vector<float> weightVec;
            unsigned int max = (k + 1) * mParameters.inputNodes + k + 1;
            unsigned int next = k * mParameters.inputNodes + k;

            for(unsigned int i = next; i < max; i++)
                weightVec.push_back(weights[i]);

            hidden.push_back(activationFunc(input, weightVec));
        }
        
        unsigned int initial = mParameters.hiddenNodes * (mParameters.inputNodes + 1);
        for(unsigned int k = 0; k < mParameters.outputNodes; k++)
        {
            vector<float> weightVec;
            unsigned int max = (k + 1) * mParameters.hiddenNodes + k + 1;
            unsigned int next = k * mParameters.hiddenNodes + k;

            for(unsigned int i = initial + next; i < initial + max; i++)
                weightVec.push_back(weights[i]);

            output.push_back(activationFunc(hidden, weightVec));
        }
    }

    return output;
}

float NeuralNetwork::activationFunc(vector<float> inputs, vector<float> weightVec)
{
    //weight vector 1 larger than input to account for bias
    assert(inputs.size() + 1 == weightVec.size());
    double powerVal = 0.0f;

    for(int k = 0; k < inputs.size(); k++)
        powerVal += weightVec[k] * inputs[k];
    powerVal += -1 * weightVec[weightVec.size() - 1];

    return 1/(1 + pow(e, powerVal));
}