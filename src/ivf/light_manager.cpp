#include <ivf/light_manager.h>
#include <ivf/render_context.h>

#include <ivf/shader_manager.h>
#include <ivf/extent_visitor.h>
#include <ivf/renderer.h>
#include <ivf/transform_manager.h>
#include <ivf/shadow_shaders.h>
#include <ivf/gl_debug.h>

#include <algorithm>
#include <array>
#include <cstring>


using namespace ivf;

// The manager lives in the current RenderContext (see ivf/render_context.h).
LightManager *LightManager::instance()
{
    return RenderContext::currentOrFallback()->lights();
}

LightManager *LightManager::create()
{
    return instance();
}

void LightManager::drop()
{
    // Managers are released together with their RenderContext.
}

LightManager::LightManager()
    : m_useLighting(true), m_useVertexColors(false), m_savedState(false), m_useLightingId(-1), m_useVertexColorsId(-1),
      m_useTextureId(-1), m_pointLightCountId(-1), m_directionalLightCountId(-1), m_spotLightCountId(-1),
      m_diffuseColorId(-1), m_specularColorId(-1), m_ambientColorId(-1), m_shininessId(-1), m_alphaId(-1)
{
    m_useLightingId = ShaderManager::instance()->currentProgram()->uniformLoc("useLighting");
    m_useVertexColorsId = ShaderManager::instance()->currentProgram()->uniformLoc("useVertexColors");
    m_pointLightCountId = ShaderManager::instance()->currentProgram()->uniformLoc("pointLightCount");
    m_directionalLightCountId = ShaderManager::instance()->currentProgram()->uniformLoc("dirLightCount");
    m_spotLightCountId = ShaderManager::instance()->currentProgram()->uniformLoc("spotLightCount");

    m_diffuseColorId = ShaderManager::instance()->currentProgram()->uniformLoc("material.diffuseColor");
    m_specularColorId = ShaderManager::instance()->currentProgram()->uniformLoc("material.specularColor");
    m_ambientColorId = ShaderManager::instance()->currentProgram()->uniformLoc("material.ambientColor");
    m_shininessId = ShaderManager::instance()->currentProgram()->uniformLoc("material.shininess");
    m_alphaId = ShaderManager::instance()->currentProgram()->uniformLoc("material.alpha");

    this->setupDefaultColors();
}

LightManager::~LightManager()
{
    if (m_lightsBuffer != 0)
        glDeleteBuffers(1, &m_lightsBuffer);
}

PointLightPtr ivf::LightManager::addPointLight()
{
    auto pointLight = PointLight::create();
    pointLight->setEnabled(true);
    m_pointLights.push_back(pointLight);
    return pointLight;
}

DirectionalLightPtr ivf::LightManager::addDirectionalLight()
{
    auto dirLight = DirectionalLight::create();
    dirLight->setEnabled(true);
    m_dirLights.push_back(dirLight);
    return dirLight;
}

SpotLightPtr ivf::LightManager::addSpotLight()
{
    auto spotLight = SpotLight::create();
    spotLight->setEnabled(true);
    m_spotLights.push_back(spotLight);
    return spotLight;
}

PointLightPtr ivf::LightManager::pointLight(int idx)
{
    if ((idx >= 0) && (static_cast<size_t>(idx) < m_pointLights.size()))
        return m_pointLights[idx];
    else
        return nullptr;
}

DirectionalLightPtr ivf::LightManager::directionalLight(int idx)
{
    if ((idx >= 0) && (static_cast<size_t>(idx) < m_dirLights.size()))
        return m_dirLights[idx];
    else
        return nullptr;
}

SpotLightPtr ivf::LightManager::spotLight(int idx)
{
    if ((idx >= 0) && (static_cast<size_t>(idx) < m_spotLights.size()))
        return m_spotLights[idx];
    else
        return nullptr;
}

size_t ivf::LightManager::pointLightCount()
{
    return m_pointLights.size();
}

size_t ivf::LightManager::dirLightCount()
{
    return m_dirLights.size();
}

