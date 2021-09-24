#include "sandbox.h"

#include "core/log.h"
#include "vulkan_api/render_context.h"

void Game::OnAttach()
{
    m_Window = GetApp()->GetPlatform().CreatePlatformWindow();
    SetWindow(m_Window);
    ENG_TRACE("Hello, world!");
}

Game::Game(engine::Application *application)
    : Layer(application)
{
}

Sandbox::Sandbox(engine::Platform *platform)
    : engine::Application(platform)
{
}

bool Sandbox::Prepare()
{
    m_Game = std::make_unique<Game>(this);
    GetLayerStack().PushLayer(m_Game.get());

    Application::Prepare();
    return true;
}
