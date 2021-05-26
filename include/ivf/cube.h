#ifndef __ivf2_cube__
#define __ivf2_cube__

#include <ivf/meshnode.h>

namespace ivf {
    
    class Cube : public MeshNode {
    private:
        GLfloat m_size;
    public:
		Cube(GLfloat size = 1.0);
		virtual ~Cube();

        static std::shared_ptr<Cube> create();
        
        void setSize(GLfloat size);
        GLfloat size();
        
    protected:
        virtual void doSetup();
    };

    typedef std::shared_ptr<Cube> CubePtr;
};

#endif
