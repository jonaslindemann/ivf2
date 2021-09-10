#pragma once

#include <ivf/transformnode.h>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <map>
#include <string>

namespace ivf {

	struct CharacterInfo {
		GLuint		 textureID; // ID handle of the glyph texture
		glm::ivec2   glyphSize;      // Size of glyph
		glm::ivec2   glyphBearing;   // Offset from baseline to left/top of glyph
		unsigned int glyphAdvance;   // Horizontal offset to advance to next glyph
	};

	enum class TextAlignX {
		LEFT,
		CENTER,
		RIGHT
	};

	enum class TextAlignY {
		BOTTOM,
		MIDDLE,
		TOP
	};

	class TextNode : public TransformNode {
	private:
		std::string m_text;
		std::map<GLchar, CharacterInfo> m_charMap;

		

		GLuint m_vertexAttrId;
		GLuint m_texAttrId;
		GLuint m_normalAttrId;
		GLuint m_colorAttrId;

		GLuint m_textRenderingId;
		GLuint m_useFixedTextColorId;
		GLuint m_textColorId;
		GLuint m_useTextureId;

		bool m_textRendering;
		bool m_useFixedTextColor;
		glm::vec3 m_textColor;

		int m_maxPixels;
		float m_scale;

		TextAlignX m_textAlignX;
		TextAlignY m_textAlignY;

		float m_textWidth;
		float m_textHeight;

		GLuint m_VAO;
		GLuint m_vertexVBO;
		GLuint m_texVBO;
		GLuint m_normalVBO;
		GLuint m_colorVBO;
		GLuint m_indexVBO;

		void updateCharMap();
		void prepareBuffers();
		void updateTextSize();
	public:
		TextNode();

		static std::shared_ptr<TextNode> create();

		void setText(const std::string text);
		std::string text();

		void setSize(const float size);
		float size();

		void setAlignX(const TextAlignX align);
		void setAlignY(const TextAlignY align);

		TextAlignX alignX();
		TextAlignY alignY();

		float textWidth();
		float textHeight();

	protected:
		virtual void doDraw();
	};

	typedef std::shared_ptr<TextNode> TextNodePtr;
}


