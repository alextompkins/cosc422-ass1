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
#define MODEL_FILENAME "geom/PatchVerts_Gumbo.txt"
#define TIMER_INTERVAL 20
#define FLOOR_WIDTH 10
#define FLOOR_LENGTH 10

GLuint bezierProgram, floorProgram;
GLuint bezierVao, floorVao, bezierVbo, floorVertsVbo, floorElemsVbo;
GLuint mvpMatrixLoc, tessLevelLoc, mvMatrixLoc, norMatrixLoc, lightPosLoc, wireframeFlagLoc, timeSinceExplosionLoc, floorMvpMatrixLoc;

int numVertices;
float* vertices;
float floorVerts[FLOOR_WIDTH * FLOOR_LENGTH * 3];      // 10x10 grid (100 vertices)
GLushort floorElems[(FLOOR_WIDTH - 1) * (FLOOR_LENGTH - 1) * 4];    // element array for 81 quad patches

struct EyePos {
    float angle;
    float rad;
    float height;
} eyePos;
float lookAtHeight;

bool wireframeMode = false;
bool isBigModel = false;
bool hasExploded = false;
int timeSinceExplosion = 0;

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

void generateFloor() {
    int indx, start;
    // vertices
    for (int i = 0; i < FLOOR_LENGTH; i++) { // 100 vertices on a 10x10 grid
        for (int j = 0; j < FLOOR_WIDTH; j++) {
            indx = FLOOR_LENGTH * i + j;
            floorVerts[3 * indx] = 10 * i - 45;         // x varies from -45 to +45
            floorVerts[3 * indx + 1] = 0;               // y is set to 0 (ground plane)
            floorVerts[3 * indx + 2] = 10 * j - 50;         // z varies from -50 to 50
        }
    }

    // elements
    for (int i = 0; i < FLOOR_LENGTH - 1; i++) {
        for (int j = 0; j < FLOOR_WIDTH - 1; j++) {
            indx = (FLOOR_LENGTH - 1) * i + j;
            start = FLOOR_LENGTH * i + j;
            floorElems[4 * indx] = start;
            floorElems[4 * indx + 1] = start + FLOOR_LENGTH;
            floorElems[4 * indx + 2] = start + FLOOR_LENGTH + 1;
            floorElems[4 * indx + 3] = start + 1;
        }
    }
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
            4.0f * (bigModel ? 6 : 1)
    };
    lookAtHeight = 1.0 * (bigModel ? 10 : 1);
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

void setupBezierProgram() {
    // Create shaders
    GLuint shaderVert = loadShader(GL_VERTEX_SHADER, "shaders/Bezier.vert");
    GLuint shaderTessCont = loadShader(GL_TESS_CONTROL_SHADER, "shaders/Bezier.tesc");
    GLuint shaderTessEval = loadShader(GL_TESS_EVALUATION_SHADER, "shaders/Bezier.tese");
    GLuint shaderGeom = loadShader(GL_GEOMETRY_SHADER, "shaders/Bezier.geom");
    GLuint shaderFrag = loadShader(GL_FRAGMENT_SHADER, "shaders/Bezier.frag");

    // Attach shaders and link
    bezierProgram = glCreateProgram();
    glAttachShader(bezierProgram, shaderVert);
    glAttachShader(bezierProgram, shaderTessCont);
    glAttachShader(bezierProgram, shaderTessEval);
    glAttachShader(bezierProgram, shaderGeom);
    glAttachShader(bezierProgram, shaderFrag);
    glLinkProgram(bezierProgram);
    printLogs(bezierProgram);

    // Get graphics memory locations for uniform variables
    mvpMatrixLoc = glGetUniformLocation(bezierProgram, "mvpMatrix");
    tessLevelLoc = glGetUniformLocation(bezierProgram, "tessLevel");
    mvMatrixLoc = glGetUniformLocation(bezierProgram, "mvMatrix");
    norMatrixLoc = glGetUniformLocation(bezierProgram, "norMatrix");
    lightPosLoc = glGetUniformLocation(bezierProgram, "lightPos");
    wireframeFlagLoc = glGetUniformLocation(bezierProgram, "wireframeFlag");
    timeSinceExplosionLoc = glGetUniformLocation(bezierProgram, "timeSinceExplosion");

    // Load vertex data
    // 4x4 bezier patches (16 vertices per patch)
    numVertices = readVertices(MODEL_FILENAME);
    long sizeOfVertices = sizeof(float) * numVertices * 3;

    // Generate VAO and VBO
    glGenVertexArrays(1, &bezierVao);
    glGenBuffers(1, &bezierVbo);

    // Put vertex data in VBO and initialise pointer
    glUseProgram(bezierProgram);
    glBindVertexArray(bezierVao);
    glBindBuffer(GL_ARRAY_BUFFER, bezierVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeOfVertices, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
}

void setupFloorProgram() {
    // Create shaders
    GLuint vertShader = loadShader(GL_VERTEX_SHADER, "shaders/Floor.vert");
    GLuint fragShader = loadShader(GL_FRAGMENT_SHADER, "shaders/Floor.frag");

    // Attach shaders and link
    floorProgram = glCreateProgram();
    glAttachShader(floorProgram, vertShader);
    glAttachShader(floorProgram, fragShader);
    glLinkProgram(floorProgram);
    printLogs(floorProgram);

    // Get graphics memory locations for uniform variables
    floorMvpMatrixLoc = glGetUniformLocation(floorProgram, "mvpMatrix");

    // Generate vertex & element data (quads)
    generateFloor();

    // Generate VAO and VBOs
    glGenVertexArrays(1, &floorVao);
    glGenBuffers(1, &floorVertsVbo);
    glGenBuffers(1, &floorElemsVbo);

    // Put vertex & element data in VBOs and initialise pointer
    glUseProgram(floorProgram);
    glBindVertexArray(floorVao);
    glBindBuffer(GL_ARRAY_BUFFER, floorVertsVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVerts), floorVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorElemsVbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorElems), floorElems, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
}

