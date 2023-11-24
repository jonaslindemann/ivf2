#pragma once

#include <ivf/transform_node.h>

#include <generator/generator.hpp>

#include <ivf/mesh.h>
#include <ivf/material.h>

#include <vector>

namespace ivf {

    class MeshNode : public TransformNode {
    private:
        std::vector<std::shared_ptr<Mesh>> m_meshes;
    public:
        MeshNode();

        static std::shared_ptr<MeshNode> create();

        void addMesh(std::shared_ptr<Mesh> mesh);
        std::vector<std::shared_ptr<Mesh>> meshes();

        void newMesh(int vsize, int isize = 0, GLuint primType=GL_TRIANGLES);
        std::shared_ptr<Mesh> mesh(int idx = 0);
        std::shared_ptr<Mesh> lastMesh();
        std::shared_ptr<Mesh> currentMesh();
        void clear();

        void createFromGenerator(generator::AnyGenerator<generator::MeshVertex>& vertices, generator::AnyGenerator<generator::Triangle>& triangles);
        void debugFromGenerator(generator::AnyGenerator<generator::MeshVertex>& vertices, generator::AnyGenerator<generator::Triangle>& triangles);

        void refresh();

        void print();

    protected:
        virtual void doDraw();

        virtual void doSetup();
    };

    typedef std::shared_ptr<MeshNode> MeshNodePtr;

};


