#ifndef LINEOBJECT_H
#define LINEOBJECT_H

#define NO_SDL_GLEXT

#include "gl/glew.h"
#include "SDL_opengl.h"

#include "object.h"
#include "common.h"

#include <btBulletDynamicsCommon.h>
#include "BulletCollision/CollisionShapes/btConvex2dShape.h"

class LineObject: public Object
{
public:
    LineObject(vector2 position, vector2 first, vector2 second, bool renderable, btDiscreteDynamicsWorld* world);
    LineObject(const LineObject& other);
    virtual ~LineObject(){}
    const LineObject& operator=(const LineObject& other);

    virtual void update(){}
    virtual void render(GLuint shadername);
    virtual void reset(){}
    vector2 getPosition(){return mPosition;}

private:
    LineObject(){}
    void initPhysics(vector2 pos1, vector2 pos2);

private:
    vector2 mPosition, mFirst, mSecond;

};


#endif