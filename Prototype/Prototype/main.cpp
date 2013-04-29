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

void update(NeuralNetwork brain, vector2 goal, bool makeDecision)
{
    sim.iterate(models, brain, goal, makeDecision);
}

bool frame(NeuralNetwork brain, vector2 goal, bool complete, bool makeDecision)
{
    bool run = handleEvents();
    if(!complete)
        update(brain, goal, makeDecision);
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

void getParams(vector2& goal, GAParams& parameters)
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
    parameters.nnInputs = 6;
    parameters.nnHiddens = 0;
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

void writeToFile(char* filename, NeuralNetwork nn, unsigned int seed, unsigned int generations)
{
    ofstream os;
    os.open(filename, ios::app);

    os << "Seed: " << seed << endl;
    os << "Fitness: " << nn.getFitness() << endl;
    os << "Generations: " << generations << endl;
    for(int k = 0; k < nn.getWeights().size(); k++)
        os << nn.getWeights()[k] << " ";
    os << endl << endl << endl;

    os.close();
}

NeuralNetwork train(unsigned int& seed, GAParams& parameters)
{
    getParams(goal, parameters);
    //modify params here

    for(int i = 0; i < 6; i++)
    {
        parameters.nnHiddens = i;
        char outputFileName[30];
        sprintf(outputFileName, "%dhidden_multipoint_random.txt", i);

        for(int k = 0; k < 10; k++)
        {
            unsigned int generations;
            GA ga(parameters);
            NeuralNetwork brain = ga.train(seed, goal, generations);
            writeToFile(outputFileName, brain, seed, generations);
        }
    }

    return NeuralNetwork(parameters.nnInputs, parameters.nnOutputs, parameters.nnHiddens);
}

int main(int argc, char* args[]) 
{
    GAParams parameters;
    unsigned int seed, counter = 0;;


    //NeuralNetwork brain = train(seed, parameters);
    
    getParams(goal, parameters);
    seed = 0;
    NeuralNetwork brain(4, 0, 2);


    initialize(seed, parameters);

    while(frame(brain, goal, counter++ >= parameters.simulationCycles, counter % 10 == 0));

    shutdown();

    return 0; 
}