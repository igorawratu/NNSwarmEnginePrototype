#ifndef COMMON_H
#define COMMON_H

struct vector2
{
    GLfloat x;
    GLfloat y;
};

struct vector4
{
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