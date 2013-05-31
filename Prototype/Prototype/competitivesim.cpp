#include "competitivesim.h"

void tickCallBack(btDynamicsWorld* world, btScalar timeStep)
{
    CompetitiveSimulation* csim = (CompetitiveSimulation*)world->getWorldUserInfo();
    csim->conformVelocities();
}

CompetitiveSimulation::CompetitiveSimulation(SimulationParams parameters, bool renderable) : Simulation(parameters, renderable)
{
    mWinner = -1;
    //create goal object
    vector2 gVelMax, gVelMin, gMoveMax, gMoveMin, modelVelMax, modelVelMin, modelMoveMax, modelMoveMin;
    vector4 gColour;
    gColour.r = gColour.b = 0;
    gColour.g = gColour.a = 1.0f;
    gVelMin.x = gVelMin.y =  gVelMax.x = gVelMax.y = gMoveMax.x = gMoveMax.y = gMoveMin.x = gMoveMin.y = 0.0f;

    vector2 gTopPos1, gTopPos2, gTopPos3, gTopPos4, gTopPos5, gTopPos6;
    vector2 gBotPos1, gBotPos2, gBotPos3, gBotPos4, gBotPos5, gBotPos6;
    vector2 frontStopPos1, frontStopPos2, backStopPos1, backStopPos2;

    gTopPos1.x = 50; gTopPos1.y = 200;
    gTopPos2.x = 400; gTopPos2.y = 250;
    gTopPos3.x = 700; gTopPos3.y = 100;
    gTopPos4.x = 800; gTopPos4.y = 150;
    gTopPos5.x = 1200; gTopPos5.y = 150;
    gTopPos6.x = 1500; gTopPos6.y = 300;

    gBotPos1.x = 50; gBotPos1.y = 600;
    gBotPos2.x = 400; gBotPos2.y = 550;
    gBotPos3.x = 700; gBotPos3.y = 700;
    gBotPos4.x = 800; gBotPos4.y = 750;
    gBotPos5.x = 1200; gBotPos5.y = 750;
    gBotPos6.x = 1500; gBotPos6.y = 600;

    frontStopPos1 = gTopPos1;
    frontStopPos2 = gBotPos1;

    mIndicators.push_back(new LineObject(vector2(), gTopPos1, gTopPos2, mRenderable, mWorld));
    mIndicators.push_back(new LineObject(vector2(), gTopPos2, gTopPos3, mRenderable, mWorld));
    mIndicators.push_back(new LineObject(vector2(), gTopPos3, gTopPos4, mRenderable, mWorld));
    mIndicators.push_back(new LineObject(vector2(), gTopPos4, gTopPos5, mRenderable, mWorld));
    mIndicators.push_back(new LineObject(vector2(), gTopPos5, gTopPos6, mRenderable, mWorld));

    mIndicators.push_back(new LineObject(vector2(), gBotPos1, gBotPos2, mRenderable, mWorld));
    mIndicators.push_back(new LineObject(vector2(), gBotPos2, gBotPos3, mRenderable, mWorld));
    mIndicators.push_back(new LineObject(vector2(), gBotPos3, gBotPos4, mRenderable, mWorld));
    mIndicators.push_back(new LineObject(vector2(), gBotPos4, gBotPos5, mRenderable, mWorld));
    mIndicators.push_back(new LineObject(vector2(), gBotPos5, gBotPos6, mRenderable, mWorld));

    mIndicators.push_back(new LineObject(vector2(), frontStopPos1, frontStopPos2, mRenderable, mWorld));


    vector2 a1pos, a2pos; vector4 a1col, a2col;
    a1col.g = a2col.g = 0;
    a1col.r = 0; a2col.r = 1.0f;
    a1col.b = 1.0f; a2col.b = 0.0f;
    a1col.a = a2col.a = 1.0f;
    a1pos.x = 100; a1pos.y = 400;
    a2pos.x = 100; a2pos.y = 420;
    modelVelMax.x = 2.0f;
    modelVelMax.y = 2.0f;
    modelVelMin.x = -2.0f;
    modelVelMin.y = -2.0f;
    modelMoveMin.x = modelMoveMin.y = 0.0f;
    modelMoveMax.x = 1600;
    modelMoveMax.y = 900;

    mAgent1 = new SquareAgent(a1pos, a1col, modelVelMax, modelVelMin, modelMoveMax, modelMoveMin, false, mRenderable, mWorld);
    mAgent2 = new SquareAgent(a2pos, a2col, modelVelMax, modelVelMin, modelMoveMax, modelMoveMin, false, mRenderable, mWorld);

    //mWorld->setWorldUserInfo(this);
    mWorld->setInternalTickCallback(tickCallBack, this);
}

