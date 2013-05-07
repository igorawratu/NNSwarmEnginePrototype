#include "chromosome.h"

Chromosome::Chromosome(vector<NeuralNetworkParameter> parameters)
{
    mParameters = parameters;
    for(unsigned int k = 0; k < mParameters.size(); k++)
    {
        mBrains.push_back(NeuralNetwork(mParameters[k]));
        mWeights.push_back(mBrains[k].mWeights);
    }
}

Chromosome::Chromosome(const Chromosome& other)
{
    mParameters = other.mParameters;
    mBrains = other.mBrains;
    mFitness = other.mFitness;
    mWeights = other.mWeights;
}

const Chromosome& Chromosome::operator=(const Chromosome& other)
{
    mParameters = other.mParameters;
    mBrains = other.mBrains;
    mFitness = other.mFitness;
    mWeights = other.mWeights;

    return *this;
}

    
vector<NeuralNetwork> Chromosome::getBrains()
{
    for(unsigned int k = 0; k < mBrains.size(); k++)
        mBrains[k].mWeights = mWeights[k];

    return mBrains;
}

void Chromosome::print(PrintMode printMode)
{
    //can leave for later
}