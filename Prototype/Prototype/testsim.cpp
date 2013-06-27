#include "testsim.h"

void testsimTickCallBack(btDynamicsWorld* world, btScalar timeStep)
{
    TestSim* tsim = (TestSim*)world->getWorldUserInfo();
    tsim->conformVelocities();
}

TestSim::TestSim(SimulationParams parameters, bool renderable) : Simulation(parameters, renderable)
{
    vector2 modelVelMax, modelVelMin, modelMoveMax, modelMoveMin;

    vector2 a1pos, a2pos; vector4 a1col, a2col;
    a1col.g = a2col.g = 0;
    a1col.r = 0; a2col.r = 1.0f;
    a1col.b = 1.0f; a2col.b = 0.0f;
    a1col.a = a2col.a = 1.0f;
    a1pos.x = 250; a1pos.y = 150;
    a2pos.x = 1000; a2pos.y = 150;
    modelVelMax.x = 2.0f;
    modelVelMax.y = 2.0f;
    modelVelMin.x = -2.0f;
    modelVelMin.y = -2.0f;
    modelMoveMin.x = modelMoveMin.y = 0.0f;
    modelMoveMax.x = 1600;
    modelMoveMax.y = 900;

    mAgent1 = new SquareAgent(a1pos, a1col, modelVelMax, modelVelMin, modelMoveMax, modelMoveMin, false, mRenderable, mWorld);
    mAgent2 = new SquareAgent(a2pos, a2col, modelVelMax, modelVelMin, modelMoveMax, modelMoveMin, false, mRenderable, mWorld);

    mWorld->setInternalTickCallback(testsimTickCallBack, this, true);
}

void TestSim::cycle(vector<NeuralNetwork> brains, unsigned int currentIteration)
{
    mWorld->stepSimulation(1.f, 1, 1.f);
    if(currentIteration % parameters.cyclesPerDecision == 0)
    {
        vector2 accel1, accel2;
        accel1.x = 1;
        accel2.x = -1;

        mAgent1->changeVelocity(accel1);
        mAgent2->changeVelocity(accel2);
    }
}

void TestSim::render(GLuint shadername)
{
    for(unsigned int k = 0; k < mIndicators.size(); k++)
        mIndicators[k]->render(shadername);

    mAgent1->render(shadername);
    mAgent2->render(shadername);
}

void TestSim::shutdown()
{
    for(unsigned int k = 0; k < mIndicators.size(); k++)
    {
        if(mIndicators[k])
        {
            delete mIndicators[k];
            mIndicators[k] = 0;
        }
    }
    mIndicators.clear();

    if(mAgent1)
    {
        delete mAgent1;
        mAgent1 = 0;
    }

    if(mAgent2)
    {
        delete mAgent2;
        mAgent2 = 0;
    }
}

void TestSim::conformVelocities()
{
    mAgent1->conformVelocities();
    mAgent2->conformVelocities();
}