void initialise() {
    // Custom setup
    isBigModel = string(MODEL_FILENAME).find("Gumbo") != -1;
    initCamera(isBigModel);

    // Create programs
    setupBezierProgram();
    setupFloorProgram();

    // Basic GL settings
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glPatchParameteri(GL_PATCH_VERTICES, 16);
}

void calcUniforms() {
    glm::mat4 proj = glm::perspective(20.0f * TO_RAD, 1.0f, 0.001f, 1000.0f);  // perspective projection matrix
    glm::mat4 view = glm::lookAt(
            glm::vec3(0.0, eyePos.height, eyePos.rad), // eye pos
            glm::vec3(0.0, lookAtHeight, 0.0), // look at pos
            glm::vec3(0.0, 1.0, 0.0)); // up vector
    glm::mat4 mvMatrix = glm::rotate(view, -eyePos.angle * TO_RAD, glm::vec3(0.0, 1.0, 0.0));

    glm::vec4 lightPos = glm::vec4(0.0, 0.0, 500.0, 1.0);
    glm::vec4 lightEye = view * lightPos;

    glm::mat4 norMatrix = glm::inverse(mvMatrix);
    glm::mat4 mvpMatrix = proj * mvMatrix;

    glUniformMatrix4fv(mvMatrixLoc, 1, GL_FALSE, &mvMatrix[0][0]);
    glUniformMatrix4fv(norMatrixLoc, 1, GL_FALSE, &norMatrix[0][0]);
    glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, &mvpMatrix[0][0]);
    glUniformMatrix4fv(floorMvpMatrixLoc, 1, GL_FALSE, &mvpMatrix[0][0]);
    glUniform4fv(lightPosLoc, 1, &lightEye[0]);
    glUniform1i(timeSinceExplosionLoc, timeSinceExplosion);
}

void setTessLevel() {
    int lhigh = 20;
    int llow = 2;

    float dmin, dmax;
    if (isBigModel) {
        dmin = 10.0;
        dmax = 150.0;
    } else {
        dmin = 5.0;
        dmax = 70.0;
    }

    float relDistance = (eyePos.rad - dmin) / (dmax - dmin);
    int tessLevel = relDistance * (llow - lhigh) + lhigh;
    if (tessLevel < llow) tessLevel = llow;
    else if (tessLevel > lhigh) tessLevel = lhigh;

    cout << "tessLevel: " << tessLevel << endl;
    glUniform1i(tessLevelLoc, tessLevel);
}

void display() {
    cout << "distance: " <<  eyePos.rad << endl;
    setTessLevel();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw floor
    glUseProgram(floorProgram);
    calcUniforms();
    setPolygonMode(true);
    glBindVertexArray(floorVao);
    glDrawElements(GL_QUADS, 81 * 4, GL_UNSIGNED_SHORT, NULL);

    // Draw bezier
	glUseProgram(bezierProgram);
    calcUniforms();
    setPolygonMode(wireframeMode);
	glBindVertexArray(bezierVao);
    glDrawArrays(GL_PATCHES, 0, numVertices);

    glBindVertexArray(0);
	glFlush();
}

void special(int key, int x, int y) {
    const float CHANGE_VIEW_ANGLE = 2.0;
    const float RAD_INCR = 1.0;

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
        case 'a':
            eyePos.height += VERT_INCR;
            break;
        case 'z':
            eyePos.height -= VERT_INCR;
            break;
        case 'i':
            lookAtHeight += VERT_INCR;
            break;
        case 'k':
            lookAtHeight -= VERT_INCR;
            break;
        case 'w':
            wireframeMode = !wireframeMode;
            break;
        case ' ':
            hasExploded = true;
            break;
    }
    glutPostRedisplay();
}

void timer(int value) {
    if (hasExploded) {
        timeSinceExplosion++;
        cout << "time: " << timeSinceExplosion << endl;
    }
    glutPostRedisplay();
    glutTimerFunc(TIMER_INTERVAL, timer, 0);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutCreateWindow(MODEL_FILENAME);
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
    glutTimerFunc(TIMER_INTERVAL, timer, 0);
	glutMainLoop();
}
