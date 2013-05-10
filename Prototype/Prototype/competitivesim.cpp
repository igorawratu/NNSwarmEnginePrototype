#include "competitivesim.h"

CompetitiveSimulation::CompetitiveSimulation(SimulationParams parameters, bool renderable) : Simulation(parameters, renderable)
{
    mA1Counter = mA2Counter = 0;
    mFit = 0;
    //create goal object
    vector2 gVelMax, gVelMin, gMoveMax, gMoveMin, modelVelMax, modelVelMin, modelMoveMax, modelMoveMin;
    vector4 gColour;
    gColour.r = gColour.b = 0;
    gColour.g = gColour.a = 1.0f;
    gVelMax.x = gVelMax.y = gVelMin.x = gVelMin.y = gMoveMax.x = gMoveMax.y = gMoveMin.x = gMoveMin.y = 0.0f;

    vector2 gPos1, gPos2, gPos3, gPos4, gPos5;
    gPos1.x = 200; gPos1.y = 200;
    gPos2.x = 500; gPos2.y = 250;
    gPos3.x = 500; gPos3.y = 400;
    gPos4.x = 460; gPos4.y = 340;
    gPos5.x = 800, gPos5.y = 500;

    mGoals.push_back(new SquareAgent(gPos1, gColour, gVelMax, gVelMin, gMoveMax, gMoveMin, false, mRenderable));
    mGoals.push_back(new SquareAgent(gPos2, gColour, gVelMax, gVelMin, gMoveMax, gMoveMin, false, mRenderable));
    mGoals.push_back(new SquareAgent(gPos3, gColour, gVelMax, gVelMin, gMoveMax, gMoveMin, false, mRenderable));
    mGoals.push_back(new SquareAgent(gPos4, gColour, gVelMax, gVelMin, gMoveMax, gMoveMin, false, mRenderable));
    mGoals.push_back(new SquareAgent(gPos5, gColour, gVelMax, gVelMin, gMoveMax, gMoveMin, false, mRenderable));

    mIndicators.push_back(new LineObject(vector2(), gPos1, gPos2, mRenderable));
    mIndicators.push_back(new LineObject(vector2(), gPos2, gPos3, mRenderable));
    mIndicators.push_back(new LineObject(vector2(), gPos3, gPos4, mRenderable));
    mIndicators.push_back(new LineObject(vector2(), gPos4, gPos5, mRenderable));

    vector2 a1pos, a2pos; vector4 a1col, a2col;
    a1col.g = a2col.g = 0;
    a1col.r = 0; a2col.r = 1.0f;
    a1col.b = 1.0f; a2col.b = 0.0f;
    a1col.a = a2col.a = 1.0f;
    a1pos.x = 100; a1pos.y = 190;
    a2pos.x = 100; a2pos.y = 210;
    modelVelMax.x = 2.0f;
    modelVelMax.y = 2.0f;
    modelVelMin.x = -2.0f;
    modelVelMin.y = -2.0f;
    modelMoveMin.x = modelMoveMin.y = 0.0f;
    modelMoveMax.x = 1600;
    modelMoveMax.y = 900;

    mAgent1 = new SquareAgent(a1pos, a1col, modelVelMax, modelVelMin, modelMoveMax, modelMoveMin, false, mRenderable);
    mAgent2 = new SquareAgent(a2pos, a2col, modelVelMax, modelVelMin, modelMoveMax, modelMoveMin, false, mRenderable);
}

CompetitiveSimulation::~CompetitiveSimulation()
{
    for(unsigned int k = 0; k < mGoals.size(); k++)
    {
        if(mGoals[k])
        {
            delete mGoals[k];
            mGoals[k] = 0;
        }
    }

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
    mAgent1->reset();
    mAgent2->reset();
    mA1Counter = mA2Counter = 0;
    mFit = 0;
}

