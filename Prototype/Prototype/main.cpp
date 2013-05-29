#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "filereader.h"
#include "pointintersection.h"
#include "ga.h"
#include "neuralnetwork.h"
#include "renderer.h"
#include "competitivesim.h"

//#define MLEAKDEBUG

#ifdef MLEAKDEBUG
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>

    #ifdef _DEBUG
    #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
    #define new DEBUG_NEW
    #endif
#endif

using namespace std;

const unsigned int WIDTH = 1600;
const unsigned int HEIGHT = 900;
const unsigned int BITDEPTH = 32;

SimulationParams getSimParams()
{
    SimulationParams params;
    params.modelGroups = 2;
    params.simulationCycles = 1000;
    params.cyclesPerDecision = 10;
    params.maxFitness = 3000;

    return params;
}

vector<NeuralNetworkParameter> getNNParameters()
{
    vector<NeuralNetworkParameter> params;

    for(int k = 0; k < 2; k++)
    {
        NeuralNetworkParameter currParam;
        currParam.hiddenNodes = 4;
        currParam.outputNodes = 2;
        currParam.inputNodes = 3;
        params.push_back(currParam);
    }

    return params;
}

GAParams getGAParams()
{
    GAParams parameters;
    parameters.GApopulation = 50;
    parameters.searchSpaceMax = 1.f;
    parameters.searchSpaceMin = -1.f;
    parameters.maxGenerations = 100;
    parameters.elitismCount = 5;
    parameters.mutationProb = 0.1f;
    parameters.epsilon = 1.f;
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
    return ga.train(&sim, false);
}

void writeResults(vector<NeuralNetwork> results)
{
    ofstream writer;
    writer.open("result.txt", ios::app);

    for(int k = 0; k < results.size(); k++)
    {
        writer << "Neural Network " << k << ":" << endl;
        results[k].print(writer);
        writer << endl << endl;
    }

    writer.close();
}

int main(int argc, char* args[]) 
{
    srand(time(0));
    vector<NeuralNetwork> brains = train();
    writeResults(brains);

    cout << "Training Complete" << endl;
    int x;
    cin >> x;

    /*float nn1arr[] = {-0.568836, -0.569681, 0.334781, -0.910464, -0.902754, -0.399543, 0.0945043, -0.392398, -0.677449, -0.863273, -0.0671058, -0.772658, 0.0787617, -0.859671, -0.14264, 0.0955521, -0.831512, -0.306233, -0.138672, 0.481729, 0.251401, -0.862106, 0.858018, -0.70108, 0.717477, 0.222602, 0.403008, -0.188498, 0.252968, 0.0624793};
    float nn2arr[] = {-0.412925, 0.462164, -0.827935, 0.307628, -0.947465, -0.490749, -0.678349, 0.127849, -0.647005, -0.0217445, -0.128007, 0.862729, -0.759496, 0.301459, -0.779348, 0.598747, -0.512174, -0.440985, 0.998712, -0.347579, -0.713229, -0.504903, -0.825188, -0.736758, -0.0276753, 0.174392, 0.274847, -0.19574, -0.265751, -0.291668};

    vector<NeuralNetworkParameter> nnparams = getNNParameters();
    
    vector<float> nn1(nn1arr, nn1arr + sizeof(nn1arr) / sizeof(nn1arr[0]));
    vector<float> nn2(nn2arr, nn2arr + sizeof(nn2arr) / sizeof(nn2arr[0]));
    
    NeuralNetwork net1(nnparams[0]), net2(nnparams[1]);
   
    net1.setWeights(nn1); net2.setWeights(nn2);

    vector<NeuralNetwork> brains;
    brains.push_back(net1);
    brains.push_back(net2);*/


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


    sim.shutdown();
    renderer.shutdown();
    
#ifdef MLEAKDEBUG
    _CrtDumpMemoryLeaks();
#endif

    return 0; 
}