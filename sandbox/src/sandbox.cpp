#include "sandbox.h"

#include "core/log.h"
#include "vulkan_api/render_context.h"

Sandbox::Sandbox(engine::Platform *platform)
    : engine::Application(platform)
{
}

void Game::OnAttach()
{
    ENG_TRACE("Hello, world!");
}

bool Sandbox::Prepare()
{
    GetLayerStack().PushLayer(m_Game.get());
    return true;
}
