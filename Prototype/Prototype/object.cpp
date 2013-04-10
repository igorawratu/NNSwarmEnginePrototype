#include "object.h"

Object::Object()
{
    mVbname = mIbname = 0;
    mPosition.x = mPosition.y = 0.0f;
    mVelocity.x = mVelocity.y = 0.0f;
    mColour.r = mColour.a = 1.0f;
    mColour.g = mColour.b = 0.0f;
    mVelMax.x = mVelMax.y = 1.0f;
    mVelMin.x = mVelMin.y = -1.0f;
	mRenderable = true;
	mReached = false;
    mMoveMax.x = mMoveMax.y = 1.0f;
    mMoveMin.x = mMoveMin.y = -1.0f;

    initBuffers();
}

Object::Object(vector2 position, vector4 colour, vector2 velMax, vector2 velMin, vector2 moveMax, vector2 moveMin, bool renderable)
{
    mVbname = mIbname = 0;
    mPosition = position;
    mColour = colour;
    mVelocity.x = mVelocity.y = 0.0f;
    mVelMax = velMax;
    mVelMin = velMin;
	mRenderable = renderable;
	mReached = false;
    mMoveMax = moveMax;
    mMoveMin = moveMin;

	if(mRenderable)
		initBuffers();
}

Object::Object(const Object& other)
{
    mVbname = mIbname = 0;
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
		initBuffers();
}

Object::~Object()
{
    if(mVbname)
        glDeleteBuffers(1, &mVbname);
    if(mIbname)
        glDeleteBuffers(1, &mIbname);
}

Object& Object::operator=(const Object& other)
{
    if(mVbname)
        glDeleteBuffers(1, &mVbname);
    if(mIbname)
        glDeleteBuffers(1, &mIbname);

    mVbname = mIbname = 0;
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
		initBuffers();

    return *this;
}
    
void Object::update()
{
    mPosition.x += mVelocity.x;
    mPosition.y += mVelocity.y;

    //bounds checking
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

void Object::render()
{
	if(!mRenderable)
		return;

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
}
    
void Object::changeVelocity(vector2 acceleration)
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

void Object::initBuffers()
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

    glGenBuffers(1, &mVbname);
    glBindBuffer(GL_ARRAY_BUFFER, mVbname);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(vertex), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &mIbname);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIbname);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
}