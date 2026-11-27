#include <ivf/font_manager.h>
#include <ivf/render_context.h>

#include <ivf/config.h>
#include <ivf/logger.h>
#include <filesystem>

#if IVF2_HAS_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

using namespace ivf;

// The manager lives in the current RenderContext (see ivf/render_context.h).
FontManager *FontManager::instance()
{
    return RenderContext::currentOrFallback()->fonts();
}

FontManager *FontManager::create()
{
    return instance();
}

void FontManager::drop()
{
    // Managers are released together with their RenderContext.
}


#if IVF2_HAS_FREETYPE

FontManager::FontManager()
{
    logInfo("Creating FontManager singleton instance", "FontManager");

    // Initialise FreeType
    if (FT_Init_FreeType(&m_freetype))
    {
        logError("Could not init FreeType Library", "FontManager");
        m_freetype = nullptr;
    }
}

FontManager::~FontManager()
{
    // The context is current while its managers are destroyed
    for (auto &[key, glyphs] : m_glyphs)
        for (auto &[c, info] : glyphs)
            glDeleteTextures(1, &info.textureID);

    for (auto &[name, face] : m_faces)
        FT_Done_Face(face);

    if (m_freetype != nullptr)
        FT_Done_FreeType(m_freetype);
}

Result<void> FontManager::loadFace(const std::string &filename, const std::string &name)
{
    if (m_freetype == nullptr)
        return makeError(ErrorCode::NotSupported, "Could not load font '" + filename + "': FreeType is not initialised",
                         "FontManager");

    FT_Face face;
    if (FT_New_Face(m_freetype, filename.c_str(), 0, &face))
    {
        if (!std::filesystem::exists(std::filesystem::path(filename)))
            return makeError(ErrorCode::FileNotFound, "Could not load font '" + filename + "': file not found",
                             "FontManager");
        return makeError(ErrorCode::UnsupportedFormat, "Could not load font '" + filename + "': not a usable font file",
                         "FontManager");
    }

    auto it = m_faces.find(name);
    if (it != m_faces.end())
    {
        FT_Done_Face(it->second);
        it->second = face;
    }
    else
        m_faces[name] = face;

    m_currentFace = face;
    logInfo("Loaded font: " + filename + " as " + name, "FontManager");
    return {};
}

const GlyphMap *FontManager::glyphs(int pixelSize)
{
    if (m_currentFace == nullptr || pixelSize <= 0)
        return nullptr;

    const auto key = std::make_pair(m_currentFace, pixelSize);
    if (auto it = m_glyphs.find(key); it != m_glyphs.end())
        return &it->second;

    FT_Face face = m_currentFace;
    FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(pixelSize));

    GLint alignmentWas = 4;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignmentWas);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glActiveTexture(GL_TEXTURE0);

    GlyphMap &glyphs = m_glyphs[key];
    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            logErrorfc("FontManager", "Failed to load glyph {}", static_cast<int>(c));
            continue;
        }

        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, // sized: OpenGL ES rejects plain GL_RED
                     static_cast<GLsizei>(face->glyph->bitmap.width),
                     static_cast<GLsizei>(face->glyph->bitmap.rows), 0, GL_RED, GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        CharacterInfo info;
        info.textureID = texture;
        info.glyphSize = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
        info.glyphBearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
        info.glyphAdvance = static_cast<unsigned int>(face->glyph->advance.x);
        glyphs[static_cast<char>(c)] = info;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignmentWas);
    return &glyphs;
}

#else // !IVF2_HAS_FREETYPE

FontManager::FontManager()
{
    logInfo("Creating FontManager (ivf2 built without FreeType - text rendering disabled)", "FontManager");
}

FontManager::~FontManager() = default;

const GlyphMap *FontManager::glyphs(int /*pixelSize*/)
{
    return nullptr;
}

Result<void> FontManager::loadFace(const std::string &filename, const std::string &/*name*/)
{
    return makeError(ErrorCode::NotSupported,
                     "Could not load font '" + filename + "': ivf2 was built without FreeType (IVF2_WITH_FREETYPE=OFF)",
                     "FontManager");
}

#endif // IVF2_HAS_FREETYPE

FontFace FontManager::currentFace() const
{
    return m_currentFace;
}

FontFace FontManager::face(const std::string &name)
{
    auto it = m_faces.find(name);
    return it != m_faces.end() ? it->second : nullptr;
}
