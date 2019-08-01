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
#include <cmath>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;

// CONSTANTS
#define TO_RAD (3.14159265f/180.0f)

GLuint vaoID;
GLuint mvpMatrixLoc, tessLevelLoc;
glm::mat4 projView;

int numVertices;
float* vertices;

struct EyePos {
    float angle;
    float rad;
    float height;
} eyePos;
float lookAtHeight;

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

int readVertices(const string &filename) {
    ifstream infile;
    infile.open(filename, ios::in);

    infile >> numVertices;

    vertices = new float[numVertices * 3];

    for (int i = 0; i < numVertices; i++) {
        infile >> vertices[i * 3]
               >> vertices[i * 3 + 1]
               >> vertices[i * 3 + 2];
    }

    infile.close();
    return numVertices;
}

void freeVertices() {
    delete[] vertices;
    vertices = NULL;
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

void initCamera(bool bigModel) {
    eyePos = {
            0.0,
            20.0f * (bigModel ? 4 : 1),
            4.0f * (bigModel ? 10 : 1)
    };
    lookAtHeight = 1.0 * (bigModel ? 10 : 1);
}

void initialise() {
    GLuint shaderVert = loadShader(GL_VERTEX_SHADER, "shaders/Bezier.vert");
    GLuint shaderTessCont = loadShader(GL_TESS_CONTROL_SHADER, "shaders/Bezier.tesc");
    GLuint shaderTessEval = loadShader(GL_TESS_EVALUATION_SHADER, "shaders/Bezier.tese");
    GLuint shaderFrag = loadShader(GL_FRAGMENT_SHADER, "shaders/Bezier.frag");

	GLuint program = glCreateProgram();
	glAttachShader(program, shaderVert);
    glAttachShader(program, shaderTessCont);
    glAttachShader(program, shaderTessEval);
	glAttachShader(program, shaderFrag);
	glLinkProgram(program);
	printLogs(program);

	glUseProgram(program);

	mvpMatrixLoc = glGetUniformLocation(program, "mvpMatrix");
	tessLevelLoc = glGetUniformLocation(program, "tessLevel");

	GLuint vboID;

	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	glGenBuffers(1, &vboID);

    // 4x4 bezier patches (16 vertices per patch)
    numVertices = readVertices("geom/PatchVerts_Teapot.txt");
    long sizeOfVertices = sizeof(float) * numVertices * 3;

	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeOfVertices, vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);  // Vertex position

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glPatchParameteri(GL_PATCH_VERTICES, 16);

    initCamera(false);
}

void calcProjView() {
    glm::mat4 proj = glm::perspective(20.0f * TO_RAD, 1.0f, 0.001f, 1000.0f);  //perspective projection matrix
    glm::mat4 view = glm::lookAt(
            glm::vec3(eyePos.rad * sin(eyePos.angle * TO_RAD), eyePos.height, eyePos.rad * cos(eyePos.angle * TO_RAD)), // eye pos
            glm::vec3(0.0, lookAtHeight, 0.0), // look at pos
            glm::vec3(0.0, 1.0, 0.0)); // up vector
    projView = proj * view;

    glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, &projView[0][0]);
}

void setTessLevel() {
    int tessLevel;
    if (eyePos.rad > 25) {
        tessLevel = 2;
    } else if (eyePos.rad > 10) {
        tessLevel = 4;
    } else if (eyePos.rad > 5) {
        tessLevel = 8;
    } else {
        tessLevel = 16;
    }
    glUniform1i(tessLevelLoc, tessLevel);
}

void display() {
    cout << "eyePos: " << eyePos.angle << " " << eyePos.height << " " <<  eyePos.rad << endl;
    cout << "lookAtHeight: " << lookAtHeight << endl;
    calcProjView();
    setTessLevel();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_PATCHES, 0, numVertices);

	glFlush();
}

void special(int key, int x, int y) {
    const float CHANGE_VIEW_ANGLE = 2.0;
    const float RAD_INCR = 0.5;

    switch (key) {
        case GLUT_KEY_LEFT:
            eyePos.angle -= CHANGE_VIEW_ANGLE;
            break;
        case GLUT_KEY_RIGHT:
            eyePos.angle += CHANGE_VIEW_ANGLE;
            break;
        case GLUT_KEY_UP:
            eyePos.rad -= RAD_INCR;
            break;
        case GLUT_KEY_DOWN:
            eyePos.rad += RAD_INCR;
            break;
    }

    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    const float VERT_INCR = 1.0;

    switch (key) {
        case ' ':
            eyePos.height += VERT_INCR;
            break;
        case 'x':
            eyePos.height -= VERT_INCR;
            break;
        case 'i':
            lookAtHeight += VERT_INCR;
            break;
        case 'k':
            lookAtHeight -= VERT_INCR;
            break;
    }
    glutPostRedisplay();
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
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);
	glutMainLoop();
}
