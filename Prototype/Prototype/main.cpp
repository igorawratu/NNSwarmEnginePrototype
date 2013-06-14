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

    /*float nn1arr[] = {0.118223, 0.199073, 0.400197, -0.672227, -0.224726, -0.573016, 0.759209, 0.890996, -0.484141, -0.207504, 0.655275, 0.341233, 0.0326525, -0.720563, 0.0395124, -0.42473, 0.346363, -0.567209, 0.146231, -0.222695, 0.767896, 0.212944, -0.783179, 0.125823, 0.419294, 0.847271, 0.0422829, 0.871248, -0.505691, 0.662619, -0.626254, 0.285511, 0.660648, 0.964871, -0.0652884, -0.177082, -0.388488, -0.962087, -0.480175, -0.731391, 0.570392, -0.214177, -0.00571135, -0.570746, -0.658133, -0.0017751};
    float nn2arr[] = {-0.76186, 0.873296, -0.951288, 0.456168, 0.631245, 0.816111, -0.66221, 0.711367, -0.570166, 0.884509, 0.520178, -0.449114, 0.734121, 0.882006, 0.961864, 0.51835, -0.249509, -0.0778012, -0.351222, -0.897363, 0.0242707, 0.810249, -0.455638, 0.600291, -0.553491, 0.0271124, -0.398036, -0.29852, -0.609456, 0.616675, -0.552414, 0.0645907, 0.399582, 0.404971, -0.498802, -0.73881, -0.153411, -0.632186, 0.698871, 0.686499, 0.205683, -0.217769, 0.669692, 0.707619, -0.187549, 0.550651};

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