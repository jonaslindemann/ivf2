#include <ivf/node.h>

#include <ivf/selection_manager.h>
#include <ivf/shader_manager.h>

using namespace ivf;

std::shared_ptr<Node> Node::parent() const
{
    return m_parent.lock();
}
void Node::setParent(std::shared_ptr<Node> parent)
{
    m_parent = parent;
}

void Node::draw()
{
    doPreDraw();
    if (m_visible)
        doDraw();
    doPostDraw();
}

void ivf::Node::drawSelection()
{
    if (m_visible)
        doDrawSelection();
}

void ivf::Node::setMaterial(std::shared_ptr<Material> material)
{
    m_material = material;
}

std::shared_ptr<Material> ivf::Node::material()
{
    return m_material;
}

void ivf::Node::setUseMaterial(bool flag)
{
    m_useMaterial = flag;
}

void Node::setTexture(std::shared_ptr<Texture> texture)
{
    m_texture = texture;
    
    // Backward compatibility: also set as first texture in multitexture array
    if (m_textures.empty()) {
        m_textures.push_back(texture);
    } else {
        m_textures[0] = texture;
    }
    
    if (m_material != nullptr)
        m_material->setUseTexture(true);

    m_useTexture = true;
}

std::shared_ptr<Texture> ivf::Node::texture()
{
    return m_texture;
}

void ivf::Node::addTexture(std::shared_ptr<Texture> texture)
{
    if (m_textures.size() >= 8) {
        // Maximum of 8 textures supported - silently ignore
        return;
    }
    m_textures.push_back(texture);
    
    // Keep the primary texture pointer in sync with first texture for backward compatibility
    if (m_textures.size() == 1) {
        m_texture = texture;
    }
    
    m_useMultiTexturing = m_textures.size() > 1;
    m_useTexture = true;
}

void ivf::Node::setTexture(size_t index, std::shared_ptr<Texture> texture)
{
    if (index >= 8) {
        // Texture index exceeds maximum - silently ignore
        return;
    }
    
    if (index >= m_textures.size()) {
        m_textures.resize(index + 1);
    }
    m_textures[index] = texture;
    
    // Update primary texture if index 0
    if (index == 0) {
        m_texture = texture;
    }
    
    m_useMultiTexturing = m_textures.size() > 1;
    m_useTexture = true;
}

void ivf::Node::removeTexture(size_t index)
{
    if (index < m_textures.size()) {
        m_textures.erase(m_textures.begin() + index);
        m_useMultiTexturing = m_textures.size() > 1;
        
        // Update primary texture if we removed index 0
        if (index == 0 && !m_textures.empty()) {
            m_texture = m_textures[0];
        } else if (m_textures.empty()) {
            m_texture = nullptr;
            m_useTexture = false;
        }
    }
}

void ivf::Node::clearTextures()
{
    m_textures.clear();
    m_texture = nullptr;
    m_useMultiTexturing = false;
    m_useTexture = false;
}

std::shared_ptr<Texture> ivf::Node::getTexture(size_t index)
{
    if (index < m_textures.size()) {
        return m_textures[index];
    }
    return nullptr;
}

size_t ivf::Node::textureCount() const
{
    return m_textures.size();
}

const std::vector<std::shared_ptr<Texture>>& ivf::Node::textures() const
{
    return m_textures;
}

void ivf::Node::setUseMultiTexturing(bool flag)
{
    m_useMultiTexturing = flag;
}

bool ivf::Node::useMultiTexturing() const
{
    return m_useMultiTexturing;
}

void ivf::Node::bindTextures()
{
    auto program = ShaderManager::instance()->currentProgram();
    
    if (m_useMultiTexturing && m_textures.size() > 1) {
        // Multitexture path
        program->uniformBool("useMultiTexturing", true);
        program->uniformInt("activeTextureCount", static_cast<int>(m_textures.size()));
        
        for (size_t i = 0; i < m_textures.size() && i < 8; ++i) {
            if (m_textures[i]) {
                m_textures[i]->setTexUnit(static_cast<GLint>(i));
                m_textures[i]->bind();
                
                // Set per-texture blend mode and factor using array uniforms
                std::string indexStr = std::to_string(i);
                program->uniformInt("textureBlendModes[" + indexStr + "]", 
                                   static_cast<int>(m_textures[i]->blendMode()));
                program->uniformFloat("textureBlendFactors[" + indexStr + "]", 
                                     m_textures[i]->blendFactor());
            }
        }
        
        // Bind texture array samplers
        for (size_t i = 0; i < m_textures.size() && i < 8; ++i) {
            std::string samplerName = "textures[" + std::to_string(i) + "]";
            program->uniformInt(samplerName, static_cast<int>(i));
        }
    } else {
        // Single texture path (backward compatible)
        program->uniformBool("useMultiTexturing", false);
        program->uniformInt("activeTextureCount", 1);
        
        if (m_texture) {
            m_texture->bind();
            program->uniformInt("texture0", 0);
        }
    }
}

bool ivf::Node::useMaterial()
{
    return m_useMaterial;
}

void ivf::Node::setUseTexture(bool flag)
{
    m_useTexture = flag;
}

bool ivf::Node::useTexture()
{
    return m_useTexture;
}

void ivf::Node::setVisible(bool flag)
{
    m_visible = flag;
}

bool ivf::Node::visible() const
{
    return m_visible;
}

void ivf::Node::setObjectId(uint32_t objectId)
{
    m_objectId = objectId;
}

uint32_t ivf::Node::objectId() const
{
    return m_objectId;
}

void ivf::Node::setName(const std::string &name)
{
    m_name = name;
}

std::string ivf::Node::name() const
{
    return m_name;
}

uint32_t ivf::Node::enumerateIds(uint32_t startId)
{
    return this->doEnumerateIds(startId);
}

void ivf::Node::accept(NodeVisitor *visitor)
{
    visitor->visit(this);
}

void Node::doPreDraw()
{
    SelectionManager::instance()->setObjectId(m_objectId);

    if ((m_material != nullptr) && (m_useMaterial))
        m_material->apply();
    
    // Use bindTextures instead of single texture bind
    if (m_useTexture)
        bindTextures();
}

void ivf::Node::doDraw()
{}

void ivf::Node::doPostDraw()
{
    // Unbind textures
    if (m_useTexture) {
        if (m_useMultiTexturing && m_textures.size() > 1) {
            for (auto& tex : m_textures) {
                if (tex) {
                    tex->unbind();
                }
            }
        } else if (m_texture) {
            m_texture->unbind();
        }
    }
}

void ivf::Node::doDrawSelection()
{}

void ivf::Node::doSetup()
{}

uint32_t ivf::Node::doEnumerateIds(uint32_t startId)
{
    m_objectId = startId;
    return startId + 1;
}

void ivf::Node::setupProperties()
{
    addProperty("Name", &m_name, "Node");
    addProperty("Visible", &m_visible, "Node");
    addProperty("Use Material", &m_useMaterial, "Node");
    addProperty("Use Texture", &m_useTexture, "Node");
    addProperty("Use MultiTexturing", &m_useMultiTexturing, "Node");
}
