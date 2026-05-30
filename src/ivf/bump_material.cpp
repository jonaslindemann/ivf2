#include <ivf/bump_material.h>

#include <ivf/light_manager.h>
#include <ivf/selection_manager.h>
#include <ivf/shader_manager.h>
#include <ivf/transform_manager.h>

using namespace ivf;

std::shared_ptr<BumpMaterial> BumpMaterial::create()
{
    return std::make_shared<BumpMaterial>();
}

void BumpMaterial::apply()
{
    ShaderManager::instance()->setCurrentProgram("bump");

    refreshManagers();

    // Upload base Phong uniforms (diffuse/specular/ambient/shininess/alpha) via LightManager.
    Material::apply();

    // Forward the material's useTexture flag to the bump shader directly, because
    // TextureManager's cached uniform IDs belong to the basic program and are stale here.
    ShaderManager::instance()->currentProgram()->uniformBool("useTexture", useTexture());

    bindMaps();

    LightManager::instance()->apply();
}

void BumpMaterial::unapply()
{
    unbindMaps();

    ShaderManager::instance()->setCurrentProgram("basic");
    refreshManagers();
}

void BumpMaterial::refreshManagers()
{
    TransformManager::instance()->refreshForProgram();
    LightManager::instance()->refreshForProgram();
    SelectionManager::instance()->refreshForProgram();
}

void BumpMaterial::bindMaps()
{
    auto prog = ShaderManager::instance()->currentProgram();

    if (m_normalMap)
    {
        m_normalMap->setTexUnit(1);
        m_normalMap->bind();
        prog->uniformInt("normalMap", 1);
        prog->uniformBool("useNormalMap", true);
    }
    else
    {
        prog->uniformBool("useNormalMap", false);
    }
}

void BumpMaterial::unbindMaps()
{
    if (m_normalMap)
        m_normalMap->unbind();
}
