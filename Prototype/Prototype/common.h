#ifndef COMMON_H
#define COMMON_H

#include "SDL_opengl.h"

struct vector2
{
    vector2() : x(0.0f), y(0.0f){}
    GLfloat x;
    GLfloat y;
};

struct vector4
{
    vector4() : r(0.0f), g(0.0f), b(0.0f), a(0.0f) {}
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
};

struct vertex
{
    vector2 position;
    vector4 colour;
};

#endif