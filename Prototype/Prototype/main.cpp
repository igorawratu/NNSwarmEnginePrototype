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
    srand(time(0));
    vector<NeuralNetwork> brains = train();
    writeResults(brains);

    cout << "Training Complete" << endl;
    int x;
    cin >> x;

    /*float nn1arr[] = {-0.600325, -0.955708, 0.131099, -0.152405, 0.6987, -0.284608, -0.961363, -0.677511, -0.214166, -0.97123, -0.82613, 0.645638, 0.760167, 0.939826, -0.359226, 0.146609, 0.584369, 0.394726, -0.0724368, -0.154672, 0.232506, -0.0417971, -0.498176, 0.997211, -0.467764, -0.721246, -0.276149, -0.791591, -0.8479, -0.0531352, 0.292705, -0.105955, 0.932536, -0.34401, -0.348585, 0.784849, -0.594151, -0.0769656, -0.570194, 0.167252, 0.649068, -0.665998, 0.381108, -0.648369, -0.354223, 0.592474};
    float nn2arr[] = {-0.918774, -0.974917, -0.0168329, -0.368724, 0.216464, -0.661003, 0.297891, -0.488792, -0.397176, -0.87236, -0.760075, 0.98823, 0.726488, -0.197215, -0.756088, -0.508356, -0.662527, -0.558881, 0.919155, 0.723619, -0.718172, -0.465103, 0.244265, 0.0430748, -0.271626, -0.218841, 0.653789, -0.804341, 0.394736, -0.670597, -0.396339, -0.523917, -0.373759, 0.610848, -0.480892, -0.784781, 0.845845, 0.868076, -0.706313, -0.537385, 0.197601, 0.238983, 0.416856, 0.120493, 0.237904, 0.664627};

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