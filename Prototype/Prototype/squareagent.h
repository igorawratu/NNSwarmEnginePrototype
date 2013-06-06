#ifndef SQUAREAGENT_H
#define SQUAREAGENT_H

#define NO_SDL_GLEXT

#include "gl/glew.h"
#include "SDL_opengl.h"

#include "object.h"
#include "common.h"

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btBox2dShape.h>
#include "BulletCollision/CollisionShapes/btConvex2dShape.h"

class SquareAgent: public Object
{
public:
    SquareAgent(vector2 position, vector4 colour, vector2 velMax, vector2 velMin, vector2 moveMax, vector2 moveMin, bool boundsCheck, bool renderable, btDiscreteDynamicsWorld* world);
    SquareAgent(const SquareAgent& other);
    virtual ~SquareAgent(){}
    const SquareAgent& operator=(const SquareAgent& other);

    virtual void update();
    virtual void render(GLuint shadername);

    void changeVelocity(vector2 acceleration);
    virtual void reset();
    vector2 getPosition()
    {
        vector2 pos;

        btTransform trans;
        mRigidBody->getMotionState()->getWorldTransform(trans);

        pos.x = trans.getOrigin().getX();
        pos.y = trans.getOrigin().getY();
        
        return pos;
    }
    vector2 getVelocity()
    {
        vector2 velocity;
        velocity.x = mRigidBody->getLinearVelocity().getX();
        velocity.y = mRigidBody->getLinearVelocity().getY();

        return velocity;
    }
    void setReached(bool reached){mReached = reached;}
    bool getReached(){return mReached;}
    void conformVelocities();

private:
    SquareAgent(){}
    void initPhys(vector2 position);

private:
    vector2 mInitPos;
    vector2 mVelMax, mVelMin, mMoveMax, mMoveMin;
    vector4 mColour;
    bool mReached, mBoundsCheck;

};

#endif