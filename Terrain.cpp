//  ========================================================================
//  COSC422: Advanced Computer Graphics;  University of Canterbury.
//
//  FILE NAME: Terrain.cpp
//  AUTHOR: Alex Tompkins
//  Part II of Assignment #1
//
//	The program generates and loads the mesh data for a terrain floor (100 verts, 81 elems).
//  Required files:  Terrain.vert, Terrain.frag, HeightMap1.tga
//  ========================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "loadTGA.h"
using namespace std;

// CONSTANTS
#define TO_RAD (3.14159265f/180.0f)

GLuint terrainProgram;
GLuint terrainVao, terrainVertsVbo, terrainElemsVbo;
GLuint mvMatrixLoc, norMatrixLoc, lightPosLoc, mvpMatrixLoc, wireframeFlagLoc, eyePosLoc,
    heightMapperLoc, waterTexturerLoc, grassTexturerLoc, rockTexturerLoc, snowTexturerLoc,
    waterLevelLoc, snowLevelLoc;

float verts[100 * 3];       //10x10 grid (100 vertices)
GLushort elems[81 * 4];       //Element array for 81 quad patches

struct EyePos {
    float x;
    float y;
    float z;
    float horAngle;
    float vertAngle;
} eyePos, lightPos;
float lookAtHeight;
float waterLevel = 2.0;
float snowLevel = 5.0;

bool wireframeMode = false;

//Generate vertex and element data for the terrain floor
void generateData() {
    int indx, start;
    //verts array
    for (int i = 0; i < 10; i++) {  //100 vertices on a 10x10 grid
        for (int j = 0; j < 10; j++) {
            indx = 10 * i + j;
            verts[3 * indx] = 10 * i - 45;        //x  varies from -45 to +45
            verts[3 * indx + 1] = 0;            //y  is set to 0 (ground plane)
            verts[3 * indx + 2] = -10 * j;        //z  varies from 0 to -100
        }
    }

    //elems array
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            indx = 9 * i + j;
            start = 10 * i + j;
            elems[4 * indx] = start;
            elems[4 * indx + 1] = start + 10;
            elems[4 * indx + 2] = start + 11;
            elems[4 * indx + 3] = start + 1;
        }
    }
}

// Loads terrain texture
void loadTextures() {
    GLuint texID[5];
    glGenTextures(5, texID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID[0]);
    loadTGA("textures/HeightMap1.tga");
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texID[1]);
    loadTGA("textures/water.tga");
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texID[2]);
    loadTGA("textures/grass.tga");
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texID[3]);
    loadTGA("textures/arid_rock.tga");
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, texID[4]);
    loadTGA("textures/snow.tga");
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glUniform1i(heightMapperLoc, 0);
    glUniform1i(waterTexturerLoc, 1);
    glUniform1i(grassTexturerLoc, 2);
    glUniform1i(rockTexturerLoc, 3);
    glUniform1i(snowTexturerLoc, 4);
}

//Loads a shader file and returns the reference to a shader object
GLuint loadShader(GLenum shaderType, string filename) {
    ifstream shaderFile(filename.c_str());
    if (!shaderFile.good()) cout << "Error opening shader file." << endl;
    stringstream shaderData;
    shaderData << shaderFile.rdbuf();
    shaderFile.close();
    string shaderStr = shaderData.str();
    const char *shaderTxt = shaderStr.c_str();

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderTxt, NULL);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
        const char *strShaderType = NULL;
        cerr << "Compile failure in shader: " << strInfoLog << endl;
        delete[] strInfoLog;
    }
    return shader;
}

void printLogs(GLuint program) {
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
        fprintf(stderr, "Linker failure: %s\n", strInfoLog);
        delete[] strInfoLog;
    }
}

