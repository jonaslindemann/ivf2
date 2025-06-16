#pragma once

#include <ivf/transform_node.h>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <map>
#include <string>

namespace ivf {

/**
 * @struct CharacterInfo
 * @brief Stores glyph texture and metrics for a single character.
 */
struct CharacterInfo {
    GLuint textureID;          ///< ID handle of the glyph texture
    glm::ivec2 glyphSize;      ///< Size of glyph
    glm::ivec2 glyphBearing;   ///< Offset from baseline to left/top of glyph
    unsigned int glyphAdvance; ///< Horizontal offset to advance to next glyph
};

/**
 * @enum TextAlignX
 * @brief Horizontal text alignment options.
 */
enum class TextAlignX {
    LEFT,   ///< Align text to the left.
    CENTER, ///< Center text horizontally.
    RIGHT   ///< Align text to the right.
};

/**
 * @enum TextAlignY
 * @brief Vertical text alignment options.
 */
enum class TextAlignY {
    BOTTOM, ///< Align text to the bottom.
    MIDDLE, ///< Center text vertically.
    TOP     ///< Align text to the top.
};

/**
 * @class TextNode
 * @brief TransformNode for rendering 2D/3D text with alignment and color options.
 *
 * The TextNode class manages and renders text using glyph textures, supporting
 * alignment, scaling, and color options. It provides methods to set the text string,
 * size, alignment, and color, and to query the rendered text's width and height.
 * Inherits from TransformNode for positioning and transformation in the scene.
 */
class TextNode : public TransformNode {
private:
    std::string m_text;                        ///< Text string to render.
    std::map<GLchar, CharacterInfo> m_charMap; ///< Glyph info for each character.

    GLuint m_vertexAttrId; ///< Vertex attribute location.
    GLuint m_texAttrId;    ///< Texture coordinate attribute location.
    GLuint m_normalAttrId; ///< Normal attribute location.
    GLuint m_colorAttrId;  ///< Color attribute location.

    GLuint m_textRenderingId;     ///< Shader uniform for text rendering.
    GLuint m_useFixedTextColorId; ///< Shader uniform for fixed text color.
    GLuint m_textColorId;         ///< Shader uniform for text color.
    GLuint m_useTextureId;        ///< Shader uniform for texture usage.

    bool m_textRendering;     ///< Whether text rendering is enabled.
    bool m_useFixedTextColor; ///< Whether to use a fixed text color.
    glm::vec3 m_textColor;    ///< Text color.

    int m_maxPixels; ///< Maximum pixel size for text rendering.
    float m_scale;   ///< Text scale factor.

    TextAlignX m_textAlignX; ///< Horizontal alignment.
    TextAlignY m_textAlignY; ///< Vertical alignment.

    float m_textWidth;  ///< Computed width of the rendered text.
    float m_textHeight; ///< Computed height of the rendered text.

    GLuint m_VAO;       ///< Vertex array object.
    GLuint m_vertexVBO; ///< Vertex buffer object for vertices.
    GLuint m_texVBO;    ///< Vertex buffer object for texture coordinates.
    GLuint m_normalVBO; ///< Vertex buffer object for normals.
    GLuint m_colorVBO;  ///< Vertex buffer object for colors.
    GLuint m_indexVBO;  ///< Index buffer object.

    /**
     * @brief Update the character map (glyph textures and metrics).
     */
    void updateCharMap();

    /**
     * @brief Prepare OpenGL buffers for text rendering.
     */
    void prepareBuffers();

    /**
     * @brief Update the computed text width and height.
     */
    void updateTextSize();

public:
    /**
     * @brief Constructor.
     */
    TextNode();

    /**
     * @brief Factory method to create a shared pointer to a TextNode instance.
     * @return std::shared_ptr<TextNode> New TextNode instance.
     */
    static std::shared_ptr<TextNode> create();

    /**
     * @brief Set the text string to render.
     * @param text New text string.
     */
    void setText(const std::string text);

    /**
     * @brief Get the current text string.
     * @return std::string Text string.
     */
    std::string text();

    /**
     * @brief Set the text size (scale).
     * @param size New text size.
     */
    void setSize(const float size);

    /**
     * @brief Get the current text size (scale).
     * @return float Text size.
     */
    float size();

    /**
     * @brief Set the horizontal text alignment.
     * @param align Alignment option.
     */
    void setAlignX(const TextAlignX align);

    /**
     * @brief Set the vertical text alignment.
     * @param align Alignment option.
     */
    void setAlignY(const TextAlignY align);

    /**
     * @brief Get the current horizontal text alignment.
     * @return TextAlignX Alignment option.
     */
    TextAlignX alignX();

    /**
     * @brief Get the current vertical text alignment.
     * @return TextAlignY Alignment option.
     */
    TextAlignY alignY();

    /**
     * @brief Get the computed width of the rendered text.
     * @return float Text width.
     */
    float textWidth();

    /**
     * @brief Get the computed height of the rendered text.
     * @return float Text height.
     */
    float textHeight();

protected:
    /**
     * @brief Internal draw method for rendering the text.
     */
    virtual void doDraw();
};

/**
 * @typedef TextNodePtr
 * @brief Shared pointer type for TextNode.
 */
typedef std::shared_ptr<TextNode> TextNodePtr;

} // namespace ivf
