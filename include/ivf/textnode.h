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

		bool m_textRendering;
		bool m_useFixedTextColor;
		glm::vec3 m_textColor;

		int m_maxPixels;
		float m_scale;

		GLuint m_VAO;
		GLuint m_vertexVBO;
		GLuint m_texVBO;
		GLuint m_normalVBO;
		GLuint m_colorVBO;
		GLuint m_indexVBO;

		void updateCharMap();
		void prepareBuffers();
	public:
		TextNode();

		static std::shared_ptr<TextNode> create();

		void setText(const std::string text);
		std::string text();

	protected:
		virtual void doDraw();
	};

	typedef std::shared_ptr<TextNode> TextNodePtr;
}


