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
GLuint mvpMatrixLoc, textureLoc, wireframeFlagLoc;

float verts[100 * 3];       //10x10 grid (100 vertices)
GLushort elems[81 * 4];       //Element array for 81 quad patches

struct EyePos {
    float x;
    float y;
    float z;
    float angle;
} eyePos;
float lookAtHeight;

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
	GLuint texID;
	glGenTextures(1, &texID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	loadTGA("textures/HeightMap1.tga");

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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
    GLuint shaderv = loadShader(GL_VERTEX_SHADER, "shaders/Terrain.vert");
    GLuint shaderf = loadShader(GL_FRAGMENT_SHADER, "shaders/Terrain.frag");

    // Attach shaders and link
    terrainProgram = glCreateProgram();
    glAttachShader(terrainProgram, shaderv);
    glAttachShader(terrainProgram, shaderf);
    glLinkProgram(terrainProgram);
    printLogs(terrainProgram);

    // Get graphics memory locations for uniform variables
    mvpMatrixLoc = glGetUniformLocation(terrainProgram, "mvpMatrix");
    textureLoc = glGetUniformLocation(terrainProgram, "heightMap");

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
            10.0,
            0.0,
            270
    };
    lookAtHeight = 0.0;
}

void initialise() {
    initCamera();

    // Create programs
    setupTerrainProgram();

    // Load textures
	loadTextures();
	glUniform1i(textureLoc, 0);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void calcUniforms() {
    glm::mat4 proj = glm::perspective(30.0f * TO_RAD, 1.25f, 20.0f, 500.0f);  // perspective projection matrix
    glm::mat4 view = glm::lookAt(
            glm::vec3(eyePos.x, eyePos.y, eyePos.z), // eye pos
            glm::vec3(eyePos.x + cos(glm::radians(eyePos.angle)), eyePos.y, eyePos.z + sin(glm::radians(eyePos.angle))), // look at pos
            glm::vec3(0.0, 1.0, 0.0)); // up vector
    glm::mat4 mvMatrix = view;

    glm::vec4 lightPos = glm::vec4(0.0, 0.0, 500.0, 1.0);
    glm::vec4 lightEye = view * lightPos;

    glm::mat4 norMatrix = glm::inverse(mvMatrix);
    glm::mat4 mvpMatrix = proj * mvMatrix;

//    view = glm::lookAt(glm::vec3(0.0, 20.0, 30.0), glm::vec3(0.0, 0.0, -40.0), glm::vec3(0.0, 1.0, 0.0)); // view matrix
    mvpMatrix = proj * view;  //Product (mvp) matrix

    glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, &mvpMatrix[0][0]);
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
    setPolygonMode(true);
	glBindVertexArray(terrainVao);
	glDrawElements(GL_QUADS, 81 * 4, GL_UNSIGNED_SHORT, NULL);

	glBindVertexArray(0);
	glFlush();
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
	glutDisplayFunc(display);
	glutMainLoop();
}
