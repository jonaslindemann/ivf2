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
     m_scale(1.0)
{
    m_textRenderingId = ShaderManager::instance()->currentProgram()->uniformLoc("textRendering");
    m_useFixedTextColorId = ShaderManager::instance()->currentProgram()->uniformLoc("useFixedTextColor");
    m_textColorId = ShaderManager::instance()->currentProgram()->uniformLoc("textColor");

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
}

std::string ivf::TextNode::text()
{
	return m_text;
}

void ivf::TextNode::updateCharMap()
{
    if (m_charMap.size() != 0)
        for (auto& it : m_charMap)
            glDeleteTextures(1, &it.second.textureID);

    m_charMap.clear();

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 2, 0, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(m_texAttrId);
    glVertexAttribPointer(m_texAttrId, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_colorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, 0, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(m_colorAttrId, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(m_colorAttrId);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 3, 0, GL_DYNAMIC_DRAW);
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // iterate through all characters
    std::string::const_iterator c;
    float x = 0.0f;
    float scale = m_scale/(float)m_maxPixels;

    for (c = m_text.begin(); c != m_text.end(); c++)
    {
        CharacterInfo ch = m_charMap[*c];

        float xpos = x + ch.glyphBearing.x * scale;
        float ypos = - (ch.glyphSize.y - ch.glyphBearing.y) * scale;

        float w = ch.glyphSize.x * scale;
        float h = ch.glyphSize.y * scale;

        GLuint indices[6] = { 0, 1, 2, 3, 4, 5 };
        // update VBO for each character
        /*
        float vertices[6][5] = {
            { xpos,     ypos + h,   0.0f, 0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 0.0f, 1.0f },
            { xpos + w, ypos,       0.0f, 1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f, 0.0f },
            { xpos + w, ypos,       0.0f, 1.0f, 1.0f },
            { xpos + w, ypos + h,   0.0f, 1.0f, 0.0f }
        };
        */
        float vertices[6][3] = {
            { xpos,     ypos + h,   0.0f },
            { xpos,     ypos,       0.0f },
            { xpos + w, ypos,       0.0f },

            { xpos,     ypos + h,   0.0f },
            { xpos + w, ypos,       0.0f },
            { xpos + w, ypos + h,   0.0f },
        };

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

        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        // update content of VBO memory
        GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO));
        GL_ERR(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW)); // be sure to use glBufferSubData and not glBufferData

        GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, m_texVBO));
        GL_ERR(glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_DYNAMIC_DRAW)); // be sure to use glBufferSubData and not glBufferData

        GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, m_colorVBO));
        GL_ERR(glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_DYNAMIC_DRAW)); // be sure to use glBufferSubData and not glBufferData

        GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, m_normalVBO));
        GL_ERR(glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_DYNAMIC_DRAW)); // be sure to use glBufferSubData and not glBufferData

        GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, 0));

        //glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, indices);
        GL_ERR(glDrawArrays(GL_TRIANGLES, 0, 6));

        // render quad
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.glyphAdvance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    ShaderManager::instance()->currentProgram()->uniformBool("textRendering", false);
    glDisable(GL_BLEND);

}
