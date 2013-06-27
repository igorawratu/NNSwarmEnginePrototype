#ifndef TESTSIM_H
#define TESTSIM_H

#include "simulation.h"
#include "common.h"
#include "squareagent.h"
#include "lineobject.h"
#include "bar.h"

#include <iostream>
#include <vector>

using namespace std;

class TestSim : public Simulation
{
public:
    TestSim(SimulationParams parameters, bool renderable);
    virtual ~TestSim(){}

    virtual void reset(){}
    virtual void cycle(vector<NeuralNetwork> brains, unsigned int currentIteration);
    virtual float evaluateFitness(){return 0;}
    virtual void render(GLuint shadername);
    virtual void shutdown();
    virtual float getWinner(){return 0;}
    void conformVelocities();

private:
    TestSim(const TestSim& other){}
    TestSim& operator=(const TestSim& other){}

private:
    vector<Bar*> mIndicators;
    SquareAgent* mAgent1;
    SquareAgent* mAgent2;
};

#endif