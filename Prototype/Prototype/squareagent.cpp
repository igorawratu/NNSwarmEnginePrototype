#include "squareagent.h"

SquareAgent::SquareAgent(vector2 position, vector4 colour, vector2 velMax, vector2 velMin, vector2 moveMax, vector2 moveMin, bool boundsCheck, bool renderable) : Object(renderable)
{
    mInitPos = mPosition = position;
    mColour = colour;
    mVelocity.x = mVelocity.y = 0.0f;
    mVelMax = velMax;
    mVelMin = velMin;
	mReached = false;
    mMoveMax = moveMax;
    mMoveMin = moveMin;
    mBoundsCheck = boundsCheck;

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
    mPosition = other.mPosition;
    mBoundsCheck = other.mBoundsCheck;
    mColour = other.mColour;
    mVelocity = other.mVelocity;
    mVelMax = other.mVelMax;
    mVelMin = other.mVelMin;
	mRenderable = other.mRenderable;
	mReached = other.mReached;
    mMoveMin = other.mMoveMin;
    mMoveMax = other.mMoveMax;

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
    mPosition = other.mPosition;
    mColour = other.mColour;
    mVelocity = other.mVelocity;
    mVelMax = other.mVelMax;
    mVelMin = other.mVelMin;
	mRenderable = other.mRenderable;
	mReached = other.mReached;
    mMoveMin = other.mMoveMin;
    mMoveMax = other.mMoveMax;

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
    mPosition.x += mVelocity.x;
    mPosition.y += mVelocity.y;

    //bounds checking
    if(mBoundsCheck)
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
    }
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
    glTranslatef(mPosition.x, mPosition.y, 0.0f);
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
    if(mVelocity.x + acceleration.x > mVelMax.x)
        mVelocity.x = mVelMax.x;
    else if(mVelocity.x + acceleration.x < mVelMin.x)
        mVelocity.x = mVelMin.x;
    else mVelocity.x += acceleration.x;

    if(mVelocity.y + acceleration.y > mVelMax.y)
        mVelocity.y = mVelMax.y;
    else if(mVelocity.y + acceleration.y < mVelMin.y)
        mVelocity.y = mVelMin.y;
    else mVelocity.y += acceleration.y;
}

void SquareAgent::reset()
{
    mPosition = mInitPos;
    mVelocity.x = mVelocity.y = 0.0f;
}