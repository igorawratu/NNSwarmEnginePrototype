#ifndef COMMON_H
#define COMMON_H

struct vector2
{
    vector2() : x(0.0f), y(0.0f){}
    float x;
    float y;
};

struct vector4
{
    vector4() : r(0.0f), g(0.0f), b(0.0f), a(0.0f) {}
    float r;
    float g;
    float b;
    float a;
};

struct vertex
{
    vector2 position;
    vector4 colour;
};

#endif