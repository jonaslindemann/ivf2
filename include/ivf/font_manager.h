#pragma once

#include <string>
#include <map>

#include <glad/glad.h>

#include <glm/glm.hpp>

// FreeType is a private dependency of ivf. Its handle types are pointers to these
// structs (FT_Library == FT_LibraryRec_*, FT_Face == FT_FaceRec_*), so they can be
// named here without including FreeType headers.
struct FT_LibraryRec_;
struct FT_FaceRec_;

namespace ivf {

/** @brief Handle to a loaded font face (FreeType's FT_Face). */
using FontFace = FT_FaceRec_ *;

/**
 * @class FontManager
 * @brief Singleton class for managing FreeType font faces.
 *
 * The FontManager class provides a global interface for loading, storing, and accessing
 * FreeType font faces. It ensures that font resources are managed efficiently and
 * provides methods to load new faces, retrieve the current face, and access faces by name.
 * This class is implemented as a singleton.
 *
 * Without FreeType (IVF2_WITH_FREETYPE=OFF) loading fails with a logged error and
 * all faces are nullptr; TextNode then renders nothing.
 */
class FontManager {
private:
    FontManager();                  ///< Private constructor for singleton pattern.
    virtual ~FontManager();         ///< Destructor.
    static FontManager *m_instance; ///< Singleton instance pointer.

    FT_LibraryRec_ *m_freetype{nullptr};       ///< FreeType library handle.
    std::map<std::string, FontFace> m_faces;   ///< Map of font face names to face handles.
    FontFace m_currentFace{nullptr};           ///< Currently active font face.

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
     * @return Handle to the current font face, or nullptr if none is loaded.
     */
    FontFace currentFace();

    /**
     * @brief Get a font face by name.
     * @param name Name of the font face.
     * @return Handle to the requested font face, or nullptr if not found.
     */
    FontFace face(const std::string name);
};

/**
 * @typedef FontManagerPtr
 * @brief Pointer type for FontManager singleton.
 */
typedef FontManager *FontManagerPtr;

} // namespace ivf
