#include "pointintersection.h"

PointIntersection::PointIntersection(PointIntersectParams piParams, SimulationParams parameters, unsigned int seed, vector2 goal, bool renderable) : Simulation(parameters, renderable)
{
    mSeed = seed;
    mPIParams = piParams;

    //initialisation of rngs
    boost::mt19937 rngx(mSeed);
    boost::mt19937 rngy(mSeed * 2);
    boost::uniform_real<float> xDist(mPIParams.modelInitSpaceMin.x, mPIParams.modelInitSpaceMax.x);
    boost::uniform_real<float> yDist(mPIParams.modelInitSpaceMin.y, mPIParams.modelInitSpaceMax.y);
    boost::variate_generator<boost::mt19937, boost::uniform_real<float>> genx(rngx, xDist);
    boost::variate_generator<boost::mt19937, boost::uniform_real<float>> geny(rngy, yDist);

    //create goal object
    vector2 gVelMax, gVelMin, gMoveMax, gMoveMin;
    vector4 gColour;
    gColour.r = gColour.b = 0;
    gColour.g = gColour.a = 1.0f;
    gVelMax.x = gVelMax.y = gVelMin.x = gVelMin.y = gMoveMax.x = gMoveMax.y = gMoveMin.x = gMoveMin.y = 0.0f;
    mGoal = new SquareAgent(goal, gColour, gVelMax, gVelMin, gMoveMax, gMoveMin, mPIParams.checkBounds, mRenderable);

    //initialise models, assume one type for now
    vector4 aColour;
    aColour.g = aColour.b = 0;
    aColour.r = aColour.a = 1.0f;
    for(unsigned int k = 0; k < parameters.simulationPopulation.size(); k++)
    {
        vector2 pos;
        pos.x = genx();
        pos.y = geny();
        
        mAgents.push_back(new SquareAgent(pos, aColour, mPIParams.velMax, mPIParams.velMin, mPIParams.modelMoveSpaceMax, mPIParams.modelMoveSpaceMin, mPIParams.checkBounds, mRenderable));
    }
}

PointIntersection::~PointIntersection()
{
    for(int k = 0; k < mAgents.size(); k++)
    {
        if(mAgents[k])
        {
            delete mAgents[k];
            mAgents[k] = 0;
        }
    }

    if(mGoal)
    {
        delete mGoal;
        mGoal = 0;
    }
}

void PointIntersection::reset()
{
    for(unsigned int k = 0; k < mAgents.size(); k++)  
        mAgents[k]->reset();

}

void PointIntersection::cycle(vector<NeuralNetwork> brains, unsigned int currentIteration)
{
    if(currentIteration > parameters.simulationCycles)
        return;

    #pragma omp parallel for
	for(int k = 0; k < mAgents.size(); k++)
	{
        if(currentIteration % parameters.cyclesPerDecision == 0)
        {
            vector<float> inputs, output;

            float x, y, gx, gy, vx, vy;
            x = mAgents[k]->getPosition().x/((float)mPIParams.modelMoveSpaceMax.x / 2.0f) - 1.0f;
            y = mAgents[k]->getPosition().y/((float)mPIParams.modelMoveSpaceMax.y / 2.0f) - 1.0f;
            gx = mGoal->getPosition().x/(mPIParams.modelMoveSpaceMax.x / 2.0f) - 1.0f;
            gy = mGoal->getPosition().y/(mPIParams.modelMoveSpaceMax.y / 2.0f) - 1.0f;
            vx = mAgents[k]->getVelocity().x;
            vy = mAgents[k]->getVelocity().y;

		    inputs.push_back(x); inputs.push_back(y);
		    inputs.push_back(gx); inputs.push_back(gy);
            inputs.push_back(vx); inputs.push_back(vy);

            output = makeDecision(brains[0], inputs);

		    vector2 acceleration;
		    acceleration.x = (output[0] - 0.5f) / 5;
		    acceleration.y = (output[1] - 0.5f) / 5;

		    mAgents[k]->changeVelocity(acceleration);
        }
        
		mAgents[k]->update();
        if(calcDistance(mAgents[k]->getPosition(), mGoal->getPosition()) < EPSILON)
		    mAgents[k]->setReached(true);
	}
}

float PointIntersection::evaluateFitness()
{
    //init fitness to worst possible fitness
    float finalFitness = parameters.maxFitness;
    float maxDistance = (mPIParams.modelMoveSpaceMax.x - mPIParams.modelMoveSpaceMin.x) * 2;

    //-1.0 if object has intersected, otherwise minus the appropriate amount normalised
	for(unsigned int k = 0; k < mAgents.size(); k++)
	{
		float negationAmount;
		if(mAgents[k]->getReached())
			negationAmount = 1.0f;
		else
		{
			float distance = calcDistance(mAgents[k]->getPosition(), mGoal->getPosition());
			negationAmount = (distance < maxDistance) ? 1.0f - (distance/maxDistance) : 0.0f;	
		}

		finalFitness -= negationAmount;
	}

	return finalFitness;
}

void PointIntersection::render(GLuint shadername)
{
    for(unsigned int k = 0; k < mAgents.size(); k++)
        mAgents[k]->render(shadername);

    mGoal->render(shadername);
}

float PointIntersection::calcDistance(vector2 from, vector2 to)
{
	float x = from.x - to.x;
	float y = from.y - to.y;
	return sqrt(x*x + y*y);
}

vector<float> PointIntersection::makeDecision(NeuralNetwork& brain, vector<float>& inputs)
{
    bool status;
    vector<float> output;

    output = brain.evaluate(inputs, status);
	assert(status);

    return output;
}