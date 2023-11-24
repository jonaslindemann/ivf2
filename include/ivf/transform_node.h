#pragma once

#include <ivf/node.h>

#include <glm/glm.hpp>

namespace ivf {

    class TransformNode : public Node {
    private:
        glm::vec3 m_pos;
        glm::vec3 m_rotAxis;
        float m_rotAngle;
        glm::vec3 m_scale;
        bool m_useTransform;
    public:
        TransformNode();

        void setPos(glm::vec3 pos);
        glm::vec3 pos();

        void setUseTransform(bool flag);
        bool useTransform();
    protected:
        virtual void doPreDraw();
        virtual void doPostDraw();
    };

    typedef std::shared_ptr<TransformNode> TransformNodePtr;

};


