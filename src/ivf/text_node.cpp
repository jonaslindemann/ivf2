#include <ivf/text_node.h>

#include <ivf/config.h>
#include <ivf/font_manager.h>
#include <ivf/shader_manager.h>
#include <ivf/render_context.h>

#include <ivf/utils.h>
#include <ivf/logger.h>

#include <iostream>
#include <utility>

using namespace std;
using namespace ivf;

TextNode::TextNode()
    : m_text(""), m_textRendering(true), m_useFixedTextColor(false), m_textColor(1.0f, 1.0f, 0.0f), m_maxPixels(400),
      m_scale(1.0), m_textAlignX(TextAlignX::LEFT), m_textAlignY(TextAlignY::BOTTOM), m_textWidth(-1.0),
      m_textHeight(-1.0), m_VAO(0), m_vertexVBO(0), m_texVBO(0)
{
    m_textRenderingId = ShaderManager::instance()->currentProgram()->uniformLoc("textRendering");
    m_useFixedTextColorId = ShaderManager::instance()->currentProgram()->uniformLoc("useFixedTextColor");
    m_textColorId = ShaderManager::instance()->currentProgram()->uniformLoc("textColor");
    m_useTextureId = ShaderManager::instance()->currentProgram()->uniformLoc("useTexture");

    this->setUseMaterial(true);
    this->setUseTexture(false);

    m_vertexAttrId = ShaderManager::instance()->currentProgram()->attribId("aPos");
    m_texAttrId = ShaderManager::instance()->currentProgram()->attribId("aTex");
    m_normalAttrId = ShaderManager::instance()->currentProgram()->attribId("aNormal");
    m_colorAttrId = ShaderManager::instance()->currentProgram()->attribId("aColor");

    this->updateCharMap();
    this->prepareBuffers();
    this->setName("TextNode");
}

std::shared_ptr<TextNode> ivf::TextNode::create()
{
    return std::make_shared<TextNode>();
}

void ivf::TextNode::setText(const std::string &text)
{
    m_text = text;
    updateTextSize();
}

std::string ivf::TextNode::text() const
{
    return m_text;
}

void ivf::TextNode::setSize(const float size)
{
    m_scale = size;
}

float ivf::TextNode::size()
{
    return m_scale;
}

void ivf::TextNode::setResolution(int pixels)
{
    if (pixels <= 0 || pixels == m_maxPixels)
        return;
    m_maxPixels = pixels;
    updateCharMap();
    updateTextSize();
}

int ivf::TextNode::resolution() const
{
    return m_maxPixels;
}

void ivf::TextNode::setColor(const glm::vec3 &color)
{
    m_textColor = color;
    m_useFixedTextColor = true;
}

glm::vec3 ivf::TextNode::color() const
{
    return m_textColor;
}

void ivf::TextNode::setUseColor(bool flag)
{
    m_useFixedTextColor = flag;
}

bool ivf::TextNode::useColor() const
{
    return m_useFixedTextColor;
}

const CharacterInfo &ivf::TextNode::glyph(char c) const
{
    static const CharacterInfo none{};
    if (m_glyphs == nullptr)
        return none;
    auto it = m_glyphs->find(c);
    return it != m_glyphs->end() ? it->second : none;
}

void ivf::TextNode::setAlignX(const TextAlignX align)
{
    m_textAlignX = align;
}

void ivf::TextNode::setAlignY(const TextAlignY align)
{
    m_textAlignY = align;
}

TextAlignX ivf::TextNode::alignX()
{
    return m_textAlignX;
}

TextAlignY ivf::TextNode::alignY()
{
    return m_textAlignY;
}

void ivf::TextNode::updateTextSize()
{
    std::string::const_iterator c;

    m_textWidth = 0.0;
    m_textHeight = 0.0;

    for (c = m_text.begin(); c != m_text.end(); c++)
    {
        const CharacterInfo &ch = glyph(*c);
        m_textWidth += static_cast<float>(ch.glyphAdvance >> 6);
        if (ch.glyphSize.y > m_textHeight)
            m_textHeight = static_cast<float>(ch.glyphSize.y);
    }
}

float ivf::TextNode::textWidth()
{
    updateTextSize();
    return m_textWidth;
}

float ivf::TextNode::textHeight()
{
    updateTextSize();
    return m_textHeight;
}

void ivf::TextNode::updateCharMap()
{
    m_textWidth = 0.0;
    m_textHeight = 0.0;

#if IVF2_HAS_FREETYPE
    auto *fonts = FontManager::instance();
    if (fonts->currentFace() == nullptr)
    {
        logWarning("No font loaded - call FontManager::loadFace() before creating text", "TextNode");
        m_glyphs = nullptr;
        return;
    }
    m_glyphs = fonts->glyphs(m_maxPixels);
#else
    logWarning("ivf2 was built without FreeType (IVF2_WITH_FREETYPE=OFF) - text is not rendered", "TextNode");
#endif
}

