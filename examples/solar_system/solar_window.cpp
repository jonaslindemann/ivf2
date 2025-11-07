#include "solar_window.h"

using namespace solar_ui;
using namespace ivfui;
using namespace ivf;
using namespace std;
using namespace solar;

SolarWindow::SolarWindow(int width, int height, std::string title) : ivfui::GLFWSceneWindow(width, height, title)
{}

std::shared_ptr<SolarWindow> SolarWindow::create(int width, int height, std::string title)
{
    return std::make_shared<SolarWindow>(width, height, title);
}

int SolarWindow::onSetup()
{

    // Create solar system simulation object.

    m_solarSystem = SolarSystem::create();
    m_solarSystem->setSize(200, 2);

    // Define simulation callbacks.

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


    // Setup lighting

    m_lightMgr = LightManager::create();
    m_lightMgr->clearLights();
    m_lightMgr->enableLighting();

    auto pointLight = m_lightMgr->addPointLight();
    pointLight->setDiffuseColor(glm::vec3(1.0, 1.0, 1.0));
    pointLight->setPosition(glm::vec3(0.0, 0.0, 0.0));
    pointLight->setEnabled(true);
    m_lightMgr->apply();

    // Create scene

    m_planetMaterial = Material::create();
    m_planetMaterial->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));

    m_sunMaterial = Material::create();
    m_sunMaterial->setDiffuseColor(glm::vec4(1.0, 1.0, 0.0, 1.0));
    m_sunMaterial->setAmbientColor(glm::vec4(1.0, 1.0, 0.0, 1.0));
    m_sunMaterial->setUseLighting(false);

    m_tracerMaterial = Material::create();
    m_tracerMaterial->setDiffuseColor(glm::vec4(0.0, 1.0, 1.0, 1.0));
    m_tracerMaterial->setUseLighting(false);

    // Create composite nodes to contain our planets:

    m_planets = CompositeNode::create();
    m_suns = CompositeNode::create();
    m_tracers = CompositeNode::create();

    this->add(m_planets);
    this->add(m_suns);
    this->add(m_tracers);

    // Setup camera

    this->cameraManipulator()->setCameraPosition(glm::vec3(0.0, 20.0, 100.0));
    this->cameraManipulator()->setCameraTarget(glm::vec3(0.0, 0.0, 0.0));
    this->cameraManipulator()->setFarZ(1000.0);
    this->cameraManipulator()->setFov(45.0);
    this->cameraManipulator()->setMouseScaling(0.1, 0.1);

    // Setup UI

    m_solarPanel = SolarPanel::create("Solar system settings");
    m_solarPanel->setSolarSystem(m_solarSystem);
    m_solarPanel->setPlanetMaterial(m_planetMaterial);
    m_solarPanel->setSunMaterial(m_sunMaterial);
    m_solarPanel->setTracerMaterial(m_tracerMaterial);
    m_solarPanel->setShowTracers(true);
    m_solarPanel->setTracersPerPlanet(100);
    m_solarPanel->update();

    // Initialise solar system -> Calls callbacks to create planets and suns.

    m_solarSystem->init();

    m_pointLightWindow = PointLightWindow::create(pointLight, "Point Light");
    m_pointLightWindow->setVisible(true);

    m_fpsWindow = FpsWindow::create();

    this->addUiWindow(m_solarPanel);
    this->addUiWindow(m_pointLightWindow);
    this->addUiWindow(m_fpsWindow);

    return 0;
}

void solar_ui::SolarWindow::onUpdate()
{

    if ((m_pointLightWindow->isDirty()))
        m_lightMgr->apply();

    m_tracers->setVisible(m_solarPanel->showTracers());

    m_solarSystem->update(1.0 / 60.0);
}

void solar_ui::SolarWindow::onClearPlanets()
{
    m_planets->clear();
    m_tracers->clear();
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

    // Create tracer for planet

    auto tracer = TraceNode::create(m_solarPanel->tracersPerPlanet());
    tracer->setNode(planet);
    tracer->setMaterial(m_tracerMaterial);
    tracer->setUseColor(false);
    tracer->refresh();
    m_tracers->add(tracer);
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
