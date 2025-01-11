#pragma once

#include <vector>

#include <ivf/gl.h>
#include <ivf/texture.h>

namespace ivf {

class PostProcessor : public GLBase {
private:
    int m_width;
    int m_height;
    float m_time;

    GLuint m_fboA, m_fboB;
    GLuint m_textureA, m_textureB;

    GLuint m_quadVAO;
    GLuint m_quadVBO;

    std::vector<ProgramPtr> m_fxPrograms;

public:
    PostProcessor(int width, int height);
    virtual ~PostProcessor();

    void addEffect(ProgramPtr fxProgram);
    void clearEffects();

    void initialize();
    void resize(int width, int height);
    void drop();

    static std::shared_ptr<PostProcessor> create(int width, int height);

    void initQuad();

    void setTime(float time);

    void apply(GLuint texture);

    GLuint id();
};

typedef std::shared_ptr<PostProcessor> PostProcessorPtr;

}; // namespace ivf
