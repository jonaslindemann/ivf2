#pragma once

#include <ivf/mesh_node.h>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <vector>

//
// Based on code by Song ho Ahn
//
// http://www.songho.ca/opengl/gl_cylinder.html#example_pipe
//

namespace ivf {

    class Extrusion : public MeshNode {
    private:
        std::vector<glm::vec3> m_path;
        std::vector<glm::vec3> m_contour;
        std::vector<std::vector<glm::vec3>> m_contours;
        std::vector<std::vector<glm::vec3>> m_normals;

        void generateContours();
        void transformFirstContour();
        std::vector<glm::vec3> projectContour(int fromIndex, int toIndex);
        std::vector<glm::vec3> computeContourNormal(int pathIndex);
        
        glm::vec3 intersectPlane(glm::vec3& pos, glm::vec3& dir, glm::vec3& planePos, glm::vec3& planeDir);


    public:
        Extrusion();

        static std::shared_ptr<Extrusion> create();

        void addPathPoint(glm::vec3 p);
        void clearPath();

        void addSectionPoint(glm::vec2 p);
        void clearSection();

        void createCircleSection(float r, int segments=36);


    protected:
        virtual void doSetup();
        virtual void doPreDraw();
        virtual void doPostDraw();
    };

    typedef std::shared_ptr<Extrusion> ExtrusionPtr;

};
