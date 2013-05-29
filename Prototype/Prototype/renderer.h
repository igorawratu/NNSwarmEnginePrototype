#ifndef RENDERER_H
#define RENDERER_H

#define NO_SDL_GLEXT

#include "gl/glew.h"
#include "SDL.h"
#include "SDL_opengl.h"

#include <vector>

#include "simulation.h"

using namespace std;

class Renderer
{
public:
    Renderer(){}
    ~Renderer();
    GLuint initializeShader(const GLchar* vsSource, const GLchar* fsSource);
    bool initialize(GLchar* windowname, const unsigned int width, const unsigned int height, const unsigned int bitdepth);
    void render(Simulation* simulation, unsigned int shadername);

    //when this returns false, terminate program
    bool handleEvents();
    void shutdown();

private:
    bool initializeOpenGL(const unsigned int width, const unsigned int height);
    Renderer(const Renderer& other);
    Renderer& operator=(const Renderer& other){}

private:
    vector<GLuint> shaders;
};

#endif