#include "simulation.h"

float Simulation::calcDistanceSquared(vector2 from, vector2 to)
{
	float x = from.x - to.x;
	float y = from.y - to.y;
	return x*x + y*y;
}

void Simulation::iterate(vector<Object*>& objects, NeuralNetwork brain, vector2 goal)
{
    #pragma omp parallel for
	for(int k = 0; k < objects.size(); k++)
	{
		vector<float> inputs, output;
		bool status;

        float x, y, gx, gy;
        x = objects[k]->getPosition().x/((float)WIDTH / 2.0f) - 1.0f;
        y = objects[k]->getPosition().y/((float)HEIGHT / 2.0f) - 1.0f;
        gx = goal.x/((float)WIDTH / 2.0f) - 1.0f;
        gy = goal.y/((float)HEIGHT / 2.0f) - 1.0f;

		inputs.push_back(x); inputs.push_back(y);
		inputs.push_back(gx); inputs.push_back(gy);

		output = brain.evaluate(inputs, status);
		assert(status);

		vector2 acceleration;
		acceleration.x = (output[0] - 0.5f) / 5;
		acceleration.y = (output[1] - 0.5f) / 5;

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
			if(sqrt(calcDistanceSquared(objects[i]->getPosition(), goal)) < EPSILON)
				objects[i]->setReached(true);
		iterate(objects, brain, goal);
	}

	for(unsigned int k = 0; k < objects.size(); k++)
	{
		float negationAmount;
		if(objects[k]->getReached())
			negationAmount = 1.0f;
		else
		{
			float distance = sqrt(calcDistanceSquared(objects[k]->getPosition(), goal));
			negationAmount = (distance < MAXDISTANCE) ? 1.0f - (distance/MAXDISTANCE) : 0.0f;	
		}

		finalFitness -= negationAmount;
	}

	return finalFitness;
}