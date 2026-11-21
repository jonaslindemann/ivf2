#include <ivf/shader_watcher.h>
#include <ivf/logger.h>
#include <ivf/render_context.h>

#include <ivf/vertex_shader.h>
#include <ivf/fragment_shader.h>

#include <iostream>
#include <algorithm>

namespace ivf {

// The manager lives in the current RenderContext (see ivf/render_context.h).
ShaderWatcher *ShaderWatcher::instance()
{
    return RenderContext::currentOrFallback()->shaderWatcher();
}



void ShaderWatcher::watch(ProgramPtr program, std::string vertPath, std::string fragPath)
{
    // Don't double-register the same program
    for (auto& e : m_entries)
        if (e.program == program) return;

    Entry entry;
    entry.program  = program;
    entry.vertPath = std::move(vertPath);
    entry.fragPath = std::move(fragPath);

    // Snapshot current modification times
    std::error_code ec;
    entry.lastVertTime = std::filesystem::last_write_time(entry.vertPath, ec);
    entry.lastFragTime = std::filesystem::last_write_time(entry.fragPath, ec);

    m_entries.push_back(std::move(entry));
}

void ShaderWatcher::unwatch(ProgramPtr program)
{
    m_entries.erase(
        std::remove_if(m_entries.begin(), m_entries.end(),
                       [&](const Entry& e){ return e.program == program; }),
        m_entries.end());
}

void ShaderWatcher::clear()
{
    m_entries.clear();
}

void ShaderWatcher::update(float dt, float checkInterval)
{
    m_accum += dt;
    if (m_accum < checkInterval) return;
    m_accum = 0.0f;

    std::error_code ec;
    for (auto& e : m_entries) {
        auto vt = std::filesystem::last_write_time(e.vertPath, ec);
        auto ft = std::filesystem::last_write_time(e.fragPath, ec);
        if (ec) continue;

        if (vt != e.lastVertTime || ft != e.lastFragTime) {
            if (tryReload(e)) {
                e.lastVertTime = vt;
                e.lastFragTime = ft;
                ivf::logInfo(ivf::logStr("Reloaded: ", e.vertPath, " / ", e.fragPath), "ShaderWatcher");
            }
        }
    }
}

bool ShaderWatcher::tryReload(Entry& e)
{
    auto vert = std::make_shared<VertexShader>(e.vertPath);
    if (!vert->read()) {
        ivf::logError(ivf::logStr("Cannot read: ", e.vertPath), "ShaderWatcher");
        return false;
    }
    if (!vert->compile()) {
        ivf::logError(ivf::logStr("Vertex compile error: ", e.vertPath), "ShaderWatcher");
        return false;
    }

    auto frag = std::make_shared<FragmentShader>(e.fragPath);
    if (!frag->read()) {
        ivf::logError(ivf::logStr("Cannot read: ", e.fragPath), "ShaderWatcher");
        return false;
    }
    if (!frag->compile()) {
        ivf::logError(ivf::logStr("Fragment compile error: ", e.fragPath), "ShaderWatcher");
        return false;
    }

    e.program->clear();
    e.program->addShader(vert);
    e.program->addShader(frag);

    if (!e.program->link()) {
        ivf::logError(ivf::logStr("Link error for: ", e.vertPath, " / ", e.fragPath), "ShaderWatcher");
        return false;
    }

    return true;
}

} // namespace ivf
