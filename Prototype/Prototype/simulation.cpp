#include "simulation.h"

float Simulation::calcDistanceSquared(vector2 from, vector2 to)
{
	float x = from.x - to.x;
	float y = from.y - to.y;
	return x*x + y*y;
}

void Simulation::iterate(vector<Object*>& objects, NeuralNetwork brain, vector2 goal)
{
	for(unsigned int k = 0; k < objects.size(); k++)
	{
		vector<float> inputs, output;
		bool status;

		inputs.push_back(objects[k]->getPosition().x); inputs.push_back(objects[k]->getPosition().y);
		inputs.push_back(goal.x); inputs.push_back(goal.y);

		output = brain.evaluate(inputs, status);
		assert(status);

		vector2 acceleration;
		acceleration.x = output[0];
		acceleration.y = output[1];

		objects[k]->changeVelocity(acceleration);

		objects[k]->update();
	}
}

float Simulation::run(unsigned int cycles, NeuralNetwork brain, vector<Object*> objects, vector2 goal)
{
	float finalFitness = objects.size();
	for(unsigned int k = 0; k < cycles; k++)
	{
		for(unsigned int i = 0; i < objects.size(); i++)
			if(calcDistanceSquared(objects[i]->getPosition(), goal) < EPSILON)
				objects[i]->reached();
		iterate(objects, brain, goal);
	}

	for(unsigned int k = 0; k < objects.size(); k++)
	{
		float negationAmount;
		if(objects[k]->getReached())
			negationAmount = 1.0f;
		else
		{
			float distanceSquared = calcDistanceSquared(objects[k]->getPosition(), goal);
			negationAmount = (distanceSquared < MAXDISTANCESQUARED) ? 1.0f - (distanceSquared/MAXDISTANCESQUARED) : 0.0f;	
		}

		finalFitness -= negationAmount;
	}

	return finalFitness;
}