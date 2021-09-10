#ifndef __arc2_extrusion__
#define __arc2_extrusion__

#include <ivf/meshnode.h>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <vector>

namespace ivf {

    class Extrusion : public MeshNode {
    private:
        std::vector<glm::vec3> m_path;
        std::vector<glm::vec3> m_section;
        std::vector<glm::vec3> m_smoothForward;
    public:
        Extrusion();

        static std::shared_ptr<Extrusion> create();

        void addPathPoint(glm::vec3 p);
        void clearPath();

        void addSectionPoint(glm::vec2 p);
        void clearSection();

    protected:
        virtual void doSetup();
        virtual void doPreDraw();
        virtual void doPostDraw();
    };

    typedef std::shared_ptr<Extrusion> ExtrusionPtr;

};

#endif