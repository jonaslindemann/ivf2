#ifndef __ivf2_glbase__
#define __ivf2_glbase__

#include <ivf/base.h>

#include <glad/glad.h>

namespace ivf {
    
    class GLBase : public Base {
    public:
        GLBase();
        virtual ~GLBase();
    };

    typedef std::shared_ptr<GLBase> GLBasePtr;
};

#endif
