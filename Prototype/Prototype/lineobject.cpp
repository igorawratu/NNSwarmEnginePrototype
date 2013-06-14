#include "lineobject.h"


void LineObject::initPhysics(vector2 pos1, vector2 pos2)
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
    btVector3 points[2] = {btVector3(pos1.x, pos1.y, 0), btVector3(pos2.x, pos2.y, 0)};

    mChildShape = new btConvexHullShape(&points[0].getX(), 2);
    mColShape = new btConvex2dShape(mChildShape);

    motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    
    btVector3 inertia(0, 0, 0);
    mColShape->calculateLocalInertia(0, inertia);

    btRigidBody::btRigidBodyConstructionInfo consInf(0, motionState, mColShape, inertia);
    mRigidBody = new btRigidBody(consInf);
    mRigidBody->setSleepingThresholds(0.f, mRigidBody->getAngularSleepingThreshold());

    mWorld->addRigidBody(mRigidBody);
}



LineObject::LineObject(vector2 position, vector2 first, vector2 second, bool renderable, btDiscreteDynamicsWorld* world) : Object(renderable, world)
{
    mPosition = position;
    mFirst = first;
    mSecond = second;

    initPhysics(first, second);

    if(mRenderable)
    {
        vertex vertices[2];
        GLuint indices[2];

        vertices[0].position.x = mFirst.x;
        vertices[0].position.y = mFirst.y;

        vertices[1].position.x = mSecond.x;
        vertices[1].position.y = mSecond.y;

        vector4 col;
        col.r = 1.0f; col.g = 1.0f; col.b = 1.0f; col.a = 1.0f;
        vertices[0].colour = vertices[1].colour = col;

        indices[0] = 0;
        indices[1] = 1;

		initBuffers(vertices, 2, indices, 2);
    }
}

LineObject::LineObject(const LineObject& other)
{
    mPosition = other.mPosition;
    mVbname = mIbname = 0;
    mRenderable = other.mRenderable;
    mFirst = other.mFirst;
    mSecond = other.mSecond;
    
    initPhysics(mFirst, mSecond);

    if(mRenderable)
    {
        vertex vertices[2];
        GLuint indices[2];

        vertices[0].position.x = mFirst.x;
        vertices[0].position.y = mFirst.y;

        vertices[1].position.x = mSecond.x;
        vertices[1].position.y = mSecond.y;

        vector4 col;
        col.r = 1.0f; col.g = 1.0f; col.b = 1.0f; col.a = 1.0f;
        vertices[0].colour = vertices[1].colour = col;

        indices[0] = 0;
        indices[1] = 1;

		initBuffers(vertices, 2, indices, 2);
    }
}

const LineObject& LineObject::operator=(const LineObject& other)
{
    if(mVbname)
        glDeleteBuffers(1, &mVbname);
    if(mIbname)
        glDeleteBuffers(1, &mIbname);

    mVbname = mIbname = 0;
    mRenderable = other.mRenderable;
    mPosition = other.mPosition;
    mFirst = other.mFirst;
    mSecond = other.mSecond;

    initPhysics(mFirst, mSecond);

    if(mRenderable)
    {
        vertex vertices[2];
        GLuint indices[2];

        vertices[0].position.x = mFirst.x;
        vertices[0].position.y = mFirst.y;

        vertices[1].position.x = mSecond.x;
        vertices[1].position.y = mSecond.y;

        vector4 col;
        col.r = 1.0f; col.g = 1.0f; col.b = 1.0f; col.a = 1.0f;
        vertices[0].colour = vertices[1].colour = col;

        indices[0] = 0;
        indices[1] = 1;

		initBuffers(vertices, 2, indices, 2);
    }

    return *this;
}

void LineObject::render(GLuint shadername)
{
    if(!mRenderable)
		return;

    glUseProgram(shadername);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glLoadIdentity();
    glTranslatef(mPosition.x, 0.0f, 0.0f);
    glPushMatrix();

    glBindBuffer(GL_ARRAY_BUFFER, mVbname);
    glVertexPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid*)offsetof(vertex, position));
    glColorPointer(4, GL_FLOAT, sizeof(vertex), (GLvoid*)offsetof(vertex, colour));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbname);

    glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glUseProgram(NULL);
}