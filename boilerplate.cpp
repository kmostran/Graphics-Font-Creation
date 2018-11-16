// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================


//Kyle Ostrander
//10128524
//CPSC 453 FALL 2016
//T01 - Lee Ringham

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>
//========= include math and vector
#include <vector>
#include <math.h>

// specify that we want the OpenGL core profile before including GLFW headers
// specify that we want the OpenGL core profile before including GLFW headers
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

using namespace std;
using std::vector;

// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

//========= add func prototype
void createQuadratic(vector<float>& vertices,vector<float>& colours);
void createCubic(vector<float>& vertices,vector<float>& colours);
//Counters for the levels of each scene
int spiralLevel;
int squareLevel;
int fractalLevel;
int fractalNumber;

// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

struct MyShader
{
        // OpenGL names for vertex and fragment shaders, shader program
        GLuint  vertex;
        GLuint  fragment;
        GLuint  program;

        // initialize shader and program names to zero (OpenGL reserved value)
        MyShader() : vertex(0), fragment(0), program(0)
        {}
};

// load, compile, and link shaders, returning true if successful
bool InitializeShaders(MyShader *shader)
{
        // load shader source from files
        string vertexSource = LoadSource("vertex.glsl");
        string fragmentSource = LoadSource("fragment.glsl");
        if (vertexSource.empty() || fragmentSource.empty()) return false;

        // compile shader source into shader objects
        shader->vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
        shader->fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

        // link shader program
        shader->program = LinkProgram(shader->vertex, shader->fragment);

        // check for OpenGL errors and return false if error occurred
        return !CheckGLErrors();
}

