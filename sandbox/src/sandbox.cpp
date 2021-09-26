#include "sandbox.h"

#include "core/log.h"
#include "vulkan_api/render_context.h"
#include "renderer/shader.h"
#include "scene/gltf_loader.h"
#include "vulkan_api/rendering/render_pipeline.h"
#include "scene/scene.h"
#include "window/window.h"
#include "vulkan_api/subpasses/forward_subpass.h"
#include "vulkan_api/rendering/render_pipeline.h"

Game::Game(engine::Application *application)
    : Layer(application)
{
}

Sandbox::Sandbox(engine::Platform *platform)
    : engine::Application(platform)
{
}

void Game::OnAttach()
{
    SetWindow(GetApp()->GetPlatform().CreatePlatformWindow());
    SetScene(GetApp()->LoadScene("scenes/sponza/Sponza01.gltf"));

    engine::Scene *scene = GetScene();
    engine::Window *window = GetWindow();
    scene->AddFreeCamera(window->GetRenderContext().GetSurfaceExtent(), window);
}

bool Sandbox::Prepare()
{
    m_Game = std::make_unique<Game>(this);
    GetLayerStack().PushLayer(m_Game.get());

    Application::Prepare();
    return true;
}
