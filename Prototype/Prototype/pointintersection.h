#ifndef POINTINTERSECTION_H
#define POINTINTERSECTION_H

#include "simulation.h"
#include "common.h"
#include "squareagent.h"

#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"

#include <iostream>
#include <vector>
#include <omp.h>

using namespace std;

const float EPSILON = 5.f;

struct PointIntersectParams
{
    vector2 modelMoveSpaceMin;
    vector2 modelMoveSpaceMax;
    vector2 modelInitSpaceMin;
    vector2 modelInitSpaceMax;
    vector2 velMax;
    vector2 velMin;
    bool checkBounds;
};

class PointIntersection: public Simulation
{
public:
    PointIntersection(PointIntersectParams piParams, SimulationParams parameters, unsigned int seed, vector2 goal, bool renderable);
    virtual ~PointIntersection();

    virtual void reset();
    virtual void cycle(vector<NeuralNetwork> brains, unsigned int currentIteration);
    virtual float evaluateFitness();
    virtual void render(GLuint shadername);

private:
    float calcDistance(vector2 from, vector2 to);
    vector<float> makeDecision(NeuralNetwork& brain, vector<float>& inputs);
    PointIntersection(const PointIntersection& other){}
    PointIntersection& operator=(const PointIntersection& other){}

private:
    //for now assume 1 type of agent
    vector<SquareAgent*> mAgents;
    SquareAgent* mGoal;
    unsigned int mSeed;
    PointIntersectParams mPIParams;
};

#endif