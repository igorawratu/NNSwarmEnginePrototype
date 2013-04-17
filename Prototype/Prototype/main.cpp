#define NO_SDL_GLEXT

#include <iostream>
#include <vector>
#include <time.h>
#include <fstream>

#include "gl/glew.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"

#include "object.h"
#include "common.h"
#include "simulation.h"
#include "ga.h"

using namespace std;

//const unsigned int WIDTH = 1600;
//const unsigned int HEIGHT = 900;
const unsigned int BITDEPTH = 32;

GLuint shadername;
vector<Object*> models;
Object* goalObject;
vector2 goal;
Simulation sim;
void initializeShader()
{
    GLint shaderCompiled;

    shadername = glCreateProgram();
    GLuint vsname = glCreateShader(GL_VERTEX_SHADER);
    GLuint fsname = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar* vssource = "void main(){gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex; gl_FrontColor = gl_Color;}";
    const GLchar* fssource = "void main(){gl_FragColor = gl_Color;}";

    glShaderSource(vsname, 1, &vssource, NULL);
    glCompileShader(vsname);

    glGetShaderiv(vsname, GL_COMPILE_STATUS, &shaderCompiled );
    if(!shaderCompiled)
    {
        cout << "Error compiling vertex shader" << endl;
        return;
    }

    glShaderSource(fsname, 1, &fssource, NULL);
    glCompileShader(fsname);
    glGetShaderiv(fsname, GL_COMPILE_STATUS, &shaderCompiled );
    if(!shaderCompiled)
    {
        cout << "Error compiling pixel shader" << endl;
        return;
    }

    glAttachShader(shadername, vsname);
    glAttachShader(shadername, fsname);
    glLinkProgram(shadername);
}

bool initializeOpenGL()
{
    GLenum glewError = glewInit();
    if( glewError != GLEW_OK )
    {
        cout << "Error initializing GLEW: " << glewGetErrorString(glewError) << endl;
        return false;
    }

    if( !GLEW_VERSION_2_1 )
    {
        cout << "OpenGL 2.1 not supported" << endl;
        return false;
    }

    glViewport(0, 0, WIDTH, HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, WIDTH, HEIGHT, 0.0, 1.0, -1.0 );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor( 0.f, 0.f, 0.f, 1.f );

    GLenum error = glGetError();
    if( error != GL_NO_ERROR )
    {
        cout << "Error initializing OpenGL" << gluErrorString(error) << endl;
        return false;
    }
    initializeShader();
    return true;
}

void initializeModels(unsigned int seed, GAParams parameters)
{
	//use seed from training to generate positions
    vector2 velMax, velMin, moveMax, moveMin;
    vector4 colour;
    boost::mt19937 rngx(seed);
    boost::mt19937 rngy(seed * 2);
    boost::uniform_real<float> xDist(parameters.modelInitSpaceMin.x, parameters.modelInitSpaceMax.x);
    boost::uniform_real<float> yDist(parameters.modelInitSpaceMin.y, parameters.modelInitSpaceMax.y);
    boost::variate_generator<boost::mt19937, boost::uniform_real<float>> genx(rngx, xDist);
    boost::variate_generator<boost::mt19937, boost::uniform_real<float>> geny(rngy, yDist);

    colour.r = colour.b = 0;
    colour.g = colour.a = 1.0f;
    velMax.x = velMax.y = velMin.x = velMin.y = moveMax.x = moveMax.y = moveMin.x = moveMin.y = 0.0f;

    goalObject = new Object(goal, colour, velMax, velMin, moveMax, moveMin, true);

    for(unsigned int k = 0; k < parameters.simulationPopulation; k++)
    {
        vector2 pos;
        pos.x = genx();
        pos.y = geny();
        models.push_back(new Object(pos, parameters.modelColour, parameters.vMax, parameters.vMin, parameters.modelMoveSpaceMax, parameters.modelMoveSpaceMin, true));
    }
}

bool initialize(unsigned int seed, GAParams parameters)
{
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
        return false;

    if(!SDL_SetVideoMode(WIDTH, HEIGHT, BITDEPTH, SDL_OPENGL))
        return false;

    if(!initializeOpenGL())
        return false;

    SDL_WM_SetCaption("Prototype", NULL);

    initializeModels(seed, parameters);

    return true;
}

bool handleEvents()
{
    SDL_Event event;
    bool run = true;

    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT: 
                run = false;
                break;
            default:    
                break;
        }
    }
    return run;
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shadername);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    for(unsigned int k = 0; k < models.size(); k++)
        models[k]->render();

    goalObject->render();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glUseProgram(NULL);
    
    SDL_GL_SwapBuffers();
}