size_t ivf::LightManager::spotLightCount()
{
    return m_spotLights.size();
}

void ivf::LightManager::clearPointLights()
{
    for (size_t i = 0; i < m_pointLights.size(); i++)
    {
        m_pointLights[i]->setEnabled(false);
        m_pointLights[i]->apply();
    }
    m_pointLights.clear();
    syncLightsBlock();
}

void ivf::LightManager::clearDirectionalLights()
{
    for (size_t i = 0; i < m_dirLights.size(); i++)
    {
        m_dirLights[i]->setEnabled(false);
        m_dirLights[i]->apply();
    }
    m_dirLights.clear();
    syncLightsBlock();
}

void ivf::LightManager::clearSpotLights()
{
    for (size_t i = 0; i < m_spotLights.size(); i++)
    {
        m_spotLights[i]->setEnabled(false);
        m_spotLights[i]->apply();
    }
    m_spotLights.clear();
    syncLightsBlock();
}

void ivf::LightManager::clearLights()
{
    this->clearPointLights();
    this->clearDirectionalLights();
    this->clearSpotLights();
}

void LightManager::setUseLighting(bool flag)
{
    m_useLighting = flag;

    ShaderManager::instance()->currentProgram()->uniformBool(m_useLightingId, m_useLighting);
}

void ivf::LightManager::setUseVertexColors(bool flag)
{
    m_useVertexColors = flag;

    ShaderManager::instance()->currentProgram()->uniformBool(m_useVertexColorsId, m_useVertexColors);
}

bool LightManager::useLighting() const
{
    return m_useLighting;
}

void LightManager::enableLighting()
{
    this->setUseLighting(true);
    this->setUseVertexColors(false);
}

void LightManager::disableLighting()
{
    this->setUseLighting(false);
    this->setUseVertexColors(true);
}

void ivf::LightManager::setupDefaultColors()
{
    this->setUseLighting(true);
    this->setUseVertexColors(false);
    this->setDiffuseColor(glm::vec3(1.0, 1.0, 0.0));
    this->setAmbientColor(glm::vec3(0.2, 0.2, 0.2));
    this->setSpecularColor(glm::vec3(1.0, 1.0, 1.0));
    this->setAlpha(1.0);
}

LightsBlock LightManager::buildLightsBlock()
{
    LightsBlock block;

    const int points = std::min(static_cast<int>(m_pointLights.size()), MaxPointLights);
    for (int i = 0; i < points; i++)
    {
        auto &light = *m_pointLights[i];
        auto &data = block.pointLights[i];
        data.position = light.position();
        data.enabled = light.enabled() ? 1 : 0;
        data.ambientColor = light.ambientColor();
        data.diffuseColor = light.diffuseColor();
        data.specularColor = light.specularColor();
        data.constant = light.constAttenuation();
        data.linear = light.linearAttenutation();
        data.quadratic = light.quadraticAttenuation();
    }

    const int dirs = std::min(static_cast<int>(m_dirLights.size()), MaxDirLights);
    for (int i = 0; i < dirs; i++)
    {
        auto &light = *m_dirLights[i];
        auto &data = block.dirLights[i];
        data.direction = light.direction();
        data.enabled = light.enabled() ? 1 : 0;
        data.ambientColor = light.ambientColor();
        data.diffuseColor = light.diffuseColor();
        data.specularColor = light.specularColor();
        data.castShadows = light.castsShadows() ? 1 : 0;
        data.shadowStrength = light.shadowStrength();
        // The matrix the shadow pass renders with (renderShadowMaps() uses the same box).
        if (light.enabled() && light.castsShadows() && light.shadowMap())
            block.lightSpaceMatrices[i] = light.calculateLightSpaceMatrix(m_sceneBBox);
    }

    const int spots = std::min(static_cast<int>(m_spotLights.size()), MaxSpotLights);
    for (int i = 0; i < spots; i++)
    {
        auto &light = *m_spotLights[i];
        auto &data = block.spotLights[i];
        data.position = light.position();
        data.enabled = light.enabled() ? 1 : 0;
        data.direction = light.direction();
        data.ambientColor = light.ambientColor();
        data.diffuseColor = light.diffuseColor();
        data.specularColor = light.specularColor();
        data.constant = light.constAttenuation();
        data.linear = light.linearAttenutation();
        data.quadratic = light.quadraticAttenuation();
        data.cutOff = glm::cos(glm::radians(light.innerCutoff()));
        data.outerCutOff = glm::cos(glm::radians(light.outerCutoff()));
    }

    block.pointLightCount = points;
    block.dirLightCount = dirs;
    block.spotLightCount = spots;
    return block;
}

