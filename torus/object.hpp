#ifndef OBJECT_HPP_
#define OBJECT_HPP_

// Include GLEW
#include <GL/glew.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

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

void prepareBuffers(Triangle * triangles, int count, glm::mat4 matrix,
		GLfloat * vertices, GLfloat * uv);

int createObject(double r, double c, int rSeg, int cSeg,
		Triangle ** triangles);


#endif /* OBJECT_HPP_ */
