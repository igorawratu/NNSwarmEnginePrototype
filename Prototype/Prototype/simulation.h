#ifndef SIMULATION_H
#define SIMULATION_H

#include "object.h"
#include "common.h"
#include "neuralnetwork.h"

#include <vector>
#include <iostream>
#include <assert.h>
#include <omp.h>

using namespace std;

const float EPSILON = 5.0f;
const unsigned int WIDTH = 1600;
const float MAXDISTANCE= (float)WIDTH * 2;
const unsigned int HEIGHT = 900;

class Simulation
{
public:
    Simulation(){}
    ~Simulation(){}
    Simulation(const Simulation& other){}
    Simulation& operator=(const Simulation& other){}

    void iterate(vector<Object*>& objects, NeuralNetwork brain, vector2 goal);
    float run(unsigned int cycles, NeuralNetwork brain, vector<Object*> objects, vector2 goal);

private:
    float calcDistanceSquared(vector2 from, vector2 to);
};

#endif