void CompetitiveSimulation::shutdown()
{
    for(unsigned int k = 0; k < mIndicators.size(); k++)
    {
        if(mIndicators[k])
        {
            delete mIndicators[k];
            mIndicators[k] = 0;
        }
    }

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

void CompetitiveSimulation::reset()
{
    mWorld->clearForces();
    mAgent1->reset();
    mAgent2->reset();
}

void CompetitiveSimulation::cycle(vector<NeuralNetwork> brains, unsigned int currentIteration)
{
    if(currentIteration > parameters.simulationCycles)
        return;

    cout << "1: " << mAgent1->getPosition().x << " " << mAgent1->getPosition().y << " 2: " << mAgent2->getPosition().x << " " << mAgent2->getPosition().y << endl;

    if(currentIteration % parameters.cyclesPerDecision == 0)
    {
        //agent1
        vector<float> a1inputs, a1output, a2inputs, a2output;
        float a1x, a1y, gx, a2x, a2y;
        a1x = mAgent1->getPosition().x/(800.f) - 1.0f;
        a1y = mAgent1->getPosition().y/(450.f) - 1.0f;
        a2x = mAgent2->getPosition().x/(800.f) - 1.0f;
        a2y = mAgent2->getPosition().y/(450.f) - 1.0f;
        gx = 1500.f/(800.f) - 1.0f;

        a1inputs.push_back(a1x); a1inputs.push_back(a1y);
        a1inputs.push_back(gx);

        a2inputs.push_back(a2x); a2inputs.push_back(a2y);
        a2inputs.push_back(gx);

        a1output = makeDecision(brains[0], a1inputs);
        a2output = makeDecision(brains[1], a2inputs);

        vector2 accel1, accel2;

        accel1.x = (a1output[0] - 0.5f)/2;
        accel1.y = (a1output[1] - 0.5f)/2;

        accel2.x = (a2output[0] - 0.5f)/2;
        accel2.y = (a2output[1] - 0.5f)/2;

        mAgent1->changeVelocity(accel1);
        mAgent2->changeVelocity(accel2);
    }

    if(mAgent1->getVelocity().x > 2 || mAgent1->getVelocity().x < -2)
    {
        cout << "VEL ERROR: " << mAgent1->getVelocity().x << endl;
        int x;
        cin >> x;
    }

	mWorld->stepSimulation(1.f, 10, 1/10.f);

    if(!mAgent1->getReached())
    {
        if(mAgent1->getPosition().x >= 1500)
        {
            mAgent1->setReached(true);
        }
    }
    
    if(!mAgent2->getReached())
    {
        if(mAgent2->getPosition().x >= 1500)
            mAgent2->setReached(true);
    }

    if(mWinner == -1)
    {
        if(mAgent1->getReached())
            mWinner = 0;
        else if(mAgent2->getReached())
            mWinner = 1;
    }

    
}

void CompetitiveSimulation::conformVelocities()
{
    mAgent1->changeVelocity(vector2());
    mAgent2->changeVelocity(vector2());
}

float CompetitiveSimulation::evaluateFitness()
{
    float fitness = 0.f;

    if(!mAgent1->getReached())
        fitness += 1500.f - mAgent1->getPosition().x;

    if(!mAgent2->getReached())
        fitness += 1500.f - mAgent2->getPosition().x;

    return fitness;
}

void CompetitiveSimulation::render(GLuint shadername)
{
    for(unsigned int k = 0; k < mIndicators.size(); k++)
        mIndicators[k]->render(shadername);

    mAgent1->render(shadername);
    mAgent2->render(shadername);
}

float CompetitiveSimulation::calcDistance(vector2 from, vector2 to)
{
	float x = from.x - to.x;
	float y = from.y - to.y;
	return sqrt(x*x + y*y);
}

vector<float> CompetitiveSimulation::makeDecision(NeuralNetwork& brain, vector<float>& inputs)
{
    bool status;
    vector<float> output;

    output = brain.evaluate(inputs, status);
	assert(status);

    return output;
}