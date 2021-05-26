#ifndef __ivf2_utils__
#define __ivf2_utils__

#include <glad/glad.h>

#include <string>

#include <ivf/transformmanager.h>

namespace ivf {
	GLenum checkPrintError(const std::string file="", const long line=0);   
	float random(float a, float b);
	double random(double a, double b);

	TransformManager* xfmMgr();
};

#endif
