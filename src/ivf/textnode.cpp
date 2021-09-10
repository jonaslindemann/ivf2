#include <ivf/textnode.h>

#include <ivf/fontmanager.h>
#include <ivf/shadermanager.h>

#include <ivf/utils.h>

#include <iostream>
#include <utility>

using namespace std;
using namespace ivf;


TextNode::TextNode()
    :m_text(""),
    m_VAO(0),
    m_vertexVBO(0),
    m_texVBO(0),
    m_textColor(1.0f, 1.0f, 0.0f),
    m_textRendering(true),
    m_useFixedTextColor(false),
    m_maxPixels(400),
    m_scale(1.0),
    m_textAlignX(TextAlignX::LEFT),
    m_textAlignY(TextAlignY::BOTTOM),
    m_textHeight(-1.0),
    m_textWidth(-1.0)
{
    m_textRenderingId = ShaderManager::instance()->currentProgram()->uniformLoc("textRendering");
    m_useFixedTextColorId = ShaderManager::instance()->currentProgram()->uniformLoc("useFixedTextColor");
    m_textColorId = ShaderManager::instance()->currentProgram()->uniformLoc("textColor");
    m_useTextureId = ShaderManager::instance()->currentProgram()->uniformLoc("useTexture");

	this->setUseMaterial(true);
	this->setUseTexture(true);

    m_vertexAttrId = ShaderManager::instance()->currentProgram()->attribId("aPos");
    m_texAttrId = ShaderManager::instance()->currentProgram()->attribId("aTex");
    m_normalAttrId = ShaderManager::instance()->currentProgram()->attribId("aNormal");
    m_colorAttrId = ShaderManager::instance()->currentProgram()->attribId("aColor");

    this->updateCharMap();
    this->prepareBuffers();
}

std::shared_ptr<TextNode> ivf::TextNode::create()
{
	return std::make_shared<TextNode>();
}

void ivf::TextNode::setText(const std::string text)
{
	m_text = text;
    updateTextSize();
}

std::string ivf::TextNode::text()
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
        CharacterInfo ch = m_charMap[*c];
        m_textWidth += (ch.glyphAdvance >> 6);
        if (ch.glyphSize.y > m_textHeight)
            m_textHeight = ch.glyphSize.y;
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
    if (m_charMap.size() != 0)
        for (auto& it : m_charMap)
            glDeleteTextures(1, &it.second.textureID);

    m_charMap.clear();

    m_textWidth = 0.0;
    m_textHeight = 0.0;

    FT_Face face = FontManager::instance()->currentFace();

    FT_Set_Pixel_Sizes(face, 0, m_maxPixels);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "TextNode: Failed to load Glyph" << std::endl;
            continue;
        }

        unsigned int texture;

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        CharacterInfo character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        m_charMap.insert(std::pair<char, CharacterInfo>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ivf::TextNode::prepareBuffers()
{
    // configure VAO/VBO for texture quads
    // -----------------------------------

    float texCoords[6][2] = {
        { 0.0f, 0.0f },
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },

        { 0.0f, 0.0f },
        { 1.0f, 1.0f },
        { 1.0f, 0.0f }
    };

    float colors[6][4] = {
        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f },

        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f }
    };

    float normals[6][3] = {
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },

        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f }
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_vertexVBO);
    glGenBuffers(1, &m_texVBO);
    glGenBuffers(1, &m_colorVBO);
    glGenBuffers(1, &m_normalVBO);

    glBindVertexArray(m_VAO);
    
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
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_VAO);

    ShaderManager::instance()->currentProgram()->uniformBool(m_textRenderingId, m_textRendering);
    ShaderManager::instance()->currentProgram()->uniformBool(m_useFixedTextColorId, m_useFixedTextColor);
    ShaderManager::instance()->currentProgram()->uniformVec3(m_textColorId, m_textColor);
    ShaderManager::instance()->currentProgram()->uniformBool(m_useTextureId, true);


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
        CharacterInfo ch = m_charMap[*c];

        float xpos = x + ch.glyphBearing.x * scale;
        float ypos = - (ch.glyphSize.y - ch.glyphBearing.y) * scale - dy;

        float w = ch.glyphSize.x * scale;
        float h = ch.glyphSize.y * scale;

        GLuint indices[6] = { 0, 1, 2, 3, 4, 5 };

        float vertices[6][3] = {
            { xpos,     ypos + h,   0.0f },
            { xpos,     ypos,       0.0f },
            { xpos + w, ypos,       0.0f },

            { xpos,     ypos + h,   0.0f },
            { xpos + w, ypos,       0.0f },
            { xpos + w, ypos + h,   0.0f },
        };

        // render glyph texture over quad

        glBindTexture(GL_TEXTURE_2D, ch.textureID);

        // update content of VBO memory

        GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO));
        GL_ERR(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW)); // be sure to use glBufferSubData and not glBufferData
        GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, 0));

        GL_ERR(glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, indices));

        x += (ch.glyphAdvance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    ShaderManager::instance()->currentProgram()->uniformBool(m_textRenderingId, false);
    ShaderManager::instance()->currentProgram()->uniformBool(m_useTextureId, false);
    glDisable(GL_BLEND);
}
