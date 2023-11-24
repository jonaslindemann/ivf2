#pragma once

#include <string>
#include <map>

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace ivf {
  
    class FontManager {
    private:
        FontManager();
        virtual ~FontManager();
        static FontManager* m_instance;

        FT_Library m_freetype;
        std::map<std::string, FT_Face> m_faces;
        FT_Face m_currentFace;

    public:
        static FontManager* instance()
        {
            if (!m_instance)
            {
                if (!m_instance)
                    m_instance = new FontManager();
            }
            return m_instance;
        }

        static FontManager* create()
        {
            if (!m_instance)
            {
                if (!m_instance)
                    m_instance = new FontManager();
            }
            return m_instance;
        }

        static void drop()
        {
            delete m_instance;
            m_instance = 0;
        }

        void loadFace(const std::string filename, const std::string name);
        FT_Face currentFace();
        FT_Face face(const std::string name);
    };

    typedef FontManager* FontManagerPtr;
    
}
