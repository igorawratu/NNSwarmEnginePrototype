#include <iostream>
#include <vector>
#include <time.h>

#include "object.h"
#include "common.h"
#include "simulation.h"
#include "ga.h"

using namespace std;

const unsigned int WIDTH = 1600;
const unsigned int HEIGHT = 900;
const unsigned int BITDEPTH = 32;


int main(int argc, char* args[]) 
{
    const GLchar* vssource = "void main(){gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex; gl_FrontColor = gl_Color;}";
    const GLchar* fssource = "void main(){gl_FragColor = gl_Color;}";

    //go in simulation
    
    //end

    return 0; 
}