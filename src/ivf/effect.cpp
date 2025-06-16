#include <ivf/effect.h>

using namespace ivf;

Effect::Effect() : GLBase(), m_time(0.0f), m_width(0), m_height(0)
{}

Effect::~Effect()
{}

std::shared_ptr<Effect> ivf::Effect::create()
{
    return std::make_shared<Effect>();
}

void ivf::Effect::setTime(float time)
{
    m_time = time;
}

void ivf::Effect::setResolution(int width, int height)
{
    m_width = width;
    m_height = height;
}

float ivf::Effect::time()
{
    return m_time;
}

int ivf::Effect::width()
{
    return m_width;
}

int ivf::Effect::height()
{
    return m_height;
}

void ivf::Effect::load()
{
    doLoad();
}

void ivf::Effect::use()
{
    if (m_program != nullptr)
    {
        if (!m_program->enabled())
            return; // Already in use

        m_program->use();
        m_program->uniformFloat("time", m_time);
        m_program->uniformInt("width", m_width);
        m_program->uniformInt("height", m_height);
        doUpdateParams();
    }
}

ProgramPtr ivf::Effect::program()
{
    return m_program;
}
