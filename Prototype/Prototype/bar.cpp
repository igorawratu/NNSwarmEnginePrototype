#include "bar.h"

void Bar::initPhysics(vector2 pos1, vector2 pos2, vector2 pos3, vector2 pos4)
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
    btVector3 points[4] = {btVector3(pos1.x, pos1.y, 0), btVector3(pos2.x, pos2.y, 0), btVector3(pos3.x, pos3.y, 0), btVector3(pos4.x, pos4.y, 0)};

    mChildShape = new btConvexHullShape(&points[0].getX(), 4);
    mColShape = new btConvex2dShape(mChildShape);

    motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    
    btVector3 inertia(0, 0, 0);
    mColShape->calculateLocalInertia(0, inertia);

    btRigidBody::btRigidBodyConstructionInfo consInf(0, motionState, mColShape, inertia);

    consInf.m_restitution = 0.5f;
    consInf.m_friction = 0.5f;

    mRigidBody = new btRigidBody(consInf);
    mRigidBody->activate(true);

    mWorld->addRigidBody(mRigidBody);
}



Bar::Bar(vector2 first, vector2 second, vector2 third, vector2 fourth, bool renderable, btDiscreteDynamicsWorld* world) : Object(renderable, world)
{
    mFirst = first;
    mSecond = second;
    mThird = third;
    mFourth = fourth;

    initPhysics(first, second, third, fourth);

    if(mRenderable)
    {
        vertex vertices[4];
        GLuint indices[4];

        vertices[0].position.x = mFirst.x;
        vertices[0].position.y = mFirst.y;

        vertices[1].position.x = mSecond.x;
        vertices[1].position.y = mSecond.y;

        vertices[2].position.x = mThird.x;
        vertices[2].position.y = mThird.y;

        vertices[3].position.x = mFourth.x;
        vertices[3].position.y = mFourth.y;

        vector4 col;
        col.r = 1.0f; col.g = 0.8f; col.b = 0.0f; col.a = 1.0f;

        for(int k = 0; k < 4; k++)
        {
            indices[k] = k;
            vertices[k].colour = col;
        }

		initBuffers(vertices, 4, indices, 4);
    }
}

Bar::Bar(const Bar& other)
{
    mPosition = other.mPosition;
    mVbname = mIbname = 0;
    mRenderable = other.mRenderable;
    mFirst = other.mFirst;
    mSecond = other.mSecond;
    mThird = other.mThird;
    mFourth = other.mFourth;
    
    initPhysics(mFirst, mSecond, mThird, mFourth);

    if(mRenderable)
    {
        vertex vertices[4];
        GLuint indices[4];

        vertices[0].position.x = mFirst.x;
        vertices[0].position.y = mFirst.y;

        vertices[1].position.x = mSecond.x;
        vertices[1].position.y = mSecond.y;

        vertices[2].position.x = mThird.x;
        vertices[2].position.y = mThird.y;

        vertices[3].position.x = mFourth.x;
        vertices[3].position.y = mFourth.y;

        vector4 col;
        col.r = 1.0f; col.g = 0.0f; col.b = 0.0f; col.a = 1.0f;

        for(int k = 0; k < 4; k++)
        {
            indices[k] = k;
            vertices[k].colour = col;
        }

		initBuffers(vertices, 4, indices, 4);
    }
}

const Bar& Bar::operator=(const Bar& other)
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
    mThird = other.mThird;
    mFourth = other.mFourth;
    
    initPhysics(mFirst, mSecond, mThird, mFourth);

    if(mRenderable)
    {
        vertex vertices[4];
        GLuint indices[4];

        vertices[0].position.x = mFirst.x;
        vertices[0].position.y = mFirst.y;

        vertices[1].position.x = mSecond.x;
        vertices[1].position.y = mSecond.y;

        vertices[2].position.x = mThird.x;
        vertices[2].position.y = mThird.y;

        vertices[3].position.x = mFourth.x;
        vertices[3].position.y = mFourth.y;

        vector4 col;
        col.r = 0.0f; col.g = 1.0f; col.b = 1.0f; col.a = 1.0f;

        for(int k = 0; k < 4; k++)
        {
            indices[k] = k;
            vertices[k].colour = col;
        }

		initBuffers(vertices, 4, indices, 4);
    }

    return *this;
}

void Bar::render(GLuint shadername)
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

    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glUseProgram(NULL);
}