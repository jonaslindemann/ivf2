/**
 * @file vis_readers1.cpp
 * @brief ivfvis: reading data files - CSV, NumPy and VTK XML.
 * @ingroup general_examples
 *
 * Demonstrates the ivfvis readers, one file of each kind (assets/data, written by
 * make_data.py):
 *  - stars.csv: a table read with ivfvis::readCsv() and turned into a point cloud with
 *    ivfvis::pointsFromTable(); the other columns become arrays to colour by
 *  - blobs.npy: a NumPy array read with ivfvis::readNpy(), as a UniformGrid shown by an
 *    isosurface (ivfvis::gridFromNpy())
 *  - boy.vtu: a VTK unstructured grid (appended, zlib-compressed data) read with
 *    ivfvis::readVtk(), drawn with ivfvis::createMeshNode() and coloured by its curvature
 */

#include <imgui.h>
#include <ivf/gl.h>
#include <ivf/logger.h>
#include <ivf/nodes.h>
#include <ivfui/ui.h>
#include <ivfvis/ivfvis.h>

#include <string>

using namespace ivf;
using namespace ivfui;

class ReadersWindow : public GLFWSceneWindow {
private:
    ivfvis::PointSetPtr m_stars;
    ivfvis::PointCloudPtr m_cloud;
    ivfvis::ColormapPtr m_starMap;
    int m_starArray{0};
    std::size_t m_triangles{0};
    std::size_t m_gridPoints{0};

public:
    ReadersWindow(int w, int h, const std::string &title) : GLFWSceneWindow(w, h, title)
    {}

    [[nodiscard]] static std::shared_ptr<ReadersWindow> create(int w, int h, const std::string &title)
    {
        return std::make_shared<ReadersWindow>(w, h, title);
    }

    static TextNodePtr label(const std::string &text, const glm::vec3 &pos)
    {
        auto node = TextNode::create();
        node->setText(text);
        node->setSize(0.22f);
        node->setColor({0.85f, 0.85f, 0.85f});
        node->setAlignX(TextAlignX::CENTER);
        node->setPos(pos);
        return node;
    }

    int onSetup() override
    {
        auto lights = renderContext()->lights();
        lights->clearLights();
        auto sun = lights->addDirectionalLight();
        sun->setDirection({-0.3f, -1.0f, -0.6f});
        sun->setDiffuseColor({0.8f, 0.8f, 0.8f});
        sun->setSpecularColor({0.3f, 0.3f, 0.3f});
        sun->setAmbientColor({0.35f, 0.35f, 0.35f});
        lights->apply();

        // CSV: a galaxy of stars
        auto table = ivfvis::readCsv("assets/data/stars.csv");
        if (!table)
            return -1;
        auto stars = ivfvis::pointsFromTable(*table, "x", "y", "z");
        if (!stars)
            return -1;
        m_stars = *stars;
        m_starMap = ivfvis::Colormap::create(ivfvis::BuiltinColormap::Inferno);
        m_cloud = ivfvis::PointCloud::create(m_stars);
        m_cloud->setPointSize(0.035f);
        m_cloud->setShape(ivfvis::PointShape::Sphere);
        m_cloud->setPos({-3.0f, 0.0f, 0.0f});
        m_cloud->setScale({0.6f, 0.6f, 0.6f});
        m_cloud->setEulerAngles(35.0f, 0.0f, 0.0f); // tilt the disc towards the camera
        applyStars();
        add(m_cloud);
        add(label("stars.csv", {-3.0f, -1.2f, 0.0f}));

        // NumPy: a density field
        auto array = ivfvis::readNpy("assets/data/blobs.npy");
        if (!array)
            return -1;
        auto grid = ivfvis::gridFromNpy(*array, "density", {-1, -1, -1}, glm::vec3(2.0f / 39.0f));
        if (!grid)
            return -1;
        m_gridPoints = (*grid)->pointCount();
        auto blobs = ivfvis::Isosurface::create(*grid, "density", 0.5f);
        auto blobMaterial = Material::create();
        blobMaterial->setDiffuseColor({0.35f, 0.7f, 1.0f, 1.0f});
        blobMaterial->setSpecularColor({0.5f, 0.5f, 0.5f, 1.0f});
        blobMaterial->setShininess(40.0f);
        blobs->setMaterial(blobMaterial);
        add(blobs);
        add(label("blobs.npy", {0.0f, -1.2f, 0.0f}));

        // VTK: Boy's surface with its curvature
        auto vtu = ivfvis::readVtk("assets/data/boy.vtu");
        if (!vtu)
            return -1;
        m_triangles = vtu->triangles.size();
        auto boy = ivfvis::createMeshNode(*vtu->points, vtu->triangles);
        auto curvatureMap = ivfvis::Colormap::create(ivfvis::BuiltinColormap::Coolwarm);
        curvatureMap->setRange(-4.0f, 4.0f); // symmetric: the curvature has long tails
        ivfvis::colorVertices(*boy->mesh(), vtu->points->scalars("curvature"), *curvatureMap);
        ivfvis::useVertexColors(*boy);
        boy->setPos({3.0f, 0.0f, 0.0f});
        boy->setScale({0.6f, 0.6f, 0.6f});
        add(boy);
        add(label("boy.vtu", {3.0f, -1.2f, 0.0f}));

        auto bar = ivfvis::ColorBar::create(curvatureMap);
        bar->setTitle("curvature");
        add(bar);

        cameraManipulator()->setCameraPosition({0.0f, 2.2f, 6.8f});
        cameraManipulator()->setCameraTarget({0.0f, -0.2f, 0.0f});
        return 0;
    }

    void applyStars()
    {
        const char *name = m_starArray == 0 ? "brightness" : "age";
        m_starMap->setRange(m_stars->range(name));
        m_cloud->setColorBy(name, m_starMap);
    }

    void onDrawUi() override
    {
        ImGui::SetNextWindowPos({12, 12}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({320, 0}, ImGuiCond_FirstUseEver);
        ImGui::Begin("Data files");
        ImGui::SeparatorText("stars.csv");
        ImGui::Text("%zu points", m_stars->pointCount());
        const char *arrays[] = {"brightness", "age"};
        if (ImGui::Combo("Colour by", &m_starArray, arrays, 2))
            applyStars();
        ImGui::SeparatorText("blobs.npy");
        ImGui::Text("%zu grid points", m_gridPoints);
        ImGui::SeparatorText("boy.vtu");
        ImGui::Text("%zu triangles", m_triangles);
        ImGui::End();
    }
};

int main()
{
    auto app = GLFWApplication::create();
    app->hint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    app->hint(GLFW_CONTEXT_VERSION_MINOR, 3);
    app->hint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto win = ReadersWindow::create(1280, 720, "ivfvis: reading data files");
    app->addWindow(win);
    return app->loop();
}
