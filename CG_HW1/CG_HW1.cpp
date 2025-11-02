// OpenGL and FreeGlut headers.
#include <glew.h>
#include <freeglut.h>

// GLM.
#include <glm.hpp>
#include <gtc/type_ptr.hpp>

// C++ STL headers.
#include <iostream>
#include <algorithm>
#include <vector>
#include <iomanip>

// My headers.
#include "TriangleMesh.h"


// Global variables.
const int screenWidth = 600;
const int screenHeight = 600;
TriangleMesh* mesh = nullptr;
string path;

// Function prototypes.
void SetupRenderState();
void SetupScene(const std::string&);
void ReleaseResources();
void RenderSceneCB();
void ReshapeCB(int, int);
void ProcessSpecialKeysCB(int, int, int);
void ProcessKeysCB(unsigned char, int, int);


// Callback function for glutDisplayFunc.
void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnableVertexAttribArray(0);

    glDrawElements(GL_TRIANGLES, mesh->GetNumIndices(), GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    glutSwapBuffers();
}

// Callback function for glutReshapeFunc.
void ReshapeCB(int w, int h)
{
    // Adjust camera and projection here.
    // Implemented in HW2.
}

// Callback function for glutSpecialFunc.
void ProcessSpecialKeysCB(int key, int x, int y)
{
    // Handle special (functional) keyboard inputs such as F1, spacebar, page up, etc. 
    switch (key) {
    case GLUT_KEY_F1:
        // Render with point mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        break;
    case GLUT_KEY_F2:
        // Render with line mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    case GLUT_KEY_F3:
        // Render with fill mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    default:
        break;
    }
}

// Callback function for glutKeyboardFunc.
void ProcessKeysCB(unsigned char key, int x, int y)
{
    // Handle other keyboard inputs those are not defined as special keys.
    if (key == 27) {
        // Release memory allocation if needed.
        ReleaseResources();
        exit(0);
    }

    if (key == 9) {
        ReleaseResources();
        
        bool loaded = false;
        // Initialization.
        SetupRenderState();
        while (!loaded) {
            string new_path;
            cout << "Plese enter the full path of the file:";
            cin >> new_path;
            for (int i = 0; i < new_path.size(); i++) {
                if (new_path[i] == '/') {
                    new_path.insert(i, "/");
                }
            }

            if (new_path == path) {
                cout << "Already rendered, please enter new one" << endl;
                continue;
            }
            else {
                path = new_path;
            }
            
            

            SetupScene(path);

            if (mesh != nullptr) loaded = true;
        }

    }
}

void ReleaseResources()
{
    if (mesh != nullptr) {
        delete mesh;
        mesh = nullptr;
    }
}

void SetupRenderState()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glm::vec4 clearColor = glm::vec4(0.44f, 0.57f, 0.75f, 1.00f);
    glClearColor(
        (GLclampf)(clearColor.r), 
        (GLclampf)(clearColor.g), 
        (GLclampf)(clearColor.b), 
        (GLclampf)(clearColor.a)
    );
}

// Load a model from obj file and apply transformation.
// You can alter the parameters for dynamically loading a model.
void SetupScene(const std::string& modelPath)
{
    ReleaseResources();

    mesh = new TriangleMesh();
    while (!mesh->LoadFromFile(modelPath)) {
        
        delete mesh;
        mesh = nullptr;
        return;
        
    }
 
    // Please DO NOT TOUCH the following code.
    // ------------------------------------------------------------------------
    // Build transformation matrices.
    // World.
    glm::mat4x4 M(1.0f);
    // Camera.
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.5f, 2.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4x4 V = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    // Projection.
    float fov = 40.0f;
    float aspectRatio = (float)screenWidth / (float)screenHeight;
    float zNear = 0.1f;
    float zFar = 100.0f;
    glm::mat4x4 P = glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar);

    // Apply CPU transformation.
    glm::mat4x4 MVP = P * V * M;
    
    mesh->ApplyTransformCPU(MVP);
    
    // Create and upload vertex/index buffers.
    mesh->CreateBuffers();
}

int main(int argc, char** argv)
{
    // Setting window properties.
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(screenWidth, screenHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("HW1: OBJ Loader");

    // Initialize GLEW.
    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        std::cerr << "GLEW initialization error: " 
                  << glewGetErrorString(res) << std::endl;
        return 1;
    }
    
    while (1) {
        bool loaded = false;
        // Initialization.
        SetupRenderState();
        while (!loaded) {
            cout << "Plese enter the full path of the file:";
            cin >> path;

            for (int i = 0; i < path.size(); i++) {
                if (path[i] == '/') {
                    path.insert(i, "/");
                }
            }

            SetupScene(path);

            if (mesh != nullptr) loaded = true;
        }
        

        // Register callback functions.
        glutDisplayFunc(RenderSceneCB);
        glutIdleFunc(RenderSceneCB);
        glutReshapeFunc(ReshapeCB);
        glutSpecialFunc(ProcessSpecialKeysCB);
        glutKeyboardFunc(ProcessKeysCB);

        // Start rendering loop.
        glutMainLoop();
    }
    return 0;
}

