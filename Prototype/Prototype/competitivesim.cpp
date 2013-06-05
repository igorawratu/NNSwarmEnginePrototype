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

    vector2 pt1, pt2, pt3, pt4, pt5, pt6, pt7, pt8;
    vector2 pte1, pte2, pte3, pte4, pte5, pte6, pte7, pte8;

    pt1.x = 190; pt1.y = 100;
    pt2.x = 200; pt2.y = 110;
    pt3.x = 1290; pt3.y = 110;
    pt4.x = 1300; pt4.y = 100;
    pt5.x = 1300; pt5.y = 710;
    pt6.x = 1290; pt6.y = 700;
    pt7.x = 200; pt7.y = 700;
    pt8.x = 190; pt8.y = 710;

    pte1.x = 200; pte1.y = 200;
    pte2.x = 190; pte2.y = 210;
    pte3.x = 1190; pte3.y = 210;
    pte4.x = 1200; pte4.y = 200;
    pte5.x = 1200; pte5.y = 610;
    pte6.x = 1190; pte6.y = 600;
    pte7.x = 190; pte7.y = 600;
    pte8.x = 200; pte8.y = 610;

    mIndicators.push_back(new Bar(pt1, pt2, pt3, pt4, renderable, mWorld));
    mIndicators.push_back(new Bar(pt3, pt4, pt5, pt6, renderable, mWorld));
    mIndicators.push_back(new Bar(pt5, pt6, pt7, pt8, renderable, mWorld));
    mIndicators.push_back(new Bar(pte1, pte2, pte3, pte4, renderable, mWorld));
    mIndicators.push_back(new Bar(pte3, pte4, pte5, pte6, renderable, mWorld));
    mIndicators.push_back(new Bar(pte5, pte6, pte7, pte8, renderable, mWorld));
    mIndicators.push_back(new Bar(pte1, pte2, pt1, pt2, renderable, mWorld));
    mIndicators.push_back(new Bar(pte8, pte7, pt8, pt7, renderable, mWorld));
    
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

    if(currentIteration % parameters.cyclesPerDecision == 0)
    {
        vector<float> a1inputs, a1output, a2inputs, a2output;
        float a1x, a1y, a2x, a2y;
        a1x = mAgent1->getPosition().x/(800.f) - 1.0f;
        a1y = mAgent1->getPosition().y/(450.f) - 1.0f;
        a2x = mAgent2->getPosition().x/(800.f) - 1.0f;
        a2y = mAgent2->getPosition().y/(450.f) - 1.0f;


        //agent 1 inputs
        a1inputs.push_back(a1x); a1inputs.push_back(a1y);
        a1inputs.push_back(mAgent1->getVelocity().x/2); a1inputs.push_back(mAgent1->getVelocity().y/2);

        btVector3 a1FromTop(a1x, a1y - 10, 0), a1FromBot(a1x, a1y + 10, 0), a1FromRight(a1x + 10, a1y, 0), a1FromLeft(a1x - 10, a1y, 0);
        btVector3 a1ToTop(a1x, 0, 0), a1ToBot(a1x, 1000, 0), a1ToRight(2000, a1y, 0), a1ToLeft(0, a1y, 0);

        btCollisionWorld::ClosestRayResultCallback a1Top(a1FromTop,a1ToTop);
        btCollisionWorld::ClosestRayResultCallback a1Bot(a1FromBot,a1ToBot);
        btCollisionWorld::ClosestRayResultCallback a1Right(a1FromLeft,a1ToLeft);
        btCollisionWorld::ClosestRayResultCallback a1Left(a1FromRight,a1ToRight);
        
        mWorld->rayTest(a1FromTop, a1ToTop, a1Top);
        mWorld->rayTest(a1FromBot, a1ToBot, a1Bot);
        mWorld->rayTest(a1FromLeft, a1ToLeft, a1Left);
        mWorld->rayTest(a1FromRight, a1ToRight, a1Right);

        vector2 a1p1, a1p2, a1p3, a1p4;
        a1p1.x = a1Top.m_hitPointWorld.getX(); a1p1.y = a1Top.m_hitPointWorld.getY();
        a1p2.x = a1Bot.m_hitPointWorld.getX(); a1p2.y = a1Bot.m_hitPointWorld.getY();
        a1p3.x = a1Left.m_hitPointWorld.getX(); a1p3.y = a1Left.m_hitPointWorld.getY();
        a1p4.x = a1Right.m_hitPointWorld.getX(); a1p4.y = a1Right.m_hitPointWorld.getY();

        a1inputs.push_back(calcDistance(mAgent1->getPosition(), a1p1)/1600);
        a1inputs.push_back(calcDistance(mAgent1->getPosition(), a1p2)/1600);
        a1inputs.push_back(calcDistance(mAgent1->getPosition(), a1p3)/1600);
        a1inputs.push_back(calcDistance(mAgent1->getPosition(), a1p4)/1600);


        //agent 2 inputs
        a2inputs.push_back(a2x); a2inputs.push_back(a2y);
        a2inputs.push_back(mAgent2->getVelocity().x/2); a2inputs.push_back(mAgent2->getVelocity().y/2);

        btVector3 a2FromTop(a2x, a2y - 10, 0), a2FromBot(a2x, a2y + 10, 0), a2FromRight(a2x + 10, a2y, 0), a2FromLeft(a2x - 10, a2y, 0);
        btVector3 a2ToTop(a2x, 0, 0), a2ToBot(a2x, 1000, 0), a2ToRight(2000, a2y, 0), a2ToLeft(0, a2y, 0);

        btCollisionWorld::ClosestRayResultCallback a2Top(a2FromTop,a2ToTop);
        btCollisionWorld::ClosestRayResultCallback a2Bot(a2FromBot,a2ToBot);
        btCollisionWorld::ClosestRayResultCallback a2Right(a2FromLeft,a2ToLeft);
        btCollisionWorld::ClosestRayResultCallback a2Left(a2FromRight,a2ToRight);

        mWorld->rayTest(a2FromTop, a2ToTop, a2Top);
        mWorld->rayTest(a2FromBot, a2ToBot, a2Bot);
        mWorld->rayTest(a2FromLeft, a2ToLeft, a2Left);
        mWorld->rayTest(a2FromRight, a2ToRight, a2Right);

        vector2 a2p1, a2p2, a2p3, a2p4;
        a2p1.x = a2Top.m_hitPointWorld.getX(); a2p1.y = a2Top.m_hitPointWorld.getY();
        a2p2.x = a2Bot.m_hitPointWorld.getX(); a2p2.y = a2Bot.m_hitPointWorld.getY();
        a2p3.x = a2Left.m_hitPointWorld.getX(); a2p3.y = a2Left.m_hitPointWorld.getY();
        a2p4.x = a2Right.m_hitPointWorld.getX(); a2p4.y = a2Right.m_hitPointWorld.getY();

        a2inputs.push_back(calcDistance(mAgent2->getPosition(), a2p1)/1600);
        a2inputs.push_back(calcDistance(mAgent2->getPosition(), a2p2)/1600);
        a2inputs.push_back(calcDistance(mAgent2->getPosition(), a2p3)/1600);
        a2inputs.push_back(calcDistance(mAgent2->getPosition(), a2p4)/1600);

        
        //PRODUCE OUTPUTS
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

	mWorld->stepSimulation(1.f, 1, 1.f);

    if(!mAgent1->getReached())
    {
        if(mAgent1->getPosition().x < 220 && mAgent1->getPosition().y > 550)
            mAgent1->setReached(true);
    }
    
    if(!mAgent2->getReached())
    {
        if(mAgent2->getPosition().x < 220 && mAgent2->getPosition().y > 550)
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

float CompetitiveSimulation::getDistanceLeft(unsigned int agent)
{
    SquareAgent* currAgent;
    currAgent = (agent == 0) ? mAgent1 : mAgent2;

    if(currAgent->getReached())
        return 0;

    vector2 pt1, pt2, pt3;
    pt1.x = 1250; pt1.y = 150;
    pt2.x = 1250; pt2.y = 650;
    pt3.x = 200; pt3.y = 150;

    float d12 = calcDistance(pt1, pt2);
    float d23 = calcDistance(pt2, pt3);

    float dist = 0.f;

    if(currAgent->getPosition().y < 210)
        dist = calcDistance(currAgent->getPosition(), pt1) + d12 + d23;
    else if(currAgent->getPosition().y > 210 && currAgent->getPosition().y < 600)
        dist = calcDistance(currAgent->getPosition(), pt2) + d23;
    else dist = calcDistance(currAgent->getPosition(), pt3);

    return dist;
}

float CompetitiveSimulation::evaluateFitness()
{
    float agent1Dist = getDistanceLeft(0);
    float agent2Dist = getDistanceLeft(1);

    return agent1Dist + agent2Dist;
}

float CompetitiveSimulation::getWinner()
{
    if(mWinner != -1)
        return (float)mWinner;
    else
    {
        float agent1Dist = getDistanceLeft(0);
        float agent2Dist = getDistanceLeft(1);
       
        if(agent1Dist < agent2Dist)
            return 0.f;
        else return 1.f;
    }
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