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
    params.simulationCycles = 2000;
    params.cyclesPerDecision = 10;
    params.maxFitness = 10;

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

    /*float nn1arr[] = {0.955696, 0.822669, -0.913918, 0.512241, 0.121692, 0.129463, 0.621017, -0.361709, -0.419729, -0.772165, -0.719983, 0.885575, -0.755213, -0.85312, -0.0920819, 0.106075, 0.796243, 0.71326, 0.380059, 0.888765, 0.178002, -0.127265, 0.637866, 0.536565, -0.392735, 0.206322, -0.217566, 0.131958, -0.203522, 0.958159, -0.511264, 0.741468, 0.718051, -0.809618, -0.028028, 0.565684, -0.86132, 0.96757, -0.98516, 0.660639, 0.230979, 0.830711, -0.603374, -0.18968, 0.673632, 0.635466};
    float nn2arr[] = {0.259016, 0.942077, 0.623125, -0.0397072, -0.267051, -0.259172, -0.254697, -0.727534, 0.0917113, -0.247898, -0.95674, 0.980413, 0.711877, -0.905414, 0.601102, -0.915542, 0.206491, -0.51084, -0.922636, -0.201825, 0.834181, -0.867011, -0.628239, -0.867472, 0.345988, 0.499718, -0.211031, -0.402126, 0.0337402, 0.915882, -0.837999, -0.996415, -0.544116, 0.574411, 0.344773, -0.621771, -0.647666, 0.844989, 0.91877, 0.750088, 0.809292, 0.00675181, 0.835602, -0.995568, 0.167752, 0.0864372};

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