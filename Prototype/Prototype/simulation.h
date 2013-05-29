#ifndef SIMULATION_H
#define SIMULATION_H

#include "neuralnetwork.h"

#include <vector>
#include <btBulletDynamicsCommon.h>

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
    Simulation(SimulationParams parameters, bool renderable)
    {
        this->parameters = parameters; 
        mRenderable = renderable;

        mBroadPhase = new btDbvtBroadphase();
        mColConfig = new btDefaultCollisionConfiguration();
        mDispatcher = new btCollisionDispatcher(mColConfig);
        mSolver = new btSequentialImpulseConstraintSolver();
        mWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadPhase, mSolver, mColConfig);

        mWorld->setGravity(btVector3(0, 0.f, 0));
    }
    Simulation(const Simulation& other){}
    Simulation& operator=(const Simulation& other){}
    virtual ~Simulation()
    {
        if(mWorld)
        {
            delete mWorld;
            mWorld = 0;
        }
        
        if(mSolver)
        {
            delete mSolver;
            mSolver = 0;
        }
        
        if(mDispatcher)
        {
            delete mDispatcher;
            mDispatcher = 0;
        }
        
        if(mColConfig)
        {
            delete mColConfig;
            mColConfig = 0;
        }
        
        if(mBroadPhase)
        {
            delete mBroadPhase;
            mBroadPhase = 0;
        }
    }

    virtual void reset() = 0;
    virtual void cycle(vector<NeuralNetwork> brains, unsigned int currentIteration) = 0;
    void fullRun(vector<NeuralNetwork> brains)
    {
        for(unsigned int k = 0; k < parameters.simulationCycles; k++)
            cycle(brains, k);
    }
    virtual float evaluateFitness()=0;
    virtual void render(unsigned int shadername)=0;
    virtual void shutdown() = 0;

public:
    SimulationParams parameters;

protected:
    bool mRenderable;

    btBroadphaseInterface* mBroadPhase;
    btDefaultCollisionConfiguration* mColConfig;
    btCollisionDispatcher* mDispatcher;
    btSequentialImpulseConstraintSolver* mSolver;
    btDiscreteDynamicsWorld* mWorld;

protected:
    Simulation(){}
};

#endif