void setupTerrainProgram() {
    // Create shaders
    GLuint shaderVert = loadShader(GL_VERTEX_SHADER, "shaders/Terrain.vert");
    GLuint shaderTessCont = loadShader(GL_TESS_CONTROL_SHADER, "shaders/Terrain.tesc");
    GLuint shaderTessEval = loadShader(GL_TESS_EVALUATION_SHADER, "shaders/Terrain.tese");
    GLuint shaderGeom = loadShader(GL_GEOMETRY_SHADER, "shaders/Terrain.geom");
    GLuint shaderFrag = loadShader(GL_FRAGMENT_SHADER, "shaders/Terrain.frag");

    // Attach shaders and link
    terrainProgram = glCreateProgram();
    glAttachShader(terrainProgram, shaderVert);
    glAttachShader(terrainProgram, shaderTessCont);
    glAttachShader(terrainProgram, shaderTessEval);
    glAttachShader(terrainProgram, shaderGeom);
    glAttachShader(terrainProgram, shaderFrag);
    glLinkProgram(terrainProgram);
    printLogs(terrainProgram);

    // Get graphics memory locations for uniform variables
    mvMatrixLoc = glGetUniformLocation(terrainProgram, "mvMatrix");
    norMatrixLoc = glGetUniformLocation(terrainProgram, "norMatrix");
    lightPosLoc = glGetUniformLocation(terrainProgram, "lightPos");
    mvpMatrixLoc = glGetUniformLocation(terrainProgram, "mvpMatrix");
    wireframeFlagLoc = glGetUniformLocation(terrainProgram, "wireframeFlag");
    eyePosLoc = glGetUniformLocation(terrainProgram, "eyePos");
    heightMapperLoc = glGetUniformLocation(terrainProgram, "heightMapper");
    waterTexturerLoc = glGetUniformLocation(terrainProgram, "waterTexturer");
    grassTexturerLoc = glGetUniformLocation(terrainProgram, "grassTexturer");
    rockTexturerLoc = glGetUniformLocation(terrainProgram, "rockTexturer");
    snowTexturerLoc = glGetUniformLocation(terrainProgram, "snowTexturer");
    waterLevelLoc = glGetUniformLocation(terrainProgram, "waterLevel");
    snowLevelLoc = glGetUniformLocation(terrainProgram, "snowLevel");

    // Generate VAO and VBOs
    glGenVertexArrays(1, &terrainVao);
    glGenBuffers(1, &terrainVertsVbo);
    glGenBuffers(1, &terrainElemsVbo);

    // Generate vertex & element data (quads)
    generateData();

    // Generate VAO and VBOs
    glUseProgram(terrainProgram);
    glBindVertexArray(terrainVao);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVertsVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainElemsVbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elems), elems, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
}

void initCamera() {
    eyePos = {
            0.0,
            15.0,
            30.0,
            270,
            -10
    };
    lookAtHeight = 0.0;
    lightPos = {0.0, 500.0, -500.0, 1.0};
}

void initialise() {
    initCamera();

    // Create programs
    setupTerrainProgram();

    // Load textures
    loadTextures();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
}

void calcUniforms() {
    glm::mat4 proj = glm::perspective(30.0f * TO_RAD, 1.25f, 0.001f, 10000.0f);  // perspective projection matrix
    glm::mat4 view = glm::lookAt(
            glm::vec3(eyePos.x, eyePos.y, eyePos.z),
            glm::vec3(eyePos.x, eyePos.y, eyePos.z - 5),
            glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 mvMatrix = glm::lookAt(
            glm::vec3(eyePos.x, eyePos.y, eyePos.z), // eye pos
            glm::vec3(eyePos.x + cos(glm::radians(eyePos.horAngle)),
                      eyePos.y + sin(glm::radians(eyePos.vertAngle)),
                      eyePos.z + sin(glm::radians(eyePos.horAngle))), // look at pos
            glm::vec3(0.0, 1.0, 0.0)); // up vector

    glm::vec4 lightVec = glm::vec4(lightPos.x, lightPos.y, lightPos.z, 1.0);
    glm::vec4 lightEye = view * lightVec;

    glm::mat4 norMatrix = glm::inverse(view);
    glm::mat4 mvpMatrix = proj * mvMatrix;  //Product (mvp) matrix

    glUniformMatrix4fv(mvMatrixLoc, 1, GL_FALSE, &mvMatrix[0][0]);
    glUniformMatrix4fv(norMatrixLoc, 1, GL_FALSE, &norMatrix[0][0]);
    glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, &mvpMatrix[0][0]);
    glUniform4fv(lightPosLoc, 1, &lightEye[0]);

    glm::vec4 eyePosVec = glm::vec4(eyePos.x, eyePos.y, eyePos.z, 1);
    glUniform4fv(eyePosLoc, 1, &eyePosVec[0]);

    glUniform1f(waterLevelLoc, waterLevel);
    glUniform1f(snowLevelLoc, snowLevel);
}