void CompetitiveSimulation::cycle(vector<NeuralNetwork> brains, unsigned int currentIteration)
{
    if(currentIteration > parameters.simulationCycles)
        return;

    if(currentIteration % parameters.cyclesPerDecision == 0)
    {
        //agent1
        if(mA1Counter < 5)
        {
            vector<float> inputs, output;
            float x, y, gx, gy, vx, vy;
            x = mAgent1->getPosition().x/(1600.0f / 2.0f) - 1.0f;
            y = mAgent1->getPosition().y/(900.0f / 2.0f) - 1.0f;
            gx = mGoals[mA1Counter]->getPosition().x/(1600.0f / 2.0f) - 1.0f;
            gy = mGoals[mA1Counter]->getPosition().y/(900.0f / 2.0f) - 1.0f;
            vx = mAgent1->getVelocity().x;
            vy = mAgent1->getVelocity().y;
	        inputs.push_back(x); inputs.push_back(y);
	        inputs.push_back(gx); inputs.push_back(gy);
            //inputs.push_back(vx); inputs.push_back(vy);
            output = makeDecision(brains[0], inputs);
	        vector2 acceleration, norm;
            norm.x = (mGoals[mA1Counter]->getPosition().x - mAgent1->getPosition().x) / (1600*3);
            norm.y = (mGoals[mA1Counter]->getPosition().y - mAgent1->getPosition().y) / (900*3);
	        acceleration.x = (((output[0] - 0.5f) / 3) + norm.x)/2;
	        acceleration.y = (((output[1] - 0.5f) / 3) + norm.y)/2;
	        mAgent1->changeVelocity(acceleration);
            if(calcDistance(mAgent1->getPosition(), mGoals[mA1Counter]->getPosition()) < 40.f)
	            mA1Counter++;
        }

        if(mA2Counter < 5)
        {
            vector<float> inputs, output;
            float x, y, gx, gy, vx, vy;
            x = mAgent2->getPosition().x/(1600.0f / 2.0f) - 1.0f;
            y = mAgent2->getPosition().y/(900.0f / 2.0f) - 1.0f;
            gx = mGoals[mA2Counter]->getPosition().x/(1600.0f / 2.0f) - 1.0f;
            gy = mGoals[mA2Counter]->getPosition().y/(900.0f / 2.0f) - 1.0f;
            vx = mAgent2->getVelocity().x;
            vy = mAgent2->getVelocity().y;
	        inputs.push_back(x); inputs.push_back(y);
	        inputs.push_back(gx); inputs.push_back(gy);
            //inputs.push_back(vx); inputs.push_back(vy);
            output = makeDecision(brains[1], inputs);
	        vector2 acceleration, norm;
            norm.x = (mGoals[mA2Counter]->getPosition().x - mAgent2->getPosition().x) / (1600/**3*/);
            norm.y = (mGoals[mA2Counter]->getPosition().y - mAgent2->getPosition().y) / (900/**3*/);
	        acceleration.x = (((output[0] - 0.5f)/* / 3*/) + norm.x)/2;
	        acceleration.y = (((output[1] - 0.5f)/* / 3*/) + norm.y)/2;
	        mAgent2->changeVelocity(acceleration);
            if(calcDistance(mAgent2->getPosition(), mGoals[mA2Counter]->getPosition()) < 40.f)
	            mA2Counter++;
        }
    }
    
    if(mA1Counter < mA2Counter)
        mFit += 1;
    if(5 > mA1Counter || 5 > mA2Counter)
    {
        //mFit += calcDistance(mAgent1->getVelocity(), mAgent2->getVelocity());
        mFit += (2.0f - fabs(mAgent1->getVelocity().x) + 2.0f - fabs(mAgent1->getVelocity().y))/4;
        mFit += (2.0f - fabs(mAgent2->getVelocity().x) + 2.0f - fabs(mAgent2->getVelocity().y))/4;
    }

	mAgent1->update();
    mAgent2->update();
    
}

float CompetitiveSimulation::evaluateFitness()
{
    float addAmount = (float)(10 - mA1Counter - mA2Counter) * 200;
    cout << mA1Counter << " " << mA2Counter << endl;
    return mFit + addAmount;
}

void CompetitiveSimulation::render(GLuint shadername)
{
    for(unsigned int k = 0; k < mGoals.size(); k++)
        mGoals[k]->render(shadername);

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