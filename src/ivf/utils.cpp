#include <ivf/utils.h>

#include <iostream>
#include <cstdio>
#include <string>
#include <cstdlib>

#include <glad/glad.h>

using namespace ivf;
using namespace std;

GLenum ivf::checkPrintError(const std::string file, const long line)
{
	GLenum err = glGetError();

	switch (err)
	{
	case GL_NO_ERROR:
		break;
	case GL_INVALID_ENUM:
		cout << file << ", line " << line << " - Error: An unacceptable value is specified for an enumerated argument. (GL_INVALID_ENUM)" << endl;
		break;
	case GL_INVALID_VALUE:
		cout << file << ", line " << line << " - Error: A numeric argument is out of range. (GL_INVALID_VALUE)" << endl;
		break;
	case GL_INVALID_OPERATION:
		cout << file << ", line " << line << " - Error: The specified operation is not allowed in the current state. (GL_INVALID_OPERATION)" << endl;
		break;
	/*
	case GL_STACK_OVERFLOW:
		cout << "Error: This command would cause a stack overflow. (GL_STACK_OVERFLOW)" << endl;
		break;
	case GL_STACK_UNDERFLOW:
		cout << "Error: This command would cause a stack underflow. (GL_STACK_UNDERFLOW)" << endl;
		break;
	case GL_TABLE_TOO_LARGE:
		cout << "Error: The specified table exceeds the implementation's maximum supported table size. (GL_TABLE_TOO_LARGE)" << endl;
		break;
	*/
	case GL_OUT_OF_MEMORY:
		cout << file << ", line " << line << " - Error: There is not enough memory left to execute the command. (GL_OUT_OF_MEMORY)" << endl;
		break;
	default:
		cout << file << ", line " << line << " - Error: Unknown error code." << endl;
		break;
	}

	return err;
}

ivf::TransformManager* ivf::xfmMgr()
{
	return ivf::TransformManager::instance();
}


float ivf::random(float a, float b)
{
	double r = (double)rand() / (double)RAND_MAX;
	return a + (b - a) * float(r);
}

double ivf::random(double a, double b)
{
	double r = (double)rand() / (double)RAND_MAX;
	return a + (b - a) * r;
}