void update(NeuralNetwork brain, vector2 goal)
{
    sim.iterate(models, brain, goal);
}

bool frame(NeuralNetwork brain, vector2 goal, bool complete)
{
    bool run = handleEvents();
    if(!complete)
        update(brain, goal);
    render();

    return run;
}

void shutdown()
{
    if(shadername)
        glDeleteProgram(shadername);

    for(unsigned int k = 0; k < models.size(); k++)
    {
        delete models[k];
        models[k] = 0;
    }

    delete goalObject;
    goalObject = 0;

    SDL_Quit();
}

void getScenarioTwo(vector2& goal, GAParams& parameters)
{
    vector2 vMax, vMin, mmsMin, mmsMax, misMin, misMax;
    vector4 col;

    vMax.x = vMax.y = 3.0f; 
    vMin.x = vMin.y = -3.0f;
    mmsMax.x = (float)WIDTH;
    mmsMax.y = (float)HEIGHT;
    misMax.x = 50.0f; misMax.y = (float)HEIGHT;
    misMin.x = 49.9f; misMin.y = 0.0f;
    
    col.g = col.b = 0.0f;
    col.r = col.a = 1.0f;
    
    parameters.GApopulation = 50;
    parameters.simulationPopulation = 20;
    parameters.searchSpaceMax = 1.0f;
    parameters.searchSpaceMin = -1.0f;
    parameters.maxGenerations = 100;
    parameters.simulationCycles = 1000;
    parameters.nnInputs = 4;
    parameters.nnHiddens = 5;
    parameters.nnOutputs = 2;
    parameters.modelMoveSpaceMin = mmsMin;
    parameters.modelMoveSpaceMax = mmsMax;
    parameters.modelInitSpaceMin = misMin;
    parameters.modelInitSpaceMax = misMax;
    parameters.vMax = vMax;
    parameters.vMin = vMin;
    parameters.modelColour = col;
    parameters.maxFitness = (float)parameters.simulationPopulation;
    parameters.elitismCount = parameters.GApopulation/10;
    parameters.mutationProb = 0.05f;
    parameters.epsilon = 0.5f;
    parameters.crossoverType = MULTIPOINT_CO;
    goal.x = (float)WIDTH - 10.0f;
    goal.y = (float)HEIGHT/2;
}

void getScenarioOne(vector2& goal, GAParams& parameters)
{
    vector2 vMax, vMin, mmsMin, mmsMax, misMin, misMax;
    vector4 col;

    vMax.x = vMax.y = 2.0f; 
    vMin.x = vMin.y = -2.0f;
    mmsMax.x = (float)WIDTH;
    mmsMax.y = (float)HEIGHT;
    misMax.x = (float)WIDTH/2.0f; misMax.y = (float)HEIGHT;
    misMin.x = 0.0f; misMin.y = 0.0f;
    
    col.g = col.b = 0.0f;
    col.r = col.a = 1.0f;
    
    parameters.GApopulation = 50;
    parameters.simulationPopulation = 20;
    parameters.searchSpaceMax = 1.0f;
    parameters.searchSpaceMin = -1.0f;
    parameters.maxGenerations = 50;
    parameters.simulationCycles = 1000;
    parameters.nnInputs = 4;
    parameters.nnHiddens = 5;
    parameters.nnOutputs = 2;
    parameters.modelMoveSpaceMin = mmsMin;
    parameters.modelMoveSpaceMax = mmsMax;
    parameters.modelInitSpaceMin = misMin;
    parameters.modelInitSpaceMax = misMax;
    parameters.vMax = vMax;
    parameters.vMin = vMin;
    parameters.modelColour = col;
    parameters.maxFitness = (float)parameters.simulationPopulation;
    parameters.elitismCount = parameters.GApopulation/10;
    parameters.mutationProb = 0.05f;
    parameters.epsilon = 0.5f;
    parameters.crossoverType = MULTIPOINT_CO;
    goal.x = (float)WIDTH - 10.0f;
    goal.y = (float)HEIGHT/2;
}

void writeToFile(char* filename, NeuralNetwork nn, unsigned int seed)
{
    ofstream os;
    os.open(filename);

    os << "seed: " << seed << endl;
    os << "fitness: " << nn.getFitness() << endl;
    for(int k = 0; k < nn.getWeights().size(); k++)
        os << nn.getWeights()[k] << " ";
    os << endl;

    os.close();
}

