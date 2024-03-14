#pragma once

#include <ivf/base.h>

#include <glad/glad.h>

namespace ivf {
    
    class GLBase : public Base {
    };

    typedef std::shared_ptr<GLBase> GLBasePtr;
};
