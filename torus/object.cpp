#include "object.hpp"

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
