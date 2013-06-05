#ifndef BAR_H
#define BAR_H

#define NO_SDL_GLEXT

#include "gl/glew.h"
#include "SDL_opengl.h"

#include "object.h"
#include "common.h"

#include <btBulletDynamicsCommon.h>
#include "BulletCollision/CollisionShapes/btConvex2dShape.h"

class Bar: public Object
{
public:
    Bar(vector2 first, vector2 second, vector2 third, vector2 fourth, bool renderable, btDiscreteDynamicsWorld* world);
    Bar(const Bar& other);
    virtual ~Bar(){}
    const Bar& operator=(const Bar& other);

    virtual void update(){}
    virtual void render(GLuint shadername);
    virtual void reset(){}
    vector2 getPosition(){return mPosition;}

private:
    Bar(){}
    void initPhysics(vector2 pos1, vector2 pos2, vector2 pos3, vector2 pos4);

private:
    vector2 mPosition, mFirst, mSecond, mThird, mFourth;

};

#endif