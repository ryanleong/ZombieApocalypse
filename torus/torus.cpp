// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include <unistd.h>
#include <libgen.h>

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
#include "object.hpp"

int width = 800;
int height = 600;

double iniX = 0;
double iniY = 0;
double iniZ = 15;
double iniHoriz = 0;
double iniVert = 3.14;

char * inputTemplate = NULL;
int inputCounter = 0;
glob_t inputGlob;

char * outputTemplate;
char defaultOutputTemplate[256] = "torus";
int imageCounter = 0;

int justPhoto = 0;

GLuint Texture;

int skip = 0;
float alpha = 0.6;
GLint alphaLocation;

void parseArgs(int argc, char ** argv) {
	int command;
	while ((command = getopt(argc, argv, "spioj")) != -1) {
		switch (command) {
		case 's':
			width = atoi(argv[optind++]);
			height = atoi(argv[optind++]);
			break;
		case 'p':
			iniX = atof(argv[optind++]);
			iniY = atof(argv[optind++]);
			iniZ = atof(argv[optind++]);
			iniHoriz = atof(argv[optind++]);
			iniVert = atof(argv[optind++]);
			break;
		case 'i':
			inputTemplate = argv[optind++];
			glob(inputTemplate, GLOB_TILDE, NULL, &inputGlob);
			break;
		case 'o':
			outputTemplate = argv[optind++];
			break;
		case 'j':
			justPhoto = !justPhoto;
			break;
		}
	}

	if (outputTemplate == NULL) {
		outputTemplate = defaultOutputTemplate;
	}

	if (inputTemplate == NULL || inputGlob.gl_pathc == 0) {
		printf("Specify the input images glob rule by -i\n");
		exit(1);
	}
}

void saveImage() {
	char outputFilename[256];
	sprintf(outputFilename, "%s-%d.png", outputTemplate, imageCounter);
	save_image(outputFilename, width, height);

	printf("Image %s has been saved.\n", outputFilename);
	imageCounter++;
}

void loadTexture() {
	png_texture_load(inputGlob.gl_pathv[inputCounter], NULL, NULL, Texture);
	printf("Texture %s has been loaded.\n", inputGlob.gl_pathv[inputCounter]);
}

void keyFunc(GLFWwindow * window, int key, int scanCode, int action, int mods) {
	if (action != GLFW_PRESS) {
		return;
	}

	switch (key) {
	case GLFW_KEY_PAGE_DOWN:
		inputCounter = inputCounter + (skip == 0 ? 1 : skip);
		if (inputCounter >= inputGlob.gl_pathc) {
			inputCounter = inputGlob.gl_pathc - 1;
		}
		skip = 0;
		loadTexture();
		break;

	case GLFW_KEY_PAGE_UP:
		inputCounter = inputCounter - (skip == 0 ? 1 : skip);
		if (inputCounter < 0) {
			inputCounter = 0;
		}
		skip = 0;
		loadTexture();
		break;

	case GLFW_KEY_HOME:
		inputCounter = 0;
		loadTexture();
		break;

	case GLFW_KEY_END:
		inputCounter = inputGlob.gl_pathc - 1;
		loadTexture();
		break;

	case GLFW_KEY_ENTER:
		saveImage();
		break;

	case GLFW_KEY_0 ... GLFW_KEY_9:
		skip = skip * 10 + (key - GLFW_KEY_0);
		break;

	case GLFW_KEY_LEFT_BRACKET:
		alpha -= 0.01 * (skip == 0 ? 1 : skip);
		if (alpha < 0) {
			alpha = 0;
		}
		skip = 0;
		break;

	case GLFW_KEY_RIGHT_BRACKET:
		alpha += 0.01 * (skip == 0 ? 1 : skip);
		if (alpha > 1) {
			alpha = 1;
		}
		skip = 0;
		break;

	default:
		skip = 0;
		break;
	}
}

int initWindow() {
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
			"Torus with semi-transparent texture",
			NULL,
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

	glfwSetKeyCallback(window, keyFunc);

	return 0;
}

int main(int argc, char ** argv) {
	parseArgs(argc, argv);

	if (initWindow() != 0) {
		return -1;
	}

	setPosition();

// OBJECT STARTS

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	char vertexShaderPath[256];
	char fragmentShaderPath[256];

	char * dir = dirname(argv[0]);
	sprintf(vertexShaderPath, "%s/TransformVertexShader.vertexshader", dir);
	sprintf(fragmentShaderPath, "%s/TextureFragmentShader.fragmentshader", dir);

// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders(vertexShaderPath, fragmentShaderPath);

// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

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

// OBJECT ENDS

	alphaLocation = glGetUniformLocation(programID, "alpha");

	glGenTextures(1, &Texture);
	loadTexture();

	do {

		glDisable(GL_CULL_FACE);

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

		glUniform1f(alphaLocation, alpha);

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
				2,		// size : U+V => 2
				GL_FLOAT,		// type
				GL_FALSE,		// normalized?
				0,		// stride
				(void*) 0		// array buffer offset
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

		if (justPhoto) {
			saveImage();
			break;
		}
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS
			&& glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS
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

