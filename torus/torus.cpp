// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "shader.hpp"
#include "png_texture.hpp"
#include "controls.hpp"
#include "save_image.hpp"

int imageCounter = 0;

int initWindow(int width, int height) {
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf( stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(width, height,
			"Torus with semi-transparent texture", NULL,
			NULL);
	if (window == NULL) {
		fprintf( stderr, "Failed to open GLFW window."
				"If you have an Intel GPU, they are not 3.3 compatible."
				"Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	return 0;
}

typedef struct Vertex {
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat u;
	GLfloat v;
} Vertex;

typedef struct Triangle {
	Vertex v1;
	Vertex v2;
	Vertex v3;
	float dist;
} Triangle;

int compareTriangles(const void * a, const void * b) {
	if (((Triangle *) a)->dist == ((Triangle *) b)->dist) {
		return 0;
	} else if (((Triangle *) a)->dist < ((Triangle *) b)->dist) {
		return 1;
	} else {
		return -1;
	}
}

void sortTriangles(Triangle * triangles, int count, glm::mat4 matrix) {
	for (int i = 0; i < count; i++) {
		Triangle * t = triangles + i;
		glm::vec4 center = glm::vec4((t->v1.x + t->v2.x + t->v3.x) / 3,
				(t->v1.y + t->v2.y + t->v3.y) / 3,
				(t->v1.z + t->v2.z + t->v3.z) / 3, 1.0);
		glm::vec4 transformed = matrix * center;
		t->dist = transformed[2];
	}

	qsort(triangles, count, sizeof(Triangle), compareTriangles);
}

void setVertex(Vertex v, GLfloat * vertices, GLfloat * uv) {
	vertices[0] = v.x;
	vertices[1] = v.y;
	vertices[2] = v.z;
	uv[0] = v.u;
	uv[1] = v.v;
}

void prepareBuffers(Triangle * triangles, int count, glm::mat4 matrix,
		GLfloat * vertices, GLfloat * uv) {
	sortTriangles(triangles, count, matrix);
	for (int i = 0; i < count; i++) {
		Triangle * t = triangles + i;
		setVertex(t->v1, vertices + 9 * i + 0, uv + 6 * i + 0);
		setVertex(t->v2, vertices + 9 * i + 3, uv + 6 * i + 2);
		setVertex(t->v3, vertices + 9 * i + 6, uv + 6 * i + 4);
	}
}

void setCoords(double r, double c, int rSeg, int cSeg, int i, int j,
		Vertex * v) {
	const double PI = 3.1415926535897932384626433832795;
	const double TAU = 2 * PI;

	double x = (c + r * cos(i * TAU / rSeg)) * cos(j * TAU / cSeg);
	double y = (c + r * cos(i * TAU / rSeg)) * sin(j * TAU / cSeg);
	double z = r * sin(i * TAU / rSeg);

	v->x = 2 * x;
	v->y = 2 * y;
	v->z = 2 * z;

	v->u = j / (double) cSeg;
	v->v = i / (double) rSeg;
}

int createObject(double r, double c, int rSeg, int cSeg,
		Triangle ** triangles) {
	int count = rSeg * cSeg * 2;
	*triangles = (Triangle *) malloc(count * sizeof(Triangle));

	for (int x = 0; x < rSeg; x++) { // through stripes
		for (int y = 0; y < cSeg; y++) { // through squares on stripe
			Triangle * tPtr = *triangles + ((x * cSeg) + y) * 2;
			setCoords(r, c, rSeg, cSeg, x, y, &(tPtr + 0)->v1);
			setCoords(r, c, rSeg, cSeg, x + 1, y, &(tPtr + 0)->v2);
			setCoords(r, c, rSeg, cSeg, x, y + 1, &(tPtr + 0)->v3);

			setCoords(r, c, rSeg, cSeg, x, y + 1, &(tPtr + 1)->v1);
			setCoords(r, c, rSeg, cSeg, x + 1, y, &(tPtr + 1)->v2);
			setCoords(r, c, rSeg, cSeg, x + 1, y + 1, &(tPtr + 1)->v3);
		}
	}

	return count;
}

int main(int argc, char ** argv) {
	if (argc < 3) {
		fprintf(stderr,
				"./torus screen-width screen-height [x y z horiz-angle vert-angle [file-name]]\n");
		exit(1);
	}

	int width = atoi(argv[1]);
	int height = atoi(argv[2]);

	double iniX = 0;
	double iniY = 0;
	double iniZ = 15;
	double iniH = 3.14;
	double iniV = 0;
	if (argc >= 8) {
		iniX = atof(argv[3]);
		iniY = atof(argv[4]);
		iniZ = atof(argv[5]);
		iniH = atof(argv[6]);
		iniV = atof(argv[7]);
	}
	setPosition(iniX, iniY, iniZ, iniH, iniV, width, height);

	if (initWindow(width, height) != 0) {
		return -1;
	}

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	glDisable(GL_CULL_FACE);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader",
			"TextureFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Load the texture using any two methods
	//GLuint Texture = loadBMP_custom("uvtemplate3.bmp");
	//GLuint Texture = loadDDS("uvtemplate.DDS");
	GLuint Texture = png_texture_load("/home/adam/UNIMELB/pmc/ZombieApocalypse/testing/mpi-40tasks/n-256/s-8192-8192/t-32/images/step-001000.png", NULL, NULL);

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	Triangle * triangles;
	int count = createObject(1, 2, 100, 100, &triangles);

	int vertices = count * 3;
	GLfloat * g_vertex_buffer_data = (GLfloat *) malloc(
			vertices * 3 * sizeof(GLfloat));
	GLfloat * g_uv_buffer_data = (GLfloat *) malloc(
			vertices * 2 * sizeof(GLfloat));

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);

	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDisable(GL_DEPTH_TEST);
		//glDepthMask(GL_TRUE);
		glDepthFunc(GL_NEVER);
		//glDepthRange(0.0f, 1.0f);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input

		computeMatricesFromInputs();

		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		prepareBuffers(triangles, count, MVP, g_vertex_buffer_data,
				g_uv_buffer_data);

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices * 3 * sizeof(GLfloat),
				g_vertex_buffer_data, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices * 2 * sizeof(GLfloat),
				g_uv_buffer_data, GL_STATIC_DRAW);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0,// attribute. No particular reason for 0, but must match the layout in the shader.
				3,		// size
				GL_FLOAT,		// type
				GL_FALSE,		// normalized?
				0,		// stride
				(void*) 0		// array buffer offset
				);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1,// attribute. No particular reason for 1, but must match the layout in the shader.
				2,				// size : U+V => 2
				GL_FLOAT,				// type
				GL_FALSE,				// normalized?
				0,				// stride
				(void*) 0				// array buffer offset
				);

		glFrontFace(GL_CW);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices * 3);// 12*3 indices starting at 0 -> 12 triangles

		glFrontFace(GL_CCW);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS || argc >= 10) {
			char * filename = NULL;
			char defaultName[] = "torus";
			if (argc >= 9) {
				filename = argv[8];
			} else {

				filename = defaultName;
			}
			char imageFilename[256];
			sprintf(imageFilename, "%s-%d.png", filename, imageCounter);
			save_image(imageFilename, width, height);

			printf("Image %s has been saved.\n", imageFilename);
			imageCounter++;

			if (argc >= 10) {
				break;
			}
		}
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS
			&& glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &TextureID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

