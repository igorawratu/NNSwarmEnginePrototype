#include "renderer.h"

Renderer::~Renderer()
{
    for(unsigned int k = 0; k < shaders.size(); k++)
        glDeleteProgram(shaders[k]);

    SDL_Quit();
}
    
GLuint Renderer::initializeShader(const GLchar* vsSource, const GLchar* fsSource)
{
    GLint shaderCompiled;

    GLuint shadername = glCreateProgram();
    GLuint vsname = glCreateShader(GL_VERTEX_SHADER);
    GLuint fsname = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vsname, 1, &vsSource, NULL);
    glCompileShader(vsname);

    glGetShaderiv(vsname, GL_COMPILE_STATUS, &shaderCompiled );
    if(!shaderCompiled)
    {
        cerr << "Error compiling vertex shader" << endl;
        return -1;
    }

    glShaderSource(fsname, 1, &fsSource, NULL);
    glCompileShader(fsname);
    glGetShaderiv(fsname, GL_COMPILE_STATUS, &shaderCompiled );
    if(!shaderCompiled)
    {
        cerr << "Error compiling pixel shader" << endl;
        return -1;
    }

    glAttachShader(shadername, vsname);
    glAttachShader(shadername, fsname);
    glLinkProgram(shadername);

    return shadername;
}

bool Renderer::initialize(GLchar* windowname, const unsigned int width, const unsigned int height, const unsigned int bitdepth)
{
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
        return false;

    if(!SDL_SetVideoMode(width, height, bitdepth, SDL_OPENGL))
        return false;

    if(!initializeOpenGL(width, height))
        return false;

    SDL_WM_SetCaption(windowname, NULL);

    return true;
}

void Renderer::render(Simulation* simulation)
{
    glClear(GL_COLOR_BUFFER_BIT);
    simulation->render();
    SDL_GL_SwapBuffers();
}

bool Renderer::initializeOpenGL(const unsigned int width, const unsigned int height)
{
    GLenum glewError = glewInit();
    if( glewError != GLEW_OK )
    {
        cout << "Error initializing GLEW: " << glewGetErrorString(glewError) << endl;
        return false;
    }

    if( !GLEW_VERSION_2_1 )
    {
        cout << "OpenGL 2.1 not supported" << endl;
        return false;
    }

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width, height, 0.0, 1.0, -1.0 );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor( 0.f, 0.f, 0.f, 1.f );

    GLenum error = glGetError();
    if( error != GL_NO_ERROR )
    {
        cout << "Error initializing OpenGL" << gluErrorString(error) << endl;
        return false;
    }

    return true;
}

bool Renderer::handleEvents()
{
    SDL_Event event;
    bool run = true;

    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT: 
                run = false;
                break;
            default:    
                break;
        }
    }
    return run;
}