#include "neuralnetwork.h"

NeuralNetwork::NeuralNetwork(unsigned int numInputs, unsigned int numOutputs, unsigned int numHidden)
{
    if(numHidden > 0)
        mWeightCount = numHidden * (numInputs + numOutputs + 1) + numOutputs;
    else mWeightCount = numOutputs * (numInputs + 1);

    mInputCount = numInputs;
    mOutputCount = numOutputs;
    mHiddenCount = numHidden;
    mFitness = 0;

    for(int k = 0; k < mWeightCount; k++)
        mWeights.push_back(0);
}

NeuralNetwork::NeuralNetwork(const NeuralNetwork& other)
{
    mWeights = other.mWeights;
    mWeightCount = other.mWeightCount;
    mInputCount = other.mInputCount;
    mOutputCount = other.mOutputCount;
    mHiddenCount = other.mHiddenCount;
    mFitness = other.mFitness;
}

NeuralNetwork::~NeuralNetwork()
{
}

NeuralNetwork& NeuralNetwork::operator=(const NeuralNetwork& other)
{
    mWeights = other.mWeights;
    mWeightCount = other.mWeightCount;
    mInputCount = other.mInputCount;
    mOutputCount = other.mOutputCount;
    mHiddenCount = other.mHiddenCount;
    mFitness = other.mFitness;

    return *this;
}

vector<float> NeuralNetwork::evaluate(vector<float> input, bool& status)
{
    vector<float> output;
    if(input.size() != mInputCount)
    {
        status = false;
        return output;
    }
        
    status = true;
    if(mHiddenCount == 0)
    {
        for(unsigned int k = 0; k < mOutputCount; k++)
        {
            vector<float> weightVec;
            unsigned int max = (k + 1) * mInputCount + k + 1;
            unsigned int next = k * mInputCount + k;
            
            for(unsigned int i = next; i < max; i++)
                weightVec.push_back(mWeights[i]);

            output.push_back(activationFunc(input, weightVec));
        }
    }
    else
    {
        vector<float> hidden;
        for(unsigned int k = 0; k < mHiddenCount; k++)
        {
            vector<float> weightVec;
            unsigned int max = (k + 1) * mInputCount + k + 1;
            unsigned int next = k * mInputCount + k;

            for(unsigned int i = next; i < max; i++)
                weightVec.push_back(mWeights[i]);

            hidden.push_back(activationFunc(input, weightVec));
        }
        
        unsigned int initial = mHiddenCount * (mInputCount + 1);
        for(unsigned int k = 0; k < mHiddenCount; k++)
        {
            vector<float> weightVec;
            unsigned int max = (k + 1) * mHiddenCount + k + 1;
            unsigned int next = k * mHiddenCount+ k;

            for(unsigned int i = initial + next; i < initial + max; i++)
                weightVec.push_back(mWeights[i]);

            output.push_back(activationFunc(input, weightVec));
        }
    }

    return output;
}

bool NeuralNetwork::setWeights(vector<float> weights)
{
    if(weights.size() == mWeightCount)
    {
        mWeights = weights;
        return true;
    }
    else return false;

}