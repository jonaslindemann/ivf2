#ifndef __ivf2_light_manager__
#define __ivf2_light_manager__

#include <ivf/glbase.h>
#include <ivf/shader.h>

#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace ivf {

    class LightManager {
    private:
        bool m_useLighting;
        bool m_useVertexColors;
	    bool m_savedState;
	    LightManager();
	    static LightManager* m_instance;
    public:
	    static LightManager* instance()
        {
            if (!m_instance)
            {
                if (!m_instance)
				    m_instance = new LightManager();
            }
            return m_instance;
        }

        static LightManager* create()
        {
            if (!m_instance)
            {
                if (!m_instance)
                    m_instance = new LightManager();
            }
            return m_instance;
        }

        static void drop()
        {
            delete m_instance;
            m_instance = 0;
        }

	    void setUseLighting(bool flag);
        void setUseVertexColors(bool flag);
	    bool useLighting();
	    void enableLighting();
	    void disableLighting();

        void setupDefaultColors();

        void setDiffuseColor(glm::vec3 color);
        void setDiffuseColor(glm::vec4 color);
        void setDiffuseColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha = 1.0f);

        void setSpecularColor(glm::vec3 color);
        void setSpecularColor(glm::vec4 color);
        void setSpecularColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha = 1.0f);
    
        void setAmbientColor(glm::vec3 color);
        void setAmbientColor(glm::vec4 color);
        void setAmbientColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha = 1.0f);

        void saveState();
	    void restoreState();
   
    };
    
    typedef LightManager* LightManagerPtr;
}

#endif
