#ifndef OBJECT_H
#define OBJECT_H

#define NO_SDL_GLEXT

#include <iostream>

#include "gl/glew.h"
#include "SDL_opengl.h"

#include "common.h"


using namespace std;

class Object
{
public:
    Object();
    Object(vector2 position, vector4 colour, vector2 velMax, vector2 velMin, bool renderable);
    Object(const Object& other);
    ~Object();

    Object& operator=(const Object& other);
    
    void update();
    void render();
    
    void changeVelocity(vector2 acceleration);

    vector2 getPosition(){return mPosition;}
    void reached(){mReached = true;}
    bool getReached(){return mReached;}

private:
    void initBuffers();

private:
    vector2 mPosition, mVelocity, mVelMax, mVelMin;
    vector4 mColour;
    GLuint mVbname, mIbname;
    bool mRenderable, mReached;
};

#endif