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
//#include "testsim.h"

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
    params.simulationCycles = 2000;
    params.cyclesPerDecision = 10;
    params.maxFitness = 50;

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
        currParam.inputNodes = 8;
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
    parameters.epsilon = 0.f;
    parameters.crossoverType = MULTIPOINT_CO;
    parameters.nnParameters = getNNParameters();
    parameters.fullSample = true;

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
    GA ga(getGAParams());
    return ga.competePopulation(getSimParams());
    //return ga.train(getSimParams());
    //return ga.competeSinglePopulation(getSimParams());
    //return ga.modularTrain(getSimParams());
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
    /*srand(time(0));
    vector<NeuralNetwork> brains = train();
    writeResults(brains);

    cout << "Training Complete" << endl;
    int x;
    cin >> x;*/

    float nn1arr[] = {-0.825058, -0.696383, 0.757351, -0.488224, 0.549728, -0.810019, -0.109508, 0.884546, 0.799048, -0.520293, -0.405717, 0.353594, -0.789409, 0.505215, 0.0895156, 0.441687, -0.0199979, 0.861421, 0.594235, 0.829232, -0.97101, 0.455204, 0.616869, -0.610116, 0.468723, 0.799554, 0.412295, 0.555807, -0.828597, 0.516307, -0.149389, -0.365386, 0.209212, -0.793437, 0.270489, 0.381828, 0.497885, 0.104969, -0.960546, 0.352848, 0.178879, 0.169277, -0.543861, -0.380089, 0.466825, 0.168425};
    float nn2arr[] = {0.999678, 0.982506, -0.649461, -0.486615, 0.230641, 0.588123, -0.58361, 0.599284, 0.323667, 0.9571, 0.0618323, -0.834203, 0.248436, 0.346645, -0.654766, -0.229155, 0.556544, -0.472474, 0.566702, 0.897871, -0.694602, 0.914802, 0.965008, 0.209288, -0.107019, -0.254142, -0.571086, 0.862113, 0.856385, -0.54685, 0.789859, 0.445858, -0.462875, -0.666925, 0.526796, -0.951232, -0.117217, -0.725477, -0.928305, -0.718139, -0.847776, 0.109865, 0.664844, -0.668771, -0.340497, 0.55717};

    vector<NeuralNetworkParameter> nnparams = getNNParameters();
    
    vector<float> nn1(nn1arr, nn1arr + sizeof(nn1arr) / sizeof(nn1arr[0]));
    vector<float> nn2(nn2arr, nn2arr + sizeof(nn2arr) / sizeof(nn2arr[0]));
    
    NeuralNetwork net1(nnparams[0]), net2(nnparams[1]);
   
    net1.setWeights(nn1); net2.setWeights(nn2);

    vector<NeuralNetwork> brains;
    brains.push_back(net1);
    brains.push_back(net2);


    Renderer renderer;
    renderer.initialize("Prototype", WIDTH, HEIGHT, BITDEPTH);
    unsigned int shadername = initShader(renderer);
    CompetitiveSimulation sim(getSimParams(), true);
    //TestSim sim(getSimParams(), true);
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