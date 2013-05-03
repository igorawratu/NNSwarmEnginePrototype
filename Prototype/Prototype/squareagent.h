#ifndef SQUAREAGENT_H
#define SQUAREAGENT_H

#define NO_SDL_GLEXT

#include "gl/glew.h"
#include "SDL_opengl.h"

#include "object.h"
#include "common.h"

class SquareAgent: public Object
{
public:
    SquareAgent(vector2 position, vector4 colour, vector2 velMax, vector2 velMin, vector2 moveMax, vector2 moveMin, bool boundsCheck, bool renderable);
    SquareAgent(const SquareAgent& other);
    virtual ~SquareAgent(){}
    const SquareAgent& operator=(const SquareAgent& other);

    virtual void update();
    virtual void render(GLuint shadername);

    void changeVelocity(vector2 acceleration);
    virtual void reset();
    vector2 getPosition(){return mPosition;}
    vector2 getVelocity(){return mVelocity;}
    void setReached(bool reached){mReached = reached;}
    bool getReached(){return mReached;}

private:
    SquareAgent(){}

private:
    vector2 mInitPos;
    vector2 mPosition, mVelocity, mVelMax, mVelMin, mMoveMax, mMoveMin;
    vector4 mColour;
    bool mReached, mBoundsCheck;

};

#endif