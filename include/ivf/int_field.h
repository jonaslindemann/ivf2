#pragma once

#include <ivf/field.h>

#include <glad/glad.h>

namespace ivf {

class IntField : public Field {
private:
    std::unique_ptr<GLuint[]> m_data;

public:
    IntField(GLuint rows, GLuint cols);
    IntField(const IntField &other);
    IntField &operator=(const IntField &other);
    IntField(IntField &&other);
    IntField &operator=(IntField &&other);

    static std::shared_ptr<IntField> create(GLuint rows, GLuint cols);

    GLuint at(GLuint row, GLuint col);
    GLuint at(GLuint pos);

    void set(GLuint row, GLuint col, GLuint value);

    virtual size_t memSize();

    virtual void zero();

    virtual GLenum dataType();

    virtual void *data();

    virtual void print();
};
typedef std::shared_ptr<IntField> IntFieldPtr;

}; // namespace ivf