void ivf::TextNode::prepareBuffers()
{
    // configure VAO/VBO for texture quads
    // -----------------------------------

    float texCoords[6][2] = {{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f},

                             {0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}};

    float colors[6][4] = {{1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f},

                          {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}};

    float normals[6][3] = {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},

                           {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}};

    GLuint indices[6] = {0, 1, 2, 3, 4, 5};

    glGenVertexArrays(1, &m_VAO);

    glGenBuffers(1, &m_vertexVBO);
    glGenBuffers(1, &m_texVBO);
    glGenBuffers(1, &m_colorVBO);
    glGenBuffers(1, &m_normalVBO);
    glGenBuffers(1, &m_indexVBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 6, indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 3, 0, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(m_vertexAttrId, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(m_vertexAttrId);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_texVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 2, texCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(m_texAttrId);
    glVertexAttribPointer(m_texAttrId, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_colorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, colors, GL_STATIC_DRAW);
    glVertexAttribPointer(m_colorAttrId, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(m_colorAttrId);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 3, normals, GL_STATIC_DRAW);
    glEnableVertexAttribArray(m_normalAttrId);
    glVertexAttribPointer(m_normalAttrId, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ivf::TextNode::doDraw()
{

    ShaderManager::instance()->currentProgram()->uniformBool(m_textRenderingId, m_textRendering);
    ShaderManager::instance()->currentProgram()->uniformBool(m_useFixedTextColorId, m_useFixedTextColor);
    ShaderManager::instance()->currentProgram()->uniformVec3(m_textColorId, m_textColor);
    ShaderManager::instance()->currentProgram()->uniformBool(m_useTextureId, true);

    GL_ERR(glActiveTexture(GL_TEXTURE0));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // iterate through all characters
    std::string::const_iterator c;
    float x = 0.0f;
    float scale = m_scale / (float)m_maxPixels;
    float dy = 0.0;

    if (m_textAlignX == TextAlignX::LEFT)
        x = 0.0;

    if (m_textAlignX == TextAlignX::RIGHT)
        x = -m_textWidth * scale;

    if (m_textAlignX == TextAlignX::CENTER)
        x = -m_textWidth * scale / 2.0f;

    if (m_textAlignY == TextAlignY::BOTTOM)
        dy = 0.0f;

    if (m_textAlignY == TextAlignY::MIDDLE)
        dy = m_textHeight * scale * 0.5f;

    if (m_textAlignY == TextAlignY::TOP)
        dy = m_textHeight * scale;

    for (c = m_text.begin(); c != m_text.end(); c++)
    {
        const CharacterInfo &ch = glyph(*c);

        float xpos= x + ch.glyphBearing.x * scale;
        float ypos = -(ch.glyphSize.y - ch.glyphBearing.y) * scale - dy;

        float w = ch.glyphSize.x * scale;
        float h = ch.glyphSize.y * scale;

        float vertices[6][3] = {
            {xpos, ypos + h, 0.0f}, {xpos, ypos, 0.0f},     {xpos + w, ypos, 0.0f},

            {xpos, ypos + h, 0.0f}, {xpos + w, ypos, 0.0f}, {xpos + w, ypos + h, 0.0f},
        };

        // render glyph texture over quad

        GL_ERR(glBindTexture(GL_TEXTURE_2D, ch.textureID));

        // update content of VBO memory

        GL_ERR(glBindVertexArray(m_VAO));

        GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO));
        GL_ERR(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                            GL_DYNAMIC_DRAW)); // be sure to use glBufferSubData and not glBufferData
        GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, 0));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBO);
        GL_ERR(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
        if (auto *context = RenderContext::current())
            context->countDrawCall();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        x += (ch.glyphAdvance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th
                                             // pixels by 64 to get amount of pixels))

        GL_ERR(glBindTexture(GL_TEXTURE_2D, 0));
        GL_ERR(glBindVertexArray(0));
    }

    ShaderManager::instance()->currentProgram()->uniformBool(m_textRenderingId, false);
    ShaderManager::instance()->currentProgram()->uniformBool(m_useTextureId, false);
    glDisable(GL_BLEND);
}

void ivf::TextNode::setupProperties()
{
    TransformNode::setupProperties();
    this->addProperty("Text", &m_text, "Geometry");
    this->addProperty("Size", &m_scale, "Geometry");
}

void ivf::TextNode::onPropertyChanged(const std::string &propertyName)
{
    if (propertyName == "Text")
    {
        this->updateTextSize();
    }
    else if (propertyName == "Size")
    {
        this->updateTextSize();
    }
    TransformNode::onPropertyChanged(propertyName);
}
