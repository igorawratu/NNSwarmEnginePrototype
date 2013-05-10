#ifndef OBJECT_H
#define OBJECT_H

#define NO_SDL_GLEXT

#include "gl/glew.h"
#include "SDL_opengl.h"

#include <iostream>

#include "common.h"

using namespace std;

class Object
{
public:
    //Object(vector2 position, vector4 colour, vector2 velMax, vector2 velMin, vector2 moveMax, vector2 moveMin, bool renderable);
    Object(bool renderable)
    {
        mRenderable = renderable;
        mVbname = mIbname = 0;
    }

    virtual ~Object()
    {
        if(mVbname)
            glDeleteBuffers(1, &mVbname);
        if(mIbname)
            glDeleteBuffers(1, &mIbname);
    }
    
    virtual void update()=0;
    virtual void render(GLuint shadername)=0;
    virtual void reset()=0;

protected:
    //does not delete the passed pointers, those need to either be deleted outside, or stack based
    void initBuffers(vertex* vertices, unsigned int vertexCount, GLuint* indices, unsigned int indexCount)
    {
        glGenBuffers(1, &mVbname);
        glBindBuffer(GL_ARRAY_BUFFER, mVbname);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(vertex), vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &mIbname);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbname);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLuint), indices, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, NULL);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
    }

    Object(){}
    Object(const Object& other){}
    Object& operator=(const Object& other){}

protected:
    GLuint mVbname, mIbname;
    bool mRenderable;
};

#endif