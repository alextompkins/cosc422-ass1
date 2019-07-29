//  ========================================================================
//  COSC422: Advanced Computer Graphics;  University of Canterbury.
//
//  FILE NAME: Bezier.cpp
//  AUTHOR: Alex Tompkins
//  Part I of Assignment #1
//
//	The program loads the mesh data for a model and displays a tesselated version of it.
//  Requires shader files Bezier.vert, Bezier.frag, Bezier.tesc, Bezier.tese
//  ========================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "CubePatches.h"

using namespace std;

GLuint vaoID;
GLuint matrixLoc;
float angle = 0.0;
glm::mat4 projView;
float CDR = 3.14159265 / 180.0;   // Conversion from degrees to radians (required in GLM 0.9.6 and later versions)

GLuint loadShader(GLenum shaderType, const string& filename) {
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


void initialise() {
	glm::mat4 proj, view;
	GLuint shaderVert = loadShader(GL_VERTEX_SHADER, "shaders/Bezier.vert");
	GLuint shaderFrag = loadShader(GL_FRAGMENT_SHADER, "shaders/Bezier.frag");
	GLuint shaderTessCont = loadShader(GL_TESS_CONTROL_SHADER, "shaders/Bezier.tesc");
	GLuint shaderTessEval = loadShader(GL_TESS_EVALUATION_SHADER, "shaders/Bezier.tese");

	GLuint program = glCreateProgram();
	glAttachShader(program, shaderVert);
	glAttachShader(program, shaderFrag);
	glAttachShader(program, shaderTessCont);
	glAttachShader(program, shaderTessEval);
	glLinkProgram(program);

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
	glUseProgram(program);


	proj = glm::perspective(20.0f * CDR, 1.0f, 10.0f, 1000.0f);  //perspective projection matrix
	view = glm::lookAt(glm::vec3(0.0, 5.0, 12.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)); //view matrix
	projView = proj * view;  //Product matrix

	GLuint vboID[4];

	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	glGenVertexArrays(2, vboID);

	glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);  // Vertex position

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elems), elems, GL_STATIC_DRAW);

	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void update(int value) {
	angle++;
	glutTimerFunc(50, update, 0);
	glutPostRedisplay();
}

void display() {
	glm::mat4 matrix = glm::mat4(1.0);
	matrix = glm::rotate(matrix, angle * CDR, glm::vec3(0.0, 1.0, 0.0));  //rotation matrix
	glm::mat4 prodMatrix = projView * matrix;        //Model-view-proj matrix

	glUniformMatrix4fv(matrixLoc, 1, GL_FALSE, &prodMatrix[0][0]);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(vaoID);
	glDrawElements(GL_PATCHES, 24, GL_UNSIGNED_SHORT, NULL);

	glPatchParameteri(GL_PATCH_VERTICES, 4);

	glFlush();
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Cube with Bezier patches");
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
	glutTimerFunc(50, update, 0);
	glutMainLoop();
}