void LightManager::syncLightsBlock()
{
    const LightsBlock block = buildLightsBlock();

    if (m_lightsBlockValid && std::memcmp(&block, &m_lightsBlock, sizeof(LightsBlock)) == 0)
        return; // unchanged: no upload

    if (m_lightsBuffer == 0)
    {
        if (glGenBuffers == nullptr)
            return; // no OpenGL functions loaded (GL-free tests)
        glGenBuffers(1, &m_lightsBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, m_lightsBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(LightsBlock), nullptr, GL_DYNAMIC_DRAW);
        glLabel(GL_BUFFER, m_lightsBuffer, LightsBlockName);
    }
    else
        glBindBuffer(GL_UNIFORM_BUFFER, m_lightsBuffer);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightsBlock), &block);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, LightsBlockBinding, m_lightsBuffer);

    m_lightsBlock = block;
    m_lightsBlockValid = true;
    if (auto *context = RenderContext::current())
        context->countLightsBlockUpload();
}

const LightsBlock &LightManager::lightsBlock() const
{
    return m_lightsBlock;
}

void LightManager::applyLegacyUniforms(Program &program)
{
    program.uniformInt("pointLightCount", static_cast<int>(m_pointLights.size()));
    program.uniformInt("dirLightCount", static_cast<int>(m_dirLights.size()));
    program.uniformInt("spotLightCount", static_cast<int>(m_spotLights.size()));

    for (auto &light : m_pointLights)
        light->apply();
    for (auto &light : m_dirLights)
        light->apply();
    for (auto &light : m_spotLights)
        light->apply();

    program.uniformMatrix4Array("lightSpaceMatrices", MaxDirLights, m_lightsBlock.lightSpaceMatrices);
}

namespace {

/// True if the program declares plain light uniforms (an old shader without the IvfLights block).
bool hasLegacyLightUniforms(Program &program)
{
    // Block members have no location of their own, so these are -1 for block programs.
    for (const char *name : {"pointLightCount", "dirLightCount", "spotLightCount", "pointLights[0].enabled",
                             "dirLights[0].enabled", "spotLights[0].enabled"})
        if (program.uniformLoc(name) != -1)
            return true;
    return false;
}

} // namespace

void ivf::LightManager::apply()
{
    for (size_t i = 0; i < m_pointLights.size(); i++)
        m_pointLights[i]->setIndex(static_cast<int>(i));
    for (size_t i = 0; i < m_dirLights.size(); i++)
        m_dirLights[i]->setIndex(static_cast<int>(i));
    for (size_t i = 0; i < m_spotLights.size(); i++)
        m_spotLights[i]->setIndex(static_cast<int>(i));

    syncLightsBlock();

    auto program = ShaderManager::instance()->currentProgram();
    if (!program)
        return;

    if (hasLegacyLightUniforms(*program))
        applyLegacyUniforms(*program);

    // Shadow maps: samplers cannot be in a uniform block. dirLights[i] uses shadowMaps[i]
    // on texture unit 1 + i (unit 0 is used for regular textures).
    std::array<int, MaxDirLights> shadowMapUnits{};
    bool anyShadows = false;
    const int dirs = std::min(static_cast<int>(m_dirLights.size()), MaxDirLights);
    for (int i = 0; i < dirs; i++)
    {
        auto &light = m_dirLights[i];
        if (!light->enabled() || !light->castsShadows() || !light->shadowMap())
            continue;

        const int unit = 1 + i;
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, light->shadowMap()->depthTexture());
        shadowMapUnits[i] = unit;

        if (!anyShadows)
        {
            // Single matrix of the first shadow-casting light (used by the debug views).
            program->uniformMatrix4("lightSpaceMatrix", m_lightsBlock.lightSpaceMatrices[i]);
            anyShadows = true;
        }
    }
    glActiveTexture(GL_TEXTURE0);

    if (anyShadows)
    {
        program->uniformBool("useShadows", true);
        program->uniformIntArray("shadowMaps", MaxDirLights, shadowMapUnits.data());
    }
}

