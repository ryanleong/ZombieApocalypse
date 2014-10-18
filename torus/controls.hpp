#ifndef CONTROLS_HPP
#define CONTROLS_HPP

void setPosition(double x, double y, double z, double horiz, double vert, int w,
		int h);
void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

#endif
