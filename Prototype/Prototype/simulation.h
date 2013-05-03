#ifndef SIMULATION_H
#define SIMULATION_H

#include "neuralnetwork.h"

#include <vector>

using namespace std;

struct SimulationParams
{
    SimulationParams() : maxFitness(1), simulationCycles(1), cyclesPerDecision(1), modelGroups(1){}
    vector<unsigned int> simulationPopulation;
    unsigned int modelGroups;
    float maxFitness;
    unsigned int simulationCycles;
    unsigned int cyclesPerDecision;
};

class Simulation
{
public:
    Simulation(SimulationParams parameters, bool renderable){this->parameters = parameters; mRenderable = renderable;}
    Simulation(const Simulation& other){}
    Simulation& operator=(const Simulation& other){}
    virtual ~Simulation(){}

    virtual void reset() = 0;
    virtual void cycle(vector<NeuralNetwork> brains, unsigned int currentIteration) = 0;
    float fullRun(vector<NeuralNetwork> brains)
    {
        for(unsigned int k = 0; k < parameters.simulationCycles; k++)
            cycle(brains, k);
    }
    virtual float evaluateFitness()=0;
    virtual void render()=0;

public:
    SimulationParams parameters;

protected:
    bool mRenderable;

protected:
    Simulation(){}
};

#endif