void LightManager::renderShadowMaps(CompositeNodePtr scene)
{
    if (!m_useShadows || !scene)
        return;

    RenderContext &context = *RenderContext::currentOrFallback();
    RenderQueue queue;
    CollectState state;
    state.world = context.transforms()->modelMatrix();
    scene->collect(queue, state);
    renderShadowMaps(context, queue);
}

void LightManager::renderShadowMaps(RenderContext &context, const RenderQueue &queue)
{
    m_shadowStats = Renderer::Stats{};
    if (!m_useShadows)
        return;

    // Fit the shadow maps to the scene (before apply(), which uploads the same light
    // matrix for the main pass). Keeps the previous box if no item has bounds.
    if (m_autoCalcBBox)
    {
        const BoundingBox bounds = queue.bounds();
        if (bounds.isValid())
            m_sceneBBox = bounds;
    }

    this->apply();

    // Save current OpenGL state

    GLboolean depthTest;
    glGetBooleanv(GL_DEPTH_TEST, &depthTest);
    GLint polygonMode[2];
    glGetIntegerv(GL_POLYGON_MODE, polygonMode);
    GLboolean cullFace;
    glGetBooleanv(GL_CULL_FACE, &cullFace);

    // Set shadow rendering state

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // Use default depth function
    glCullFace(GL_FRONT); // Can help with shadow acne (or try GL_BACK)

    BoundingBox sceneBBox = m_sceneBBox;

    // Use the current shader (stock shader)

    ProgramPtr shader = context.shaders()->currentProgram();

    // Save current OpenGL state

    GLint previousViewport[4];
    glGetIntegerv(GL_VIEWPORT, previousViewport);

    // The scene may be rendering into an offscreen framebuffer (post-processing),
    // so restore that binding rather than the default framebuffer afterwards.
    GLint previousFramebuffer = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);

    Renderer renderer(context, RenderPass::Shadow);

    // For each shadow-casting light:

    for (auto &light : m_dirLights)
    {
        if (!light->enabled() || !light->castsShadows() || !light->shadowMap())
            continue;

        // Calculate the light space matrix with the current scene bbox

        glm::mat4 lightSpaceMatrix = light->calculateLightSpaceMatrix(sceneBBox);
        light->shadowMap()->setLightSpaceMatrix(lightSpaceMatrix);

        // Bind the shadow map's framebuffer

        light->shadowMap()->bind();

        // Clear the depth buffer

        glClear(GL_DEPTH_BUFFER_BIT);

        // Set shader to shadow pass mode

        shader->use();
        shader->uniformBool("shadowPass", true);
        shader->uniformMatrix4("lightSpaceMatrix", lightSpaceMatrix);

        // Render the scene

        renderer.execute(queue, Frustum(lightSpaceMatrix)); // only what the light sees
        m_shadowStats += renderer.stats();

        // Reset shader mode

        shader->uniformBool("shadowPass", false);

        // Unbind the shadow map's framebuffer

        light->shadowMap()->unbind();
    }

    // Restore framebuffer and viewport

    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(previousFramebuffer));
    glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);

    // Restore regular shader

    shader->use();
    shader->uniformBool("shadowPass", false);

    // Restore OpenGL state
    if (depthTest)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, polygonMode[0]);

    if (cullFace)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
}

