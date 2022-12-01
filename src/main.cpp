#include <stdlib.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
// Use of degrees is deprecated. Use radians for GLM functions
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Screenshot.h"
#include "Shader.h"
#include "Cube.h"
#include "Obj.h"
#include "Camera.h"
#include "HalfEdge.h"

static const int width = 800;
static const int height = 600;
static const char* title = "Model viewer";
static const glm::vec4 background(0.1f, 0.2f, 0.3f, 1.0f);

std::vector<HalfEdgeMesh> surfaces;
std::vector<HalfEdgeQuadMesh> quadSurfaces;

static HalfEdgeMesh icosohedron;
static HalfEdgeMesh octohedron;
static HalfEdgeMesh sphere;
static HalfEdgeMesh quad;

static HalfEdgeQuadMesh quadCube;
static HalfEdgeQuadMesh quadQuad;

static Camera camera;
struct NormalShader : Shader {
    
    // modelview and projection
    glm::mat4 modelview = glm::mat4(1.0f); GLuint modelview_loc;
    glm::mat4 projection = glm::mat4(1.0f); GLuint projection_loc;
    
    void initUniforms(){
        modelview_loc = glGetUniformLocation( program, "modelview" );
        projection_loc = glGetUniformLocation( program, "projection" );
    }
    void setUniforms(){
        glUniformMatrix4fv(modelview_loc, 1, GL_FALSE, &modelview[0][0]);
        glUniformMatrix4fv(projection_loc, 1, GL_FALSE, &projection[0][0]);
    }
};
static NormalShader shader;
static bool outline = false;
static bool use_face_norm = true;
static bool enable_perspective = true;
// A simple projection matrix for orthographic view.
static glm::mat4 proj_default = glm::mat4(0.75f,0.0f,0.0f,0.0f,
                                          0.0f,1.0f,0.0f,0.0f,
                                          0.0f,0.0f,-0.1f,0.0f,
                                          0.0f,0.0f,0.0f,1.0f);
static int model_selection = 0;
static bool use_catMull = false;

#include "hw2AutoScreenshots.h"

void printHelp(){
    std::cout << R"(
    Available commands:
      press 'h' to print this message again.
      press Esc to quit.
      press 'o' to save a screenshot.
      press the arrow keys to rotate camera.
      press 'r' to reset camera.
      press 'p' to toggle orthographic/perspective.
      press '1','2','3' to select cube, teapot, bunny.
      press '/' to subdivide the current model
      press '.' to toggle face normals and vertext normals
      press ',' to toggle outlines

      press Spacebar to generate images for hw2 submission.
    
)";
    std::cout<< "perspective: " << (enable_perspective?"on":"off") << std::endl;
}

void initialize(void){
    printHelp();
    glClearColor(background[0], background[1], background[2], background[3]); // background color
    glViewport(0,0,width,height);
    
    // Initialize geometries
    icosohedron.init("models/20_icosahedron.obj");
    icosohedron.buildVAO(use_face_norm, outline);

    octohedron.init("models/8_octahedron.obj");
    octohedron.buildVAO(use_face_norm, outline);

    sphere.init("models/sphere.obj");
    sphere.buildVAO(use_face_norm, outline);

    std::vector<glm::vec3> vertices;
    std::vector<GLuint> indices = {
        1, 2, 4,
        4, 2, 3
    };
    vertices.push_back(glm::vec3(-1.0f, -1.0f, 0.0f));
    vertices.push_back(glm::vec3(-1.0f, 1.0f, 0.0f));
    vertices.push_back(glm::vec3(1.0f, 1.0f, 0.0f));
    vertices.push_back(glm::vec3(1.0f, -1.0f, 0.0f));
    quad.init(vertices, indices);
    quad.buildVAO(use_face_norm, outline);

    surfaces.push_back(icosohedron);
    surfaces.push_back(octohedron);
    surfaces.push_back(sphere);
    surfaces.push_back(quad);

    quadCube.init("models/cube_quads.obj");
    quadCube.buildVAO(use_face_norm, outline);

    std::vector<GLuint> quadInds = {
        1,2,3,4
    };
    quadQuad.init(vertices, quadInds);

    quadSurfaces.push_back(quadCube);
    quadSurfaces.push_back(quadQuad);
    quadSurfaces.push_back(quadCube);
    quadSurfaces.push_back(quadCube);

    // Initialize camera (set default values)
    camera.eye_default = glm::vec3(0.0f, 0.2f, 2.5f);
    camera.target_default = glm::vec3(0.0f, 0.2f, 0.0f);
    camera.up_default = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.fovy_default = 90.0f;
    camera.aspect_default = float(width) / float(height);
    camera.near_default = 0.01f;
    camera.far_default = 100.0f;
    camera.reset();

    // Initialize shader
    shader.read_source( "shaders/projective.vert", "shaders/normal.frag");
    shader.compile();
    glUseProgram(shader.program);
    shader.initUniforms();
    shader.setUniforms();

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
}

