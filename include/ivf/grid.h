#ifndef __ivf2_grid__
#define __ivf2_grid__

#include <ivf/meshnode.h>

namespace ivf {
    
    class Grid : public MeshNode {
    private:
        GLint m_tickX;
        GLint m_tickY;
        GLint m_tickZ;
        GLfloat m_tickSpacingX;
        GLfloat m_tickSpacingY;
        GLfloat m_tickSpacingZ;
    public:
        Grid();
        virtual ~Grid();

        static std::shared_ptr<Grid> create();

        void setTicks(GLint nx, GLint ny, GLint nz);
        GLint tickCountX();
        GLint tickCountY();
        GLint tickCountZ();
        
        void setSpacing(GLfloat dx, GLfloat dy, GLfloat dz);
        GLfloat tickSpacingX();
        GLfloat tickSpacingY();
        GLfloat tickSpacingZ();
        
    protected:
        virtual void doSetup();
		virtual void doPreDraw();
		virtual void doPostDraw();
    };

    typedef std::shared_ptr<Grid> GridPtr;
};

#endif
