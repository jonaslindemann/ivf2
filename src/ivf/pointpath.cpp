#include <ivf/pointpath.h>

#include <iostream>

using namespace ivf;
using namespace generator;
using namespace std;

PointPath::PointPath(int segments, std::vector<glm::vec3> points) 
	:m_segments(segments),
	 parametricPath_{
		[segments, this, points](double t) {
			PathVertex vertex;

			LinearInterpolFunc interpFunc;
			for (auto& p : points)
				interpFunc.addPoint(p);

			glm::vec3 prev = interpFunc(t);
			glm::vec3 next = interpFunc(t + 0.1);

			glm::vec3 tangent = glm::normalize(next - prev);
			glm::vec3 normal = glm::normalize(glm::cross(next - prev, next + prev));
			//glm::vec3 normal = glm::normalize(glm::cross(tangent, glm::vec3(0.0, 1.0, 0.0)));

			glm::vec3 v = interpFunc(t);

			vertex.position[0] = v.x;
			vertex.position[1] = v.y;
			vertex.position[2] = v.z;
			vertex.normal[0] = normal.x;
			vertex.normal[1] = normal.y;
			vertex.normal[2] = normal.z;
			vertex.tangent[0] = tangent.x;
			vertex.tangent[1] = tangent.y;
			vertex.tangent[2] = tangent.z;

			cout << "v: (" << vertex.position[0] << ", " << vertex.position[1] << ", " << vertex.position[2] << ")" << endl;
			cout << "n: (" << vertex.normal[0] << ", " << vertex.normal[1] << ", " << vertex.normal[2] << ")" << endl;
			//cout << "t: (" << vertex.texCoord[0] << ", " << vertex.texCoord[1] << ")" << endl;

			vertex.texCoord = t;

			/*
			const double angle = start + t * sweep;
			const double sine = std::sin(angle);
			const double cosine = std::cos(angle);

			vertex.position = gml::dvec3{
				radius * cosine,
				radius * sine,
				2.0 * t * size - size
			};

			vertex.tangent = gml::normalize(gml::dvec3{
				-radius * sine,
				radius * cosine,
				2.0 * size / sweep,
			});

			vertex.normal = gml::dvec3{cosine, sine, 0.0};

			vertex.texCoord = t;

			*/

			return vertex;
		},
		segments
}
{ }
