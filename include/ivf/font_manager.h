#pragma once

#include <string>
#include <map>

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace ivf {

/**
 * @class FontManager
 * @brief Singleton class for managing FreeType font faces.
 *
 * The FontManager class provides a global interface for loading, storing, and accessing
 * FreeType font faces. It ensures that font resources are managed efficiently and
 * provides methods to load new faces, retrieve the current face, and access faces by name.
 * This class is implemented as a singleton.
 */
class FontManager {
private:
    FontManager();                  ///< Private constructor for singleton pattern.
    virtual ~FontManager();         ///< Destructor.
    static FontManager *m_instance; ///< Singleton instance pointer.

    FT_Library m_freetype;                  ///< FreeType library handle.
    std::map<std::string, FT_Face> m_faces; ///< Map of font face names to FT_Face handles.
    FT_Face m_currentFace;                  ///< Currently active font face.

public:
    /**
     * @brief Get the singleton instance of the FontManager.
     * @return FontManager* Pointer to the singleton instance.
     */
    static FontManager *instance()
    {
        if (!m_instance)
        {
            if (!m_instance)
                m_instance = new FontManager();
        }
        return m_instance;
    }

    /**
     * @brief Create the singleton instance of the FontManager (if not already created).
     * @return FontManager* Pointer to the singleton instance.
     */
    static FontManager *create()
    {
        if (!m_instance)
        {
            if (!m_instance)
                m_instance = new FontManager();
        }
        return m_instance;
    }

    /**
     * @brief Destroy the singleton instance and release all resources.
     */
    static void drop()
    {
        delete m_instance;
        m_instance = 0;
    }

    /**
     * @brief Load a font face from file and store it with a given name.
     * @param filename Path to the font file.
     * @param name Name to associate with the loaded font face.
     */
    void loadFace(const std::string filename, const std::string name);

    /**
     * @brief Get the currently active font face.
     * @return FT_Face Handle to the current font face.
     */
    FT_Face currentFace();

    /**
     * @brief Get a font face by name.
     * @param name Name of the font face.
     * @return FT_Face Handle to the requested font face.
     */
    FT_Face face(const std::string name);
};

/**
 * @typedef FontManagerPtr
 * @brief Pointer type for FontManager singleton.
 */
typedef FontManager *FontManagerPtr;

} // namespace ivf
