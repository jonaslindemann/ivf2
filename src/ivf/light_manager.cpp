#include <ivf/light_manager.h>
#include <ivf/render_context.h>

#include <ivf/shader_manager.h>
#include <ivf/extent_visitor.h>
#include <ivf/renderer.h>
#include <ivf/transform_manager.h>
#include <ivf/shadow_shaders.h>
#include <ivf/gl_debug.h>
#include <ivf/logger.h>
#include <ivf/shadow_math.h>

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
    m_shadowSlots.clear();
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
    m_shadowSlots.clear();
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

    // Shadow maps and the lights they belong to
    for (std::size_t s = 0; s < m_shadowSlots.size(); ++s)
        block.shadows[s] = m_shadowSlots[s].data;
    auto shadowIndex = [this](const Light *light) {
        for (std::size_t s = 0; s < m_shadowSlots.size(); ++s)
            if (m_shadowSlots[s].light == light)
                return static_cast<std::int32_t>(s);
        return std::int32_t{-1};
    };

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
        data.shadowIndex = shadowIndex(&light);
        data.shadowStrength = light.shadowStrength();
    }

    const int dirs= std::min(static_cast<int>(m_dirLights.size()), MaxDirLights);
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
        data.shadowIndex = shadowIndex(&light);
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
    if (m_renderingShadows)
        return; // materials drawn into a shadow map: the depth program needs no lights

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

    // Shadow maps: samplers cannot be in a uniform block. shadows[i] uses shadowMaps[i] on
    // texture unit ShadowMapUnit + i (Program::link() sets the samplers).
    for (std::size_t s = 0; s < m_shadowSlots.size(); ++s)
    {
        const auto &map = m_shadowSlots[s].light->shadowMap();
        if (!map)
            continue;
        glActiveTexture(GL_TEXTURE0 + ShadowMapUnit + static_cast<GLuint>(s));
        glBindTexture(GL_TEXTURE_2D, map->depthTexture());
    }
    glActiveTexture(GL_TEXTURE0);

    // Matrix of the first shadow tile (used by the debug views)
    if (!m_shadowSlots.empty())
        program->uniformMatrix4("lightSpaceMatrix", m_shadowSlots.front().data.matrices[0]);
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

void LightManager::updateShadowSlots(const glm::mat4 &view, const glm::mat4 &projection)
{
    m_shadowSlots.clear();
    if (!m_useShadows)
        return;

    const glm::vec3 eye = glm::vec3(glm::inverse(view)[3]);
    const glm::vec2 nearFar = projectionNearFar(projection);
    const bool sceneValid = m_sceneBBox.isValid();

    // Distance from a point to the farthest corner of the scene
    auto farthest = [this](const glm::vec3 &from) {
        float d = 0.0f;
        for (const auto &corner : m_sceneBBox.corners())
            d = std::max(d, glm::length(corner - from));
        return d;
    };

    auto addSlot = [this](Light &light) -> ShadowSlot * {
        if (static_cast<int>(m_shadowSlots.size()) >= MaxShadowMaps)
        {
            if (!m_shadowLimitWarned)
                logWarning(logStr("More than ", MaxShadowMaps, " lights cast shadows; the others are ignored"),
                           "LightManager");
            m_shadowLimitWarned = true;
            return nullptr;
        }
        ShadowSlot &slot = m_shadowSlots.emplace_back();
        slot.light = &light;
        slot.data.info.z = static_cast<int>(light.shadowFilter());
        slot.data.info.w = light.shadowFilterRadius();
        slot.data.params = glm::vec4(light.shadowBias(), light.shadowNormalBias(), light.shadowSoftness(), 0.0f);
        return &slot;
    };

    const int dirs = std::min(static_cast<int>(m_dirLights.size()), MaxDirLights);
    for (int i = 0; i < dirs; ++i)
    {
        DirectionalLight &light = *m_dirLights[static_cast<std::size_t>(i)];
        if (!light.enabled() || !light.castsShadows())
            continue;
        ShadowSlot *slot = addSlot(light);
        if (slot == nullptr)
            break;

        const int cascades = light.shadowCascades();
        slot->tiles = cascades;
        slot->columns = cascades == 1 ? 1 : 2;
        const int rows = cascades <= 2 ? 1 : 2;
        slot->tileSize = std::max(16, std::min(static_cast<int>(light.shadowMapWidth()) / slot->columns,
                                               static_cast<int>(light.shadowMapHeight()) / rows));
        slot->width = slot->tileSize * slot->columns;
        slot->height = slot->tileSize * rows;
        slot->data.info.x = static_cast<int>(ShadowType::Directional);
        slot->data.info.y = cascades;
        slot->data.atlas = glm::vec4(float(slot->tileSize), float(slot->columns), float(rows), 0.0f);

        if (cascades == 1)
        {
            // One map around the whole scene
            slot->data.matrices[0] = light.calculateLightSpaceMatrix(m_sceneBBox);
            continue;
        }

        // Cascades: slices of the camera's view range, each in its own tile
        float distance = light.shadowDistance();
        if (distance <= 0.0f)
            distance = sceneValid ? farthest(eye) : nearFar.y;
        distance = std::clamp(distance, nearFar.x * 1.01f, nearFar.y);
        const std::vector<float> splits = cascadeSplits(nearFar.x, distance, cascades, light.cascadeSplitLambda());
        float from = nearFar.x;
        for (int k = 0; k < cascades; ++k)
        {
            const float to = splits[static_cast<std::size_t>(k)];
            slot->data.matrices[k] = fitCascade(frustumSliceCorners(view, projection, from, to), light.direction(),
                                                m_sceneBBox, slot->tileSize);
            slot->data.splits[k] = to;
            from = to;
        }
    }

    const int points = std::min(static_cast<int>(m_pointLights.size()), MaxPointLights);
    for (int i = 0; i < points; ++i)
    {
        PointLight &light = *m_pointLights[static_cast<std::size_t>(i)];
        if (!light.enabled() || !light.castsShadows())
            continue;
        ShadowSlot *slot = addSlot(light);
        if (slot == nullptr)
            break;

        // Six cube faces in a 3 x 2 atlas, each a bit wider than 90 degrees so filtering
        // near a face's edge still finds depth values
        const int face = std::max(16, static_cast<int>(light.shadowMapWidth()) / 2);
        const float tanHalfFov = 1.0f + 16.0f / float(face);
        float range = light.shadowRange();
        if (range <= 0.0f)
            range = sceneValid ? farthest(light.position()) * 1.01f : 100.0f;
        range = std::max(range, 0.1f);
        const float nearDist = std::max(range * 0.002f, 0.01f);

        slot->tiles = 6;
        slot->columns = 3;
        slot->tileSize = face;
        slot->width = face * 3;
        slot->height = face * 2;
        slot->data.info.x = static_cast<int>(ShadowType::Point);
        slot->data.info.y = 6;
        slot->data.splits = glm::vec4(nearDist, range, 0.0f, 0.0f);
        slot->data.lightPos = glm::vec4(light.position(), 1.0f);
        slot->data.atlas = glm::vec4(float(face), 3.0f, 2.0f, tanHalfFov);
        const auto matrices = pointShadowMatrices(light.position(), nearDist, range, tanHalfFov);
        for (int k = 0; k < 6; ++k)
            slot->data.matrices[k] = matrices[static_cast<std::size_t>(k)];
    }
}

