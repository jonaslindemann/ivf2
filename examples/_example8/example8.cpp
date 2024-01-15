#include <iostream>
#include <memory>
#include <vector>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivf/ui.h>

#include <ivf/utils.h>

#include <generator/generator.hpp>

#include <cmath>

using namespace ivf;
using namespace std;
using namespace generator;

int main()
{
	double t = 0.0;
	double dt = 0.1;

	LinearInterpolFunc linInterpol;

	linInterpol.addPoint(glm::vec3(0.0, 0.0, 0.0));
	linInterpol.addPoint(glm::vec3(1.0, 0.0, 0.0));
	linInterpol.addPoint(glm::vec3(1.0, 1.0, 0.0));
	linInterpol.addPoint(glm::vec3(0.0, 1.0, 0.0));
	linInterpol.addPoint(glm::vec3(0.0, 1.0, 1.0));

	glm::vec3 pos;

	while (t < 4.0)
	{
		pos = linInterpol(t);
		cout << pos.x << ", " << pos.y << ", " << pos.z << endl;
		t += dt;
	}
}