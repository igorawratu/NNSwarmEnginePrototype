#define NO_SDL_GLEXT

#include <iostream>
#include <vector>

#include "gl/glew.h"
#include "SDL.h"
#include "SDL_opengl.h"

#include "object.h"
#include "common.h"
#include "simulation.cpp"

using namespace std;

const unsigned int WIDTH = 1600;
const unsigned int HEIGHT = 900;
const unsigned int BITDEPTH = 32;

GLuint shadername;
vector<Object*> models;

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

void initializeModels()
{
	//use seed from training to generate positions
    vector2 position, velMax, velMin;
    vector4 colour;

    colour.r = colour.b = 0;
    colour.g = colour.a = 1.0f;
    position.x = WIDTH/2;
    position.y = HEIGHT/2;
    velMin.x = velMin.y = -1;
    velMax.x = velMax.y = 1;

    models.push_back(new Object(position, colour, velMax, velMin));
    models.push_back(new Object(position, colour, velMax, velMin));

    models[0]->changeVelocity(velMax);
    models[1]->changeVelocity(velMin);
}

bool initialize()
{
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
        return false;

    if(!SDL_SetVideoMode(WIDTH, HEIGHT, BITDEPTH, SDL_OPENGL))
        return false;

    if(!initializeOpenGL())
        return false;

    SDL_WM_SetCaption("Prototype", NULL);

    initializeModels();

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

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glUseProgram(NULL);
    
    SDL_GL_SwapBuffers();
}

void update(NeuralNetwork brain, vector2 goal)
{
    iterate(models, brain, goal);
}

bool frame(NeuralNetwork brain, vector2 goal)
{
    bool run = handleEvents();
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

    SDL_Quit();
}

int main(int argc, char* args[]) 
{
	NeuralNetwork brain;
	vector2 goal;
	goal.x = 0.0f;
	goal.y = 0.0f;

    initialize();
    bool run = true;

    while(run)
        run = frame(brain, goal);

    shutdown();

    return 0; 
}