#ifndef LINEOBJECT_H
#define LINEOBJECT_H

#define NO_SDL_GLEXT

#include "gl/glew.h"
#include "SDL_opengl.h"

#include "object.h"
#include "common.h"

class LineObject: public Object
{
public:
    LineObject(vector2 position, vector2 first, vector2 second, bool renderable);
    LineObject(const LineObject& other);
    virtual ~LineObject(){}
    const LineObject& operator=(const LineObject& other);

    virtual void update(){}
    virtual void render(GLuint shadername);
    virtual void reset(){}
    vector2 getPosition(){return mPosition;}

private:
    LineObject(){}

private:
    vector2 mPosition, mFirst, mSecond;

};


#endif