void ivf::LightManager::setDiffuseColor(glm::vec3 color)
{
    glm::vec4 colorAlpha(1.0);
    colorAlpha.r = color.r;
    colorAlpha.g = color.g;
    colorAlpha.b = color.b;
    ShaderManager::instance()->currentProgram()->uniformVec3(m_diffuseColorId, colorAlpha);
}

void ivf::LightManager::setDiffuseColor(glm::vec4 color)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_diffuseColorId, glm::vec3(color.r, color.g, color.b));
}

void ivf::LightManager::setDiffuseColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat /*alpha*/)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_diffuseColorId, glm::vec3(red, green, blue));
}

void ivf::LightManager::setSpecularColor(glm::vec3 color)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_specularColorId, color);
}

void ivf::LightManager::setSpecularColor(glm::vec4 color)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_specularColorId, glm::vec3(color.r, color.g, color.b));
}

void ivf::LightManager::setSpecularColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat /*alpha*/)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_specularColorId, glm::vec3(red, green, blue));
}

void ivf::LightManager::setAmbientColor(glm::vec3 color)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_ambientColorId, color);
}

void ivf::LightManager::setAmbientColor(glm::vec4 color)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_ambientColorId, glm::vec3(color.r, color.g, color.b));
}

void ivf::LightManager::setAmbientColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat /*alpha*/)
{
    ShaderManager::instance()->currentProgram()->uniformVec3(m_ambientColorId, glm::vec3(red, green, blue));
}

void ivf::LightManager::setShininess(float shininess)
{
    ShaderManager::instance()->currentProgram()->uniformFloat(m_shininessId, shininess);
}

void ivf::LightManager::setAlpha(float alpha)
{
    ShaderManager::instance()->currentProgram()->uniformFloat(m_alphaId, alpha);
}

void ivf::LightManager::setUseShadows(bool flag)
{
    m_useShadows = flag;
    ShaderManager::instance()->currentProgram()->uniformBool("useShadows", flag);
}

bool ivf::LightManager::useShadows() const
{
    return m_useShadows;
}

void ivf::LightManager::setAutoCalcBBox(bool flag)
{
    m_autoCalcBBox = flag;
}

bool ivf::LightManager::autoCalcBBox() const
{
    return m_autoCalcBBox;
}

void ivf::LightManager::setDebugShadow(int value)
{
    m_debugShadow = value;
    ShaderManager::instance()->currentProgram()->uniformInt("debugShadow", value);
}

int ivf::LightManager::debugShadow() const
{
    return m_debugShadow;
}

void ivf::LightManager::setSceneBoundingBox(BoundingBox &bbox)
{
    m_sceneBBox = bbox;
}

void ivf::LightManager::setSceneBoundingBox(glm::vec3 min, glm::vec3 max)
{
    m_sceneBBox.setMin(min);
    m_sceneBBox.setMax(max);
}

BoundingBox ivf::LightManager::sceneBoundingBox() const
{
    return m_sceneBBox;
}

BoundingBox &ivf::LightManager::sceneBoundingBox()
{
    return m_sceneBBox;
}

void LightManager::saveState()
{
    m_savedState = m_useLighting;
}

void LightManager::restoreState()
{
    m_useLighting = m_savedState;
    this->setUseLighting(m_useLighting);
}

void LightManager::refreshForProgram()
{
    auto prog = ShaderManager::instance()->currentProgram();
    m_useLightingId = prog->uniformLoc("useLighting");
    m_useVertexColorsId = prog->uniformLoc("useVertexColors");
    m_pointLightCountId = prog->uniformLoc("pointLightCount");
    m_directionalLightCountId = prog->uniformLoc("dirLightCount");
    m_spotLightCountId = prog->uniformLoc("spotLightCount");
    m_diffuseColorId = prog->uniformLoc("material.diffuseColor");
    m_specularColorId = prog->uniformLoc("material.specularColor");
    m_ambientColorId = prog->uniformLoc("material.ambientColor");
    m_shininessId = prog->uniformLoc("material.shininess");
    m_alphaId = prog->uniformLoc("material.alpha");
}
