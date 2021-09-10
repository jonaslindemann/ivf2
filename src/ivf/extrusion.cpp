#include <ivf/extrusion.h>

#include <ivf/lightmanager.h>

using namespace ivf;

ivf::Extrusion::Extrusion()
{
}

std::shared_ptr<Extrusion> ivf::Extrusion::create()
{
	return std::make_shared<Extrusion>();
}

void ivf::Extrusion::addPathPoint(glm::vec3 p)
{
	m_path.push_back(p);
}

void ivf::Extrusion::clearPath()
{
	m_path.clear();
}

void ivf::Extrusion::addSectionPoint(glm::vec2 p)
{
	glm::vec3 pp(p.x, p.y, 0.0);
	m_section.push_back(pp);
}

void ivf::Extrusion::clearSection()
{
	m_section.clear();
}

void ivf::Extrusion::doSetup()
{
    this->clear();
    this->newMesh(m_path.size(), 0, GL_LINE_STRIP);

    mesh()->begin(GL_LINE_STRIP);

    for (auto &p : m_path)
    {
        mesh()->vertex3f(p);
        mesh()->color3f(1.0, 1.0, 1.0);
    }

    mesh()->end();

    int i = 0;

    std::vector<glm::vec3> fvec;

    for (auto& p : m_path)
    {
        glm::vec3 forward;

        if (i < m_path.size() - 1)
            forward = glm::normalize(m_path[i + 1] - m_path[i]);
        else
            forward = glm::normalize(m_path[m_path.size() - 1] - m_path[m_path.size() - 2]);

        fvec.push_back(forward);

        i++;
    }

    std::vector<glm::vec3> sfvec;
    sfvec.resize(fvec.size());

    sfvec[0] = fvec[0];
    sfvec[sfvec.size() - 1] = fvec[fvec.size() - 1];

    for (i = 1; i < m_path.size() - 1; i++)
        sfvec[i] = glm::normalize(fvec[i - 1] + fvec[i]);

    i = 0;

    for (auto& p : m_path)
    {
        this->newMesh(4, 0, GL_LINE_LOOP);

        lastMesh()->begin(GL_LINE_LOOP);
        for (auto& s : m_section)
        {
            glm::vec3 forward = sfvec[i];
            glm::vec3 left;
            glm::vec3 up(0.0f, 1.0f, 0.0f);

            left = glm::normalize(glm::cross(forward, up));
            up = glm::normalize(glm::cross(forward, left));

            glm::vec3 sp = p + left*s.x + up*s.y;
            lastMesh()->vertex3f(sp);
            lastMesh()->color3f(1.0, 1.0, 1.0);
        }
        lastMesh()->end();
        i++;
    }

}

void ivf::Extrusion::doPreDraw()
{
    LightManager::instance()->saveState();
    LightManager::instance()->disableLighting();
}

void ivf::Extrusion::doPostDraw()
{
    LightManager::instance()->restoreState();
}
