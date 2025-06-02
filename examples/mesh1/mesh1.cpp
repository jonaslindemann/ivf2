#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include <ivf/gl.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>

using namespace ivf;
using namespace ivfui;
using namespace std;

class ExampleWindow : public GLFWSceneWindow {
private:
    PlanePtr m_plane;
    RoundedBoxPtr m_box;
    VerticesPtr m_vertices;
    IndicesPtr m_indices;

public:
    ExampleWindow(int width, int height, std::string title) : GLFWSceneWindow(width, height, title)
    {}

    static std::shared_ptr<ExampleWindow> create(int width, int height, std::string title)
    {
        return std::make_shared<ExampleWindow>(width, height, title);
    }

    virtual int onSetup() override
    {
        auto axis = Axis::create();

        this->add(axis);

        auto redMaterial = Material::create();
        redMaterial->setDiffuseColor(glm::vec4(1.0, 0.0, 0.0, 1.0));

        auto greenMaterial = Material::create();
        greenMaterial->setDiffuseColor(glm::vec4(0.0, 1.0, 0.0, 1.0));

        auto blueMaterial = Material::create();
        blueMaterial->setDiffuseColor(glm::vec4(0.0, 0.0, 1.0, 1.0));

        auto yellowMaterial = Material::create();
        yellowMaterial->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

        mmDefaultMeshUsage(GL_DYNAMIC_DRAW);

        m_plane = Plane::create(10.0, 10.0, 200, 200);
        m_plane->setMaterial(blueMaterial);
        m_plane->setWireframe(false);

        m_box = RoundedBox::create();
        m_box->setMaterial(yellowMaterial);

        auto mesh = m_box->mesh(0);

        m_indices = mesh->indices();
        m_vertices = mesh->vertices();

        cout << "Vertices: " << m_vertices->rows() << " x " << m_vertices->cols() << endl;
        cout << "Indices: " << m_indices->rows() << " x " << m_indices->cols() << endl;

        // Animate the plane

        this->add(m_plane);
        this->add(m_box);

        return 0;
    }

    virtual void onUpdate()
    {
        for (int i = 0; i < m_vertices->rows(); ++i)
        {
            float x = m_vertices->at(i, 0);
            float y = m_vertices->at(i, 1);
            float z = m_vertices->at(i, 2);
            // Simple sine wave animation

            float r = std::sqrt(x * x + z * z);

            float dy = std::sin(r + glfwGetTime()) * 0.5f; // Adjust amplitude as needed

            m_vertices->setVertex(i, x, y + dy, z);
        }

        m_box->updateVertices();
        m_box->updateNormals();
    }
};

typedef std::shared_ptr<ExampleWindow> ExampleWindowPtr;

int main()
{
    auto app = GLFWApplication::create();

    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    app->hint(GLFW_SAMPLES, 4);

    auto window = ExampleWindow::create(800, 800, "Animation");
    window->maximize();

    app->addWindow(window);
    return app->loop();
}
