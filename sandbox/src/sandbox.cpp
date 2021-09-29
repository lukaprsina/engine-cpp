#include "sandbox.h"

#include "core/log.h"
#include "vulkan_api/render_context.h"
#include "vulkan_api/device.h"
#include "renderer/shader.h"
#include "scene/gltf_loader.h"
#include "vulkan_api/rendering/render_pipeline.h"
#include "scene/scene.h"
#include "window/window.h"
#include "vulkan_api/subpasses/forward_subpass.h"
#include "vulkan_api/rendering/render_pipeline.h"

Sandbox::Sandbox(engine::Platform *platform)
    : engine::Application(platform)
{
}

Game::Game(engine::Application *application)
    : Layer(application)
{
}

void Game::OnAttach()
{
    SetWindow(GetApp()->GetPlatform().CreatePlatformWindow());
    SetScene(GetApp()->LoadScene("scenes/sponza/Sponza01.gltf"));

    engine::Scene *scene = GetScene();
    engine::Window *window = GetWindow();

    std::vector<VkPresentModeKHR> present_mode_priority({VK_PRESENT_MODE_IMMEDIATE_KHR,
                                                         VK_PRESENT_MODE_FIFO_KHR,
                                                         VK_PRESENT_MODE_MAILBOX_KHR});

    std::vector<VkSurfaceFormatKHR> surface_format_priority({{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                             {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                             {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                             {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}});

    window->CreateSurface(GetApp()->GetInstance(), GetApp()->GetDevice().GetGPU());
    window->CreateRenderContext(GetApp()->GetDevice(), present_mode_priority, surface_format_priority);
    window->GetRenderContext().Prepare();
    scene->AddFreeCamera(window->GetRenderContext().GetSurfaceExtent(), window);
    scene->CreateRenderPipeline(GetApp()->GetDevice());
}

bool Sandbox::Prepare()
{
    GetLayerStack().PushLayer("Game", std::make_shared<Game>(this));
    GetLayerStack().PushLayer("PopUp", std::make_shared<Game>(this));
    Application::Prepare();
    return true;
}
