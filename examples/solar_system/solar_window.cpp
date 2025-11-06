#include "solar_window.h"

using namespace solar_ui;
using namespace ivfui;
using namespace ivf;
using namespace std;
using namespace solar;

SolarWindow::SolarWindow(int width, int height, std::string title) : ivfui::GLFWWindow(width, height, title)
{}

std::shared_ptr<SolarWindow> SolarWindow::create(int width, int height, std::string title)
{
    return std::make_shared<SolarWindow>(width, height, title);
}

int SolarWindow::onSetup()
{
    m_solarSystem = SolarSystem::create();
    m_solarSystem->setSize(200, 2);

    using namespace std::placeholders;

    m_clearPlanetsFunc = std::bind(&SolarWindow::onClearPlanets, this);
    m_solarSystem->assignClearPlanetsFunc(m_clearPlanetsFunc);

    m_clearSunsFunc = std::bind(&SolarWindow::onClearSuns, this);
    m_solarSystem->assignClearSunsFunc(m_clearSunsFunc);

    m_createPlanetFunc = std::bind(&SolarWindow::onCreatePlanet, this, _1, _2, _3, _4, _5);
    m_solarSystem->assignCreatePlanetFunc(m_createPlanetFunc);

    m_createSunFunc = std::bind(&SolarWindow::onCreateSun, this, _1, _2, _3, _4, _5);
    m_solarSystem->assignCreateSunFunc(m_createSunFunc);

    m_updatePlanetPosFunc = std::bind(&SolarWindow::onUpdatePlanetPos, this, _1, _2, _3, _4);
    m_solarSystem->assignUpdatePlanetPosFunc(m_updatePlanetPosFunc);

    glEnable(GL_DEPTH_TEST);

    auto fontMgr = FontManager::create();
    fontMgr->loadFace("fonts/Gidole-Regular.ttf", "gidole");

    ShaderManagerPtr shaderMgr = ShaderManager::create();
    shaderMgr->loadBasicShader();

    if (shaderMgr->compileLinkErrors())
    {
        cout << "Couldn't compile shaders, exiting..." << endl;
        return -1;
    }

    m_lightMgr = LightManager::create();
    m_lightMgr->enableLighting();

    auto dirLight = m_lightMgr->addDirectionalLight();
    dirLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
    dirLight->setDirection(glm::vec3(-1.0, -1.0, -1.0));
    dirLight->setEnabled(false);
    m_lightMgr->apply();

    auto pointLight = m_lightMgr->addPointLight();
    pointLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
    pointLight->setPosition(glm::vec3(0.0, 0.0, 0.0));
    pointLight->setEnabled(true);
    m_lightMgr->apply();

    m_pointLightWindow = PointLightWindow::create(pointLight, "Point Light");
    m_pointLightWindow->setVisible(true);

    m_fpsWindow = FpsWindow::create();

    m_scene = CompositeNode::create();

    AxisPtr axis = Axis::create();

    m_scene->add(axis);

    m_planetMaterial = Material::create();
    m_planetMaterial->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

    m_sunMaterial = Material::create();
    m_sunMaterial->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));
    m_sunMaterial->setAmbientColor(glm::vec4(1.0, 1.0, 0.0, 1.0));
    m_sunMaterial->setUseLighting(false);

    m_planets = CompositeNode::create();
    m_suns = CompositeNode::create();

    m_scene->add(m_planets);
    m_scene->add(m_suns);

    m_camManip = ivfui::CameraManipulator::create(this->ref());
    m_camManip->setCameraPosition(glm::vec3(0.0, 20.0, 100.0));
    m_camManip->setCameraTarget(glm::vec3(0.0, 0.0, 0.0));
    m_camManip->setFarZ(1000.0);
    m_camManip->setFov(45.0);
    m_camManip->setMouseScaling(0.1, 0.1);

    m_solarSystem->init();

    m_solarPanel = SolarPanel::create("Solar system settings");
    m_solarPanel->setSolarSystem(m_solarSystem);
    m_solarPanel->update();

    return 0;
}

void SolarWindow::onDraw()
{
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_solarSystem->update(1.0 / 60.0);

    if ((m_pointLightWindow->isDirty()))
        m_lightMgr->apply();

    m_scene->draw();
}

void solar_ui::SolarWindow::onDrawUi()
{
    m_pointLightWindow->draw();
    m_solarPanel->draw();
    m_fpsWindow->draw();
}

void SolarWindow::onUpdateOtherUi()
{
    m_camManip->update();
}

void solar_ui::SolarWindow::onClearPlanets()
{
    m_planets->clear();
}

void solar_ui::SolarWindow::onClearSuns()
{
    m_suns->clear();
}

void solar_ui::SolarWindow::onCreatePlanet(size_t idx, double x, double y, double z, double m)
{
    auto planet = Sphere::create(m, 20, 20);
    planet->setMaterial(m_planetMaterial);
    planet->setPos(glm::vec3(x, y, z));
    m_planets->add(planet);
}

void solar_ui::SolarWindow::onCreateSun(size_t idx, double x, double y, double z, double m)
{
    auto sun = Sphere::create(m, 20, 20);
    sun->setMaterial(m_sunMaterial);
    sun->setPos(glm::vec3(x, y, z));
    m_suns->add(sun);
}

void solar_ui::SolarWindow::onUpdatePlanetPos(size_t idx, double x, double y, double z)
{
    auto node = m_planets->at(idx);
    Sphere *planet = dynamic_cast<Sphere *>(node.get());
    planet->setPos(glm::vec3(x, y, z));
}
