// RenderContext bookkeeping: current context per thread, manager forwarding and
// lifetime. Only GL-free managers are touched, so no OpenGL context is needed.

#include "test_main.h"

#include <ivf/mesh_manager.h>
#include <ivf/render_context.h>
#include <ivf/shader_manager.h>

#include <thread>

using ivf::RenderContext;

IVF_TEST(no_current_context_initially)
{
    IVF_CHECK(RenderContext::current() == nullptr);
}

IVF_TEST(create_without_loader_and_make_current)
{
    auto ctx = RenderContext::create(nullptr); // nullptr: don't load GL functions
    IVF_CHECK(ctx != nullptr);
    IVF_CHECK(!ctx->isCurrent());

    ctx->makeCurrent();
    IVF_CHECK(ctx->isCurrent());
    IVF_CHECK(RenderContext::current() == ctx.get());

    RenderContext::clearCurrent();
    IVF_CHECK(RenderContext::current() == nullptr);
}

IVF_TEST(singleton_accessors_forward_to_current_context)
{
    auto ctx = RenderContext::create(nullptr);
    ctx->makeCurrent();

    IVF_CHECK(ivf::ShaderManager::instance() == ctx->shaders());
    IVF_CHECK(ivf::ShaderManager::create() == ctx->shaders());
    IVF_CHECK(ivf::MeshManager::instance() == ctx->meshes());

    RenderContext::clearCurrent();
}

IVF_TEST(contexts_have_separate_managers)
{
    auto a = RenderContext::create(nullptr);
    auto b = RenderContext::create(nullptr);
    IVF_CHECK(a->meshes() != b->meshes());
    IVF_CHECK(a->shaders() != b->shaders());

    a->makeCurrent();
    IVF_CHECK(ivf::MeshManager::instance() == a->meshes());
    b->makeCurrent();
    IVF_CHECK(ivf::MeshManager::instance() == b->meshes());

    // State changes stay in their own context.
    a->meshes()->setDefaultMeshUsage(GL_DYNAMIC_DRAW);
    b->meshes()->setDefaultMeshUsage(GL_STATIC_DRAW);
    IVF_CHECK(a->meshes()->defaultMeshUsage() == GL_DYNAMIC_DRAW);
    IVF_CHECK(b->meshes()->defaultMeshUsage() == GL_STATIC_DRAW);

    RenderContext::clearCurrent();
}

IVF_TEST(destroying_current_context_clears_current)
{
    auto ctx = RenderContext::create(nullptr);
    ctx->makeCurrent();
    ctx.reset();
    IVF_CHECK(RenderContext::current() == nullptr);
}

IVF_TEST(current_context_is_per_thread)
{
    auto ctx = RenderContext::create(nullptr);
    ctx->makeCurrent();

    RenderContext *seenOnOtherThread = ctx.get();
    std::thread([&] { seenOnOtherThread = RenderContext::current(); }).join();

    IVF_CHECK(seenOnOtherThread == nullptr);
    IVF_CHECK(RenderContext::current() == ctx.get());
    RenderContext::clearCurrent();
}

IVF_TEST_MAIN()
