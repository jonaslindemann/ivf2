#include <ivf/font_manager.h>
#include <ivf/render_context.h>

#include <ivf/config.h>
#include <ivf/logger.h>

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
    for (auto &[name, face] : m_faces)
        FT_Done_Face(face);

    if (m_freetype != nullptr)
        FT_Done_FreeType(m_freetype);
}

void FontManager::loadFace(const std::string filename, const std::string name)
{
    if (m_freetype == nullptr)
    {
        logError("Could not load font " + filename + ": FreeType is not initialised", "FontManager");
        return;
    }

    FT_Face face;
    if (FT_New_Face(m_freetype, filename.c_str(), 0, &face))
    {
        logError("Could not load font: " + filename, "FontManager");
        return;
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
}

#else // !IVF2_HAS_FREETYPE

FontManager::FontManager()
{
    logInfo("Creating FontManager (ivf2 built without FreeType - text rendering disabled)", "FontManager");
}

FontManager::~FontManager() = default;

void FontManager::loadFace(const std::string filename, const std::string name)
{
    logError("Cannot load font " + filename + ": ivf2 was built without FreeType (IVF2_WITH_FREETYPE=OFF)",
             "FontManager");
}

#endif // IVF2_HAS_FREETYPE

FontFace FontManager::currentFace()
{
    return m_currentFace;
}

FontFace FontManager::face(const std::string name)
{
    auto it = m_faces.find(name);
    return it != m_faces.end() ? it->second : nullptr;
}
