#ifndef __ivf2_axis__
#define __ivf2_axis__

#include <ivf/meshnode.h>

namespace ivf {
    
    class Axis : public MeshNode {
    private:
        GLfloat m_size;
    public:
        Axis(GLfloat size = 1.0);
        virtual ~Axis();

        static std::shared_ptr<Axis> create(GLfloat size = 1.0);
        
        void setSize(GLfloat size);
        GLfloat size();
        
    protected:
        virtual void doSetup();
        virtual void doPreDraw();
        virtual void doPostDraw();
    };

    typedef std::shared_ptr<Axis> AxisPtr;

};

#endif
