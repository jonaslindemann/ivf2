#pragma once

#include <ivf/base.h>

#include <cstdlib>

#include <glad/glad.h>

namespace ivf {

class Field : public Base {
protected:
    GLuint m_size[2] = {0, 0};

public:
    Field() = default;
    virtual ~Field() = default;

    GLuint rows() const;
    GLuint cols() const;
    GLuint size() const;

    virtual void zero();
    virtual size_t memSize();

    virtual void *data();
    virtual GLenum dataType();

    virtual void print();
};

}; // namespace ivf