void LightManager::renderShadowMaps(RenderContext &context, const RenderQueue &queue)
{
    m_shadowStats = Renderer::Stats{};
    if (!m_useShadows)
    {
        if (!m_shadowSlots.empty())
        {
            m_shadowSlots.clear();
            this->apply();
        }
        return;
    }

    // Fit the shadow maps to the scene. Keeps the previous box if no item has bounds.
    if (m_autoCalcBBox)
    {
        const BoundingBox bounds = queue.bounds();
        if (bounds.isValid())
            m_sceneBBox = bounds;
    }

    auto *xfm = context.transforms();
    updateShadowSlots(xfm->viewMatrix(), xfm->projectionMatrix());

    // Save current OpenGL state

    GLboolean depthTest;
    glGetBooleanv(GL_DEPTH_TEST, &depthTest);
    GLint polygonMode[2];
    glGetIntegerv(GL_POLYGON_MODE, polygonMode);
    GLboolean cullFace;
    glGetBooleanv(GL_CULL_FACE, &cullFace);
    GLint cullFaceMode = GL_BACK;
    glGetIntegerv(GL_CULL_FACE_MODE, &cullFaceMode);
    GLint previousViewport[4];
    glGetIntegerv(GL_VIEWPORT, previousViewport);

    // The scene may be rendering into an offscreen framebuffer (post-processing),
    // so restore that binding rather than the default framebuffer afterwards.
    GLint previousFramebuffer = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);

    // Set shadow rendering state

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glCullFace(GL_FRONT); // Can help with shadow acne (when culling is enabled)

    // Every node is drawn with the current (stock) program in its shadow mode, whatever
    // program its material would switch to
    ShaderManager *shaders = context.shaders();
    ProgramPtr shader = shaders->currentProgram();
    shader->use();
    shader->uniformBool("shadowPass", true);
    shaders->lockProgram();
    m_renderingShadows = true;

    // Only opaque nodes cast shadows (not the sky, overlays or blended objects)
    const Renderer::SkipFn skip = [](const DrawItem &item) {
        return item.layer != RenderLayer::Opaque || !item.castShadows;
    };
    Renderer renderer(context, RenderPass::Shadow);

    for (auto &slot : m_shadowSlots)
    {
        Light &light = *slot.light;
        if (!light.shadowMap())
            light.setShadowMap(ShadowMap::create(slot.width, slot.height));
        else
            light.shadowMap()->resize(slot.width, slot.height);
        auto map = light.shadowMap();
        map->setLightSpaceMatrix(slot.data.matrices[0]);

        map->bind();
        glClear(GL_DEPTH_BUFFER_BIT);

        for (int k = 0; k < slot.tiles; ++k)
        {
            const glm::mat4 &matrix = slot.data.matrices[k];
            glViewport((k % slot.columns) * slot.tileSize, (k / slot.columns) * slot.tileSize, slot.tileSize,
                       slot.tileSize);
            shader->uniformMatrix4("lightSpaceMatrix", matrix);
            renderer.execute(queue, Frustum(matrix), skip); // only what this tile sees
            m_shadowStats += renderer.stats();
        }

        map->unbind();
    }

    m_renderingShadows = false;
    shaders->unlockProgram();

    // Restore framebuffer and viewport

    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(previousFramebuffer));
    glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);

    // Restore regular shader

    shaders->setCurrentProgram(shader);
    shader->uniformBool("shadowPass", false);

    // Restore OpenGL state
    if (depthTest)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, polygonMode[0]);
    glCullFace(static_cast<GLenum>(cullFaceMode));

    if (cullFace)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);

    // The matrices and maps of this frame for the main pass
    this->apply();
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
    if (!flag)
        m_shadowSlots.clear(); // the next apply() turns them off in the shaders
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
