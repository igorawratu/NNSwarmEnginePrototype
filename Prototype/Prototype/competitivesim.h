#ifndef COMPETITIVESIM_H
#define COMPETITIVESIM_H

#include "simulation.h"
#include "common.h"
#include "squareagent.h"
#include "lineobject.h"

#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;

class CompetitiveSimulation : public Simulation
{
public:
    CompetitiveSimulation(SimulationParams parameters, bool renderable);
    virtual ~CompetitiveSimulation();

    virtual void reset();
    virtual void cycle(vector<NeuralNetwork> brains, unsigned int currentIteration);
    virtual float evaluateFitness();
    virtual void render(GLuint shadername);

private:
    float calcDistance(vector2 from, vector2 to);
    vector<float> makeDecision(NeuralNetwork& brain, vector<float>& inputs);
    CompetitiveSimulation(const CompetitiveSimulation& other){}
    CompetitiveSimulation& operator=(const CompetitiveSimulation& other){}

private:
    vector<SquareAgent*> mGoals;
    vector<LineObject*> mIndicators;
    SquareAgent* mAgent1;
    SquareAgent* mAgent2;
    unsigned int mA1Counter, mA2Counter;
    float mFit;
};

#endif