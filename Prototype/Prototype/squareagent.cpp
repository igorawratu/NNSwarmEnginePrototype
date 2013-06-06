#include "squareagent.h"

void SquareAgent::initPhys(vector2 position)
{
    if(mColShape)
        delete mColShape;
    
    if(mRigidBody)
    {
        mWorld->removeRigidBody(mRigidBody);

        if(mRigidBody->getMotionState())
            delete mRigidBody->getMotionState();

        delete mRigidBody;
    }

    btMotionState* motionState;

    btVector3 points[4] = {btVector3(-5.0f, 5.0f, 0), btVector3(-5.0f, -5.0f, 0), btVector3(5.0f, -5.0f, 0), btVector3(5.0f, 5.0f, 0)};

    btConvexShape* childShape = new btConvexHullShape(&points[0].getX(), 4);
    mColShape = new btConvex2dShape(childShape);

    motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(position.x, position.y, 0)));
    
    btVector3 inertia(0, 0, 0);
    mColShape->calculateLocalInertia(1, inertia);

    btRigidBody::btRigidBodyConstructionInfo consInf(1, motionState, mColShape, inertia);
    consInf.m_restitution = 10.f;
    consInf.m_friction = 1.f;
    mRigidBody = new btRigidBody(consInf);
    mRigidBody->setSleepingThresholds(0.f, mRigidBody->getAngularSleepingThreshold());

    mWorld->addRigidBody(mRigidBody);
}

void SquareAgent::conformVelocities()
{
    vector2 newVel;
    btVector3 velocity = mRigidBody->getLinearVelocity();

    if(velocity.getX() > mVelMax.x)
        newVel.x = mVelMax.x;
    else if(velocity.getX() < mVelMin.x)
        newVel.x = mVelMin.x;
    else newVel.x = velocity.getX();

    if(velocity.getY() > mVelMax.y)
        newVel.y = mVelMax.y;
    else if(velocity.getY() < mVelMin.y)
        newVel.y = mVelMin.y;
    else newVel.y = velocity.getY();

    mRigidBody->setLinearVelocity(btVector3(newVel.x, newVel.y, 0.f));
}

SquareAgent::SquareAgent(vector2 position, vector4 colour, vector2 velMax, vector2 velMin, vector2 moveMax, vector2 moveMin, bool boundsCheck, bool renderable, btDiscreteDynamicsWorld* world) : Object(renderable, world)
{
    mInitPos = position;
    mColour = colour;
    mVelMax = velMax;
    mVelMin = velMin;
	mReached = false;
    mMoveMax = moveMax;
    mMoveMin = moveMin;
    mBoundsCheck = boundsCheck;

    initPhys(mInitPos);

	if(mRenderable)
    {
        vertex vertices[4];
        GLuint indices[4];

        vertices[0].position.x = -5.0f;
        vertices[0].position.y = 5.0f;

        vertices[1].position.x = -5.0f;
        vertices[1].position.y = -5.0f;

        vertices[2].position.x = 5.0f;
        vertices[2].position.y = -5.0f;

        vertices[3].position.x = 5.0f;
        vertices[3].position.y = 5.0f;

        vertices[0].colour = vertices[1].colour = vertices[2].colour = vertices[3].colour = mColour;

        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;
        indices[3] = 3;

		initBuffers(vertices, 4, indices, 4);
    }
}

SquareAgent::SquareAgent(const SquareAgent& other)
{
    mInitPos = other.mInitPos;
    mVbname = mIbname = 0;
    mBoundsCheck = other.mBoundsCheck;
    mColour = other.mColour;
    mVelMax = other.mVelMax;
    mVelMin = other.mVelMin;
	mRenderable = other.mRenderable;
	mReached = other.mReached;
    mMoveMin = other.mMoveMin;
    mMoveMax = other.mMoveMax;

    initPhys(mInitPos);

	if(mRenderable)
    {
        vertex vertices[4];
        GLuint indices[4];

        vertices[0].position.x = -5.0f;
        vertices[0].position.y = 5.0f;

        vertices[1].position.x = -5.0f;
        vertices[1].position.y = -5.0f;

        vertices[2].position.x = 5.0f;
        vertices[2].position.y = -5.0f;

        vertices[3].position.x = 5.0f;
        vertices[3].position.y = 5.0f;

        vertices[0].colour = vertices[1].colour = vertices[2].colour = vertices[3].colour = mColour;

        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;
        indices[3] = 3;

		initBuffers(vertices, 4, indices, 4);
    }
}