void display(void){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // Pre-draw sequence
    camera.computeMatrices();
    shader.modelview = camera.view;
    shader.projection = enable_perspective? camera.proj : proj_default;
    shader.setUniforms();
    // BEGIN draw
    if (use_catMull)
        quadSurfaces[model_selection].draw();
    else
        surfaces[model_selection].draw();


    // END draw
    
    glutSwapBuffers();
    glFlush();
    
}

void saveScreenShot(const char* filename = "test.png"){
    int currentwidth = glutGet(GLUT_WINDOW_WIDTH);
    int currentheight = glutGet(GLUT_WINDOW_HEIGHT);
    Screenshot imag = Screenshot(currentwidth,currentheight);
    imag.save(filename);
}



void keyboard(unsigned char key, int x, int y){
    switch(key){
        case 27: // Escape to quit
            exit(0);
            break;
        case 'h': // print help
            printHelp();
            break;
        case 'o': // save screenshot
            saveScreenShot();
            break;
        case 'r':
            camera.reset();
            glutPostRedisplay();
            break;
        case 'p':
            enable_perspective = !enable_perspective; 
            std::cout<< "perspective: " << (enable_perspective?"on":"off") << std::endl;
            glutPostRedisplay();
            break;
        case '1':
            model_selection = (model_selection + 1)%surfaces.size();
            glutPostRedisplay();
            break;
        case '/':
            if (use_catMull) {
                quadSurfaces[model_selection].subdivide();
                quadSurfaces[model_selection].buildVAO(use_face_norm, outline);
            }
            else {
                surfaces[model_selection].subdivide();
                surfaces[model_selection].buildVAO(use_face_norm, outline);
            }
            glutPostRedisplay();
            break;
        case '.':
            use_face_norm = !use_face_norm;
            for (int i = 0; i < surfaces.size(); i++) {
                surfaces[i].buildVAO(use_face_norm, outline);
                quadSurfaces[i].buildVAO(use_face_norm, outline);
            }
            glutPostRedisplay();
            break;
        case ',': 
            outline = !outline;
            if (outline) {
                glLineWidth(10);
                glEnable(GL_CULL_FACE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else {
                glDisable(GL_CULL_FACE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            for (int i = 0; i < surfaces.size(); i++) {
                surfaces[i].buildVAO(use_face_norm, outline);
                quadSurfaces[i].buildVAO(use_face_norm, outline);
            }
            glutPostRedisplay();
            break;
        case 'c':
            use_catMull = !use_catMull;
            glutPostRedisplay();
            break;
        default:
            glutPostRedisplay();
            break;
    }
}
void specialKey(int key, int x, int y){
    switch (key) {
        case GLUT_KEY_UP: // up
            camera.rotateUp(-10.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_DOWN: // down
            camera.rotateUp(10.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_RIGHT: // right
            camera.rotateRight(-10.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_LEFT: // left
            camera.rotateRight(10.0f);
            glutPostRedisplay();
            break;
    }
}




int main(int argc, char** argv)
{
    // BEGIN CREATE WINDOW
    glutInit(&argc, argv);
    glutInitContextVersion(3,1);
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
    glutInitWindowSize(width, height);
    glutCreateWindow(title);
    glewExperimental = GL_TRUE;
    GLenum err = glewInit() ;
    if (GLEW_OK != err) {
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    // END CREATE WINDOW
    
    initialize();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKey);
    
    glutMainLoop();
	return 0;   /* ANSI C requires main to return int. */
}
