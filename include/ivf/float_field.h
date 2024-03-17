#pragma once

#include <ivf/field.h>

#include <glad/glad.h>

namespace ivf {

class FloatField : public Field {
private:
    std::unique_ptr<GLfloat[]> m_data;

public:
    FloatField(GLuint rows, GLuint cols);
    FloatField(const FloatField &other);
    FloatField &operator=(const FloatField &other);
    FloatField(FloatField &&other);
    FloatField &operator=(FloatField &&other);

    static std::shared_ptr<FloatField> create(GLuint rows, GLuint cols);

    GLfloat at(GLuint rows, GLuint cols);
    GLfloat at(GLuint pos);

    void set(GLuint rows, GLuint cols, GLfloat value);
    GLfloat get(GLuint rows, GLuint cols);

    virtual size_t memSize();

    virtual void zero();

    virtual void *data();
    virtual GLenum dataType();

    virtual void print();
};

typedef std::shared_ptr<FloatField> FloatFieldPtr;

}; // namespace ivf