// deallocate shader-related objects
void DestroyShaders(MyShader *shader)
{
        // unbind any shader programs and destroy shader objects
        glUseProgram(0);
        glDeleteProgram(shader->program);
        glDeleteShader(shader->vertex);
        glDeleteShader(shader->fragment);
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

struct MyGeometry
{
        // OpenGL names for array buffer objects, vertex array object
        GLuint  vertexBuffer;
        GLuint  colourBuffer;
        GLuint  vertexArray;
        GLsizei elementCount;

        // initialize object names to zero (OpenGL reserved value)
        MyGeometry() : vertexBuffer(0), colourBuffer(0), vertexArray(0), elementCount(0)
        {}
};

//globals
MyGeometry geometry;
GLuint drawingMode;
//GLunit renderMode;

// create buffers and fill with geometry data, returning true if successful
//========= Add parameters
bool InitializeGeometry(MyGeometry *geometry, const vector<float>& vertices, const vector<float>& colours)
{
        //========= update element count
        geometry->elementCount = vertices.size() / 2;

        // these vertex attribute indices correspond to those specified for the
        // input variables in the vertex shader
        const GLuint VERTEX_INDEX = 0;
        const GLuint COLOUR_INDEX = 1;

        // create an array buffer object for storing our vertices
        glGenBuffers(1, &geometry->vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
        //========= Update buffer data vertices
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

        // create another one for storing our colours
        glGenBuffers(1, &geometry->colourBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
        //========= Update budder data colours
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*colours.size(), &colours[0], GL_STATIC_DRAW);

        // create a vertex array object encapsulating all our vertex attributes
        glGenVertexArrays(1, &geometry->vertexArray);
        glBindVertexArray(geometry->vertexArray);

        // associate the position array with the vertex array object
        glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
        glVertexAttribPointer(VERTEX_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(VERTEX_INDEX);

        // assocaite the colour array with the vertex array object
        glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
        glVertexAttribPointer(COLOUR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(COLOUR_INDEX);

        // unbind our buffers, resetting to default state
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // check for OpenGL errors and return false if error occurred
        return !CheckGLErrors();
}

// deallocate geometry-related objects
void DestroyGeometry(MyGeometry *geometry)
{
        // unbind and destroy our vertex array object and associated buffers
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &geometry->vertexArray);
        glDeleteBuffers(1, &geometry->vertexBuffer);
        glDeleteBuffers(1, &geometry->colourBuffer);
}

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer
//========= Render with given renderMode
void RenderScene(MyGeometry *geometry, MyShader *shader, GLuint renderMode = GL_TRIANGLES)
{
        // clear screen to a dark grey colour
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // bind our shader program and the vertex array object containing our
        // scene geometry, then tell OpenGL to draw our geometry
        glUseProgram(shader->program);
        glBindVertexArray(geometry->vertexArray);
        glDrawArrays(renderMode, 0, geometry->elementCount);

        // reset state to default (no shader or geometry bound)
        glBindVertexArray(0);
        glUseProgram(0);

        // check for an report any OpenGL errors
        CheckGLErrors();
}

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
        cout << "GLFW ERROR " << error << ":" << endl;
        cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
        vector<float> vertices;
        vector<float> colours;        

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GL_TRUE);

        //When 1 is pressed draw the quadra
        else if(key == GLFW_KEY_1 && action == GLFW_PRESS)
        {
            cout << "Drawing Bezier Curves" << endl;
            createQuadratic(vertices, colours);
            drawingMode = GL_LINE_STRIP;
            InitializeGeometry(&geometry, vertices, colours);
        }
}

// ==========================================================================
// PROGRAM ENTRY POINT

//Main method displays intro explaining program. Sets up window/shader etc. and starts the wait for keyboard input
int main(int argc, char *argv[])
{
        spiralLevel = 0;
        squareLevel = 0;
        fractalLevel = 0;

        //Display opening meessage explaining how the program works and how to navigate between scenes
        cout << "Welcome" << endl;
        // initialize the GLFW windowing system
        if (!glfwInit()) {
                cout << "ERROR: GLFW failed to initilize, TERMINATING" << endl;
                return -1;
        }
        glfwSetErrorCallback(ErrorCallback);

        // attempt to create a window with an OpenGL 4.1 core profile context
        GLFWwindow *window = 0;
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(512, 512, "CPSC 453 OpenGL Boilerplate", 0, 0);
        if (!window) {
                cout << "Program failed to create GLFW window, TERMINATING" << endl;
                glfwTerminate();
                return -1;
        }

        // set keyboard callback function and make our context current (active)
        glfwSetKeyCallback(window, KeyCallback);
        glfwMakeContextCurrent(window);

        // query and print out information about our OpenGL environment
        QueryGLVersion();

        // call function to load and compile shader programs
        MyShader shader;
        if (!InitializeShaders(&shader)) {
                cout << "Program could not initialize shaders, TERMINATING" << endl;
                return -1;
        }

        vector<float> vertices;
        vector<float> colours;
        if (!InitializeGeometry(&geometry, vertices, colours))
               cout << "Program failed to intialize geometry!" << endl;

        // run an event-triggered main loop
        while (!glfwWindowShouldClose(window))
        {
                // call function to draw our scene
                //========= Add drawing mode
                RenderScene(&geometry, &shader, drawingMode);

                // scene is rendered to the back buffer, so swap to front for display
                glfwSwapBuffers(window);

                // sleep until next event before drawing again
                glfwWaitEvents();
        }

        // clean up allocated resources before exit
        DestroyGeometry(&geometry);
        DestroyShaders(&shader);
        glfwDestroyWindow(window);
        glfwTerminate();

        cout << "Goodbye!" << endl;
        return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
        // query opengl version and renderer information
        string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
        string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
        string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

        cout << "OpenGL [ " << version << " ] "
                << "with GLSL [ " << glslver << " ] "
                << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
        bool error = false;
        for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
        {
                cout << "OpenGL ERROR:  ";
                switch (flag) {
                case GL_INVALID_ENUM:
                        cout << "GL_INVALID_ENUM" << endl; break;
                case GL_INVALID_VALUE:
                        cout << "GL_INVALID_VALUE" << endl; break;
                case GL_INVALID_OPERATION:
                        cout << "GL_INVALID_OPERATION" << endl; break;
                case GL_INVALID_FRAMEBUFFER_OPERATION:
                        cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
                case GL_OUT_OF_MEMORY:
                        cout << "GL_OUT_OF_MEMORY" << endl; break;
                default:
                        cout << "[unknown error code]" << endl;
                }
                error = true;
        }
        return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
        string source;

        ifstream input(filename.c_str());
        if (input) {
                copy(istreambuf_iterator<char>(input),
                        istreambuf_iterator<char>(),
                        back_inserter(source));
                input.close();
        }
        else {
                cout << "ERROR: Could not load shader source from file "
                        << filename << endl;
        }

        return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
        // allocate shader object name
        GLuint shaderObject = glCreateShader(shaderType);

        // try compiling the source as a shader of the given type
        const GLchar *source_ptr = source.c_str();
        glShaderSource(shaderObject, 1, &source_ptr, 0);
        glCompileShader(shaderObject);

        // retrieve compile status
        GLint status;
        glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
        {
                GLint length;
                glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
                string info(length, ' ');
                glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
                cout << "ERROR compiling shader:" << endl << endl;
                cout << source << endl;
                cout << info << endl;
        }

        return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
        // allocate program object name
        GLuint programObject = glCreateProgram();

        // attach provided shader objects to this program
        if (vertexShader)   glAttachShader(programObject, vertexShader);
        if (fragmentShader) glAttachShader(programObject, fragmentShader);

        // try linking the program with given attachments
        glLinkProgram(programObject);

        // retrieve link status
        GLint status;
        glGetProgramiv(programObject, GL_LINK_STATUS, &status);
        if (status == GL_FALSE)
        {
                GLint length;
                glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
                string info(length, ' ');
                glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
                cout << "ERROR linking shader program:" << endl;
                cout << info << endl;
        }

        return programObject;
}

//Draws the spiral
//Simply modified the code from the circle we drew in lab
void createQuadratic(vector<float>& vertices,vector<float>& colours)
{
        float red = 1.f;
        float green = 0.f;
        float blue = 0.f;
        float u = 0;
            while (u <= 1.00001)
            {
                    vertices.push_back((float)((1-u)*(1-u)*(1/3)+2*u*(1-u)*(2/3)+u*u*0));
                    vertices.push_back((float)((1-u)*(1-u)*(1/3)+2*u*(1-u)*(-1/3)+u*u*(-1/3)));
                    colours.push_back(red);
                    colours.push_back(green);
                    colours.push_back(blue);
                    red -= 1.f/160.f;
                    green += 1.4f/320.f;
                    blue += .2f/60.f;
                    u = u + 0.00625;
            }
}

void createCubic(vector<float>& vertices,vector<float>& colours)
{
        float red = 1.f;
        float green = 0.f;
        float blue = 0.f;
        float u = 0;
        float rad;
            while (u <= 1.00001)
            {
                    rad = u*4*M_PI;
                    vertices.push_back((float)(u*cos(rad)));
                    vertices.push_back(-(float)(u*sin(rad)));

                    colours.push_back(red);
                    colours.push_back(green);
                    colours.push_back(blue);
                    red -= 1.f/160.f;
                    green += 1.4f/320.f;
                    blue += .2f/60.f;
                    u = u + 0.00625;
            }
}