const SquareAgent& SquareAgent::operator=(const SquareAgent& other)
{
    if(mVbname)
        glDeleteBuffers(1, &mVbname);
    if(mIbname)
        glDeleteBuffers(1, &mIbname);

    mVbname = mIbname = 0;
    mInitPos = other.mInitPos;
    mBoundsCheck = other.mBoundsCheck;
    mColour = other.mColour;
    mVelMax = other.mVelMax;
    mVelMin = other.mVelMin;
	mRenderable = other.mRenderable;
	mReached = other.mReached;
    mMoveMin = other.mMoveMin;
    mMoveMax = other.mMoveMax;

    initPhys(mInitPos);

	if(mRenderable)
    {
        vertex vertices[4];
        GLuint indices[4];

        vertices[0].position.x = -5.0f;
        vertices[0].position.y = 5.0f;

        vertices[1].position.x = -5.0f;
        vertices[1].position.y = -5.0f;

        vertices[2].position.x = 5.0f;
        vertices[2].position.y = -5.0f;

        vertices[3].position.x = 5.0f;
        vertices[3].position.y = 5.0f;

        vertices[0].colour = vertices[1].colour = vertices[2].colour = vertices[3].colour = mColour;

        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;
        indices[3] = 3;

		initBuffers(vertices, 4, indices, 4);
    }

    return *this;
}
    
void SquareAgent::update()
{
    //bounds checking
    /*if(mBoundsCheck)
    {
        if(mPosition.x > mMoveMax.x)
        {
            mPosition.x -= 2 * (mPosition.x - mMoveMax.x);
            mVelocity.x = -mVelocity.x;
        }
        else if(mPosition.x < mMoveMin.x)
        {
            mPosition.x += 2 * (mMoveMin.x - mPosition.x);
            mVelocity.x = -mVelocity.x;
        }
        
        if(mPosition.y > mMoveMax.y)
        {
            mPosition.y -= 2 * (mPosition.y - mMoveMax.y);
            mVelocity.y = -mVelocity.y;
        }
        else if(mPosition.y < mMoveMin.y)
        {
            mPosition.y += 2 * (mMoveMin.y - mPosition.y);
            mVelocity.y = -mVelocity.y;
        }
    }*/
}

void SquareAgent::render(GLuint shadername)
{
	if(!mRenderable)
		return;

    glUseProgram(shadername);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glLoadIdentity();

    btTransform trans;
    mRigidBody->getMotionState()->getWorldTransform(trans);

    glTranslatef(trans.getOrigin().getX(), trans.getOrigin().getY(), 0.0f); 

    glPushMatrix();

    glBindBuffer(GL_ARRAY_BUFFER, mVbname);
    glVertexPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)offsetof(vertex, position));
    glColorPointer(4, GL_FLOAT, sizeof(vertex), (GLvoid*)offsetof(vertex, colour));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbname);

    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glUseProgram(NULL);
}
    
void SquareAgent::changeVelocity(vector2 acceleration)
{
    mRigidBody->applyCentralForce(btVector3(acceleration.x, acceleration.y, 0));
}

void SquareAgent::reset()
{
    mRigidBody->clearForces();
    mRigidBody->setLinearVelocity(btVector3(0, 0, 0));
    mRigidBody->setAngularVelocity(btVector3(0, 0, 0));
    btTransform transform = mRigidBody->getCenterOfMassTransform();
    transform.setOrigin(btVector3(mInitPos.x, mInitPos.y, 0));
    mRigidBody->setCenterOfMassTransform(transform);

    mReached = false;
}