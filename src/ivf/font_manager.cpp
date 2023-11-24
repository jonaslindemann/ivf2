#include <ivf/font_manager.h>

#include <iostream>


using namespace ivf;
using namespace std;


FontManager* FontManager::m_instance = 0;

FontManager::FontManager()
    :m_currentFace(nullptr),
     m_freetype(nullptr)
{
    cout << "FontManager: Created." << endl;

    // Initialise FreeType

    if (FT_Init_FreeType(&m_freetype))
    {
        std::cout << "FontManager: Could not init FreeType Library" << std::endl;
        m_freetype = nullptr;
    }
}

ivf::FontManager::~FontManager()
{
    if (m_freetype != nullptr)
        FT_Done_FreeType(m_freetype);
}

void ivf::FontManager::loadFace(const std::string filename, const std::string name)
{
    FT_Face face;
    if (FT_New_Face(m_freetype, filename.c_str(), 0, &face)) 
    {
        std::cout << "FontManager: Failed to load font" << std::endl;
    }
    else
    {
        auto it = m_faces.find(name);

        if (it == m_faces.end())
        {
            m_faces[name] = face;
            m_currentFace = face;
        }
        else
        {
            FT_Done_Face(it->second);
            it->second = face;
            m_currentFace = face;
        }

        std::cout << "FontManager: Loaded font " << filename << " as " << name << std::endl;
    }
}

FT_Face ivf::FontManager::currentFace()
{
    return m_currentFace;
}

FT_Face ivf::FontManager::face(const std::string name)
{
    if (m_faces.find(name) != m_faces.end())
        return m_faces[name];
    else
        return nullptr;
}