void setPolygonMode(bool isWireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, isWireframe ? GL_LINE : GL_FILL);
    glUniform1i(wireframeFlagLoc, isWireframe);
    if (isWireframe) {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(terrainProgram);
    calcUniforms();
    setPolygonMode(wireframeMode);
    glBindVertexArray(terrainVao);
    glDrawElements(GL_PATCHES, 81 * 4, GL_UNSIGNED_SHORT, NULL);

    glBindVertexArray(0);
    glFlush();
}

void special(int key, int x, int y) {
    const float MOVE_DISTANCE = 1.0;

    switch (key) {
        case GLUT_KEY_LEFT:
            eyePos.x -= MOVE_DISTANCE;
            break;
        case GLUT_KEY_RIGHT:
            eyePos.x += MOVE_DISTANCE;
            break;
        case GLUT_KEY_UP:
            eyePos.z -= MOVE_DISTANCE;
            break;
        case GLUT_KEY_DOWN:
            eyePos.z += MOVE_DISTANCE;
            break;
    }

    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    const float MOVE_DISTANCE = 1.0;
    const float ANGLE_INCR = 1.0;

    switch (key) {
        case ' ':
            eyePos.y += MOVE_DISTANCE;
            break;
        case 'x':
            eyePos.y -= MOVE_DISTANCE;
            break;
        case 'i':
            eyePos.vertAngle += ANGLE_INCR;
            break;
        case 'k':
            eyePos.vertAngle -= ANGLE_INCR;
            break;
        case 'j':
            eyePos.horAngle -= ANGLE_INCR;
            break;
        case 'l':
            eyePos.horAngle += ANGLE_INCR;
            break;
        case 'w':
            wireframeMode = !wireframeMode;
            break;
        case 'y':
            waterLevel += 0.25;
            cout << "Water Level: " << waterLevel << endl;
            break;
        case 'h':
            waterLevel -= 0.25;
            cout << "Water Level: " << waterLevel << endl;
            break;
        case 't':
            snowLevel += 0.25;
            cout << "Snow Level: " << snowLevel << endl;
            cout << "Rock Level: " << snowLevel - 2.5 << endl;
            break;
        case 'g':
            snowLevel -= 0.25;
            cout << "Snow Level: " << snowLevel << endl;
            cout << "Rock Level: " << snowLevel - 2.0 << endl;
            break;
        case 'e':
            lightPos.z -= MOVE_DISTANCE * 10;
            break;
        case 'd':
            lightPos.z += MOVE_DISTANCE * 10;
            break;
        case 's':
            lightPos.x -= MOVE_DISTANCE * 10;
            break;
        case 'f':
            lightPos.x += MOVE_DISTANCE * 10;
            break;
        case 'q':
            lightPos.y += MOVE_DISTANCE * 10;
            break;
        case 'a':
            lightPos.y -= MOVE_DISTANCE * 10;
            break;
    }
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
    glutInitWindowSize(1000, 800);
    glutCreateWindow("Terrain");
    glutInitContextVersion(4, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);

    if (glewInit() == GLEW_OK) {
        cout << "GLEW initialization successful! " << endl;
        cout << " Using GLEW version " << glewGetString(GLEW_VERSION) << endl;
    } else {
        cerr << "Unable to initialize GLEW  ...exiting." << endl;
        exit(EXIT_FAILURE);
    }

    initialise();
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutMainLoop();
}
