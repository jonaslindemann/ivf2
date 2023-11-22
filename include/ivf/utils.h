#ifndef __ivf2_utils__
#define __ivf2_utils__

#include <glad/glad.h>

#include <string>

#include <ivf/transformmanager.h>

#ifdef _DEBUG
#define GL_ERR(stmt) ivf::clearError(); \
            stmt; \
			ivf::checkPrintError(#stmt, __FILE__, __LINE__); 
#define GL_ERR_BEGIN ivf::clearError();
#define GL_ERR_END(name) ivf::checkPrintError(name, __FILE__, __LINE__); 
#else
#define GL_ERR(stmt) stmt;
#define GL_ERR_BEGIN
#define GL_ERR_END(name)
#endif

namespace ivf {

	class LinearInterpolFunc {
	private:
		std::vector<glm::vec3> m_points;

		float tri(float t);
	public:
		LinearInterpolFunc();

		void addPoint(glm::vec3 p);
		void clear();
		int size();

		glm::vec3 operator()(float t);
	};

	void clearError();
	GLenum checkPrintError(const std::string context, const std::string file="", const long line=0);   
	float random(float a, float b);
	double random(double a, double b);

	TransformManager* xfmMgr();
};

#endif