int main(int argc, char* args[]) 
{
    /*srand(time(0));

    //TRAIN
    GAParams parameters;

    getScenarioOne(goal, parameters);

    unsigned int seed, counter = 0;
        
    GA ga(parameters);
    NeuralNetwork brain = ga.train(seed, goal);

    cout << "FITNESS: " << brain.getFitness() << endl;

    writeToFile("random_multipoint_5.txt", brain, seed);

    
    cout << "finished training" << endl;
    int x; cin >> x;*/

    //RUN
    GAParams parameters;
    getScenarioTwo(goal, parameters);
    
    


    //fixed starting pos, broken mutation, 0 hidden, 0.492689 fitness
    
    unsigned int seed = 1366186904;
    NeuralNetwork brain(4, 2, 0);
    static const float arr[] = {0.820145, 0.169595, -0.420525, 0.917414, 0.112781, -0.635511, 0.390111, 0.953923, 0.886033, -0.111409};






    //fixed starting pos, broken mutation, 5 hidden, 0.679159 fitness
    /*unsigned int seed = 1366187757;
    NeuralNetwork brain(4, 2, 5);
    static const float arr[] = {-0.962675, 0.588216, 0.49828, -0.0297721, -0.0875987,
        -0.139209, -0.89658, 0.841324, 0.416497, 0.0187634, -0.315081, -0.351639, -0.493444,
        -0.542943, -0.364325, -0.73212, 0.835666, 0.127941, 0.0145293, 0.47307, -0.7409,
        -0.66698, 0.010776, -0.320704, 0.116337, -0.471666, 0.346973, -0.330152, 0.722695,
        0.865675, 0.618766, -0.81991, 0.770683, 0.458277, -0.928521, 0.926467, -0.0196038};*/
    




    //fixed starting pos, 3 hidden, 0.4282 fitness
    
    /*unsigned int seed = 1366197956;
    NeuralNetwork brain(4, 2, 3);
    static const float arr[] = {-0.00341332, 0.901713, -0.973966, -0.461915, 0.861362, 0.588173, 0.609768,
        -0.885908, 0.543338, -0.021608, -0.857131, -0.544013, -0.0334977, -0.689121, -0.940414, -0.707547,
        -0.630562, 0.609461, -0.287555, -0.821912, -0.407458, 0.838727, -0.277651};*/




    //fixed starting pos, 0 hidden, 0.391756 fitness, bound
    /*
    unsigned int seed = 1366201929;
    NeuralNetwork brain(4, 2, 0);
    static const float arr[] = {-0.503379, 0.132398, -0.430728, 0.683958, 0.704276, 0.539303, 0.104466, 0.103953, -0.662985, 0.364969};*/




    //fixed starting pos, 5 hidden, 0.18 fitness, bound
    
    /*unsigned int seed = 1366202228;
    NeuralNetwork brain(4, 2, 5);
    static const float arr[] = {-0.364195, -0.647887, -0.89466, 0.408244, -0.415702, 0.719333, 
    -0.177986, 0.715218, -0.56539, -0.306142, 0.25556, -0.268706, 0.0852885, 0.605348, 0.423719, 
    0.750521, -0.499322, 0.304426, 0.653066, -0.198795, 0.717026, -0.42312, -0.769437, 0.54964, 
    -0.639983, -0.241765, -0.00169611, -0.693809, 0.68473, -0.573217, 0.990824, -0.344518, 0.303608, 
    0.749428, 0.782001, 0.358236, 0.773391};*/



    /*GAParams parameters;
    getScenarioOne(goal, parameters);*/



    //random starting pos, 0 hidden, 1.1081 fitness
    /*
    unsigned int seed = 1366205005;
    NeuralNetwork brain(4, 2, 0);
    static const float arr[] = {0.306409, 0.0228944, 0.230332, -0.429737, 0.405686, 0.183583, 0.392145, 0.195283, 0.803369, 0.572016};*/

    //random starting pos,  hidden, 0.874406 fitness
    /*
    unsigned int seed = 1366208190;
    NeuralNetwork brain(4, 2, 5);
    static const float arr[] = {-0.908552, 0.435966, 0.89764, 0.857999, -0.515941, -0.922256, 0.852167, 0.88024, -0.281442, 
    -0.0168102, 0.379159, -0.599959, 0.441494, 0.154943, -0.67363, -0.330456, -0.868506, 0.896452, 0.401422, -0.0199213, 0.944812, 
    0.887258, -0.100087, 0.220059, -0.921285, 0.461758, -0.00183128, 0.0741204, 0.075393, -0.550676, -0.0316504, -0.268472, -0.748215, 
    0.541304, 0.992816, -0.408214, -0.307868};*/


    


    
    //test code
    vector<float> weights(arr, arr + sizeof(arr)/sizeof(arr[0]));
    brain.setWeights(weights);
    unsigned int counter = 0;
    initialize(seed, parameters);

    bool run = true;
    

    while(run)
        run = frame(brain, goal, counter++ >= parameters.simulationCycles);

    shutdown();

    return 0; 
}