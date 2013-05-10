#include <iostream>
#include <string>
#include <vector>

#include "filereader.h"
#include "pointintersection.h"
#include "ga.h"
#include "neuralnetwork.h"
#include "renderer.h"
#include "competitivesim.h"

using namespace std;

const unsigned int WIDTH = 1600;
const unsigned int HEIGHT = 900;
const unsigned int BITDEPTH = 32;

SimulationParams getSimParams()
{
    SimulationParams params;
    params.modelGroups = 2;
    params.simulationCycles = 2000;
    params.cyclesPerDecision = 10;

    return params;
}

vector<NeuralNetworkParameter> getNNParameters()
{
    vector<NeuralNetworkParameter> params;

    for(int k = 0; k < 2; k++)
    {
        NeuralNetworkParameter currParam;
        currParam.hiddenNodes = 0;
        currParam.outputNodes = 2;
        currParam.inputNodes = 4;
        params.push_back(currParam);
    }

    return params;
}

GAParams getGAParams()
{
    GAParams parameters;
    parameters.GApopulation = 100;
    parameters.searchSpaceMax = 1.f;
    parameters.searchSpaceMin = -1.f;
    parameters.maxGenerations = 100;
    parameters.elitismCount = 10;
    parameters.mutationProb = 0.1f;
    parameters.epsilon = 0.f;
    parameters.crossoverType = MULTIPOINT_CO;
    parameters.nnParameters = getNNParameters();

    return parameters;
}

unsigned int initShader(Renderer& renderer)
{
    string vs, fs;
    FileReader fileReader;
    vs = fileReader.readText("colour2d.vs");
    fs = fileReader.readText("colour2d.fs");
    return renderer.initializeShader(vs.c_str(), fs.c_str());
}

vector<NeuralNetwork> train()
{
    CompetitiveSimulation sim(getSimParams(), false);
    GA ga(getGAParams());
    return ga.train(&sim);
}

int main(int argc, char* args[]) 
{
    vector<NeuralNetwork> brains = train();

    int x;
    cin >> x;

    Renderer renderer;
    renderer.initialize("Prototype", WIDTH, HEIGHT, BITDEPTH);
    unsigned int shadername = initShader(renderer);
    CompetitiveSimulation sim(getSimParams(), true);
    unsigned int counter = 0;
    while(renderer.handleEvents())
    {
        sim.cycle(brains, counter++);
        renderer.render(&sim, shadername);
    }

    return 0; 
}