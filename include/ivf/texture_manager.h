#pragma once

#include <ivf/glbase.h>
#include <ivf/shader.h>

#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace ivf {

/*
#define BLEND_NORMAL 0
#define BLEND_MULTIPLY 1
#define BLEND_ADD 2
#define BLEND_SCREEN 3
#define BLEND_OVERLAY 4
#define BLEND_DECAL 5
*/

enum TextureBlendMode {
    Normal = 0,
    Multiply = 1,
    Add = 2,
    Screen = 3,
    Overlay = 4,
    Decal = 5
};

class TextureManager {
private:
    TextureBlendMode m_textureBlendMode;
    TextureBlendMode m_savedTextureBlendMode;
    GLint m_textureBlendModeId;

    GLfloat m_blendFactor;
    GLfloat m_savedBlendFactor;
    GLint m_blendFactorId;

    GLboolean m_useTexture;
    GLboolean m_savedUseTexture;
    GLint m_useTextureId;

    TextureManager();
    static TextureManager *m_instance;

public:
    static TextureManager *instance()
    {
        if (!m_instance)
            m_instance = new TextureManager();

        return m_instance;
    }

    static TextureManager *create()
    {
        return instance();
    }

    static void drop()
    {
        delete m_instance;
        m_instance = 0;
    }

    void setTextureBlendMode(TextureBlendMode blendMode);
    void setBlendFactor(float blendFactor);
    void setUseTexture(bool useTexture);

    TextureBlendMode textureBlendMode();
    GLfloat blendFactor();
    GLboolean useTexture();

    void apply();

    void saveState();
    void restoreState();
};

typedef TextureManager *TextureManagerPtr;
}; // namespace ivf
