#include "sandbox.h"

#include "core/log.h"
#include "vulkan_api/render_context.h"
#include "vulkan_api/device.h"
#include "renderer/shader.h"
#include "scene/gltf_loader.h"
#include "vulkan_api/rendering/render_pipeline.h"
#include "scene/scene.h"
#include "window/glfw_window.h"
#include "vulkan_api/subpasses/forward_subpass.h"
#include "vulkan_api/rendering/render_pipeline.h"
#include "core/gui.h"

Sandbox::Sandbox(engine::Platform *platform)
    : engine::Application(platform)
{
}

Game::Game(engine::Application *application, engine::Window *window, const std::string &name)
    : Layer(application, name)
{
    SetWindow(window);
    SetScene(GetApp().GetScenes()[0].get());
}

void Game::OnAttach()
{
    engine::Window *window = GetWindow();

    std::vector<VkPresentModeKHR> present_mode_priority({VK_PRESENT_MODE_FIFO_KHR,
                                                         VK_PRESENT_MODE_IMMEDIATE_KHR,
                                                         VK_PRESENT_MODE_MAILBOX_KHR});

    std::vector<VkSurfaceFormatKHR> surface_format_priority({{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                             {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                             {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                             {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}});

    window->CreateSurface(GetApp().GetInstance(), GetApp().GetDevice().GetGPU());
    window->CreateRenderContext(GetApp().GetDevice(), present_mode_priority, surface_format_priority);
    window->GetRenderContext().Prepare();
    AddFreeCamera(window->GetRenderContext().GetSurfaceExtent(), window);
}

Simple::Simple(engine::Application *application, const std::string &name)
    : Layer(application, name)
{
}

void Simple::OnAttach()
{
    SetWindow(GetApp().GetPlatform().CreatePlatformWindow());
    SetScene(GetApp().GetScenes()[1].get());
    engine::Window *window = GetWindow();

    std::vector<VkPresentModeKHR> present_mode_priority({VK_PRESENT_MODE_FIFO_KHR,
                                                         VK_PRESENT_MODE_IMMEDIATE_KHR,
                                                         VK_PRESENT_MODE_MAILBOX_KHR});

    std::vector<VkSurfaceFormatKHR> surface_format_priority({{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                             {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                             {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                             {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}});

    window->CreateSurface(GetApp().GetInstance(), GetApp().GetDevice().GetGPU());
    window->CreateRenderContext(GetApp().GetDevice(), present_mode_priority, surface_format_priority);
    window->GetRenderContext().Prepare();
    AddFreeCamera(window->GetRenderContext().GetSurfaceExtent(), window);
}

bool Sandbox::Prepare()
{
    Application::Prepare();
    engine::Scene *s1 = LoadScene("scenes/sponza/Sponza01.gltf");
    engine::Scene *s2 = LoadScene("scenes/planet.gltf");
    s1->CreateRenderPipeline(GetDevice());
    s2->CreateRenderPipeline(GetDevice());

    engine::Window *main_window = GetPlatform().CreatePlatformWindow();
    std::vector<VkPresentModeKHR> present_mode_priority({VK_PRESENT_MODE_FIFO_KHR,
                                                         VK_PRESENT_MODE_IMMEDIATE_KHR,
                                                         VK_PRESENT_MODE_MAILBOX_KHR});

    std::vector<VkSurfaceFormatKHR> surface_format_priority({{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                             {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                             {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                             {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}});

    main_window->CreateSurface(GetInstance(), GetDevice().GetGPU());
    main_window->CreateRenderContext(GetDevice(), present_mode_priority, surface_format_priority);
    main_window->GetRenderContext().Prepare();

    GetLayerStack().PushLayer(std::make_shared<Game>(this, main_window, "first"));
    GetLayerStack().PushLayer(std::make_shared<Simple>(this, "second"));
    GetLayerStack().PushLayer(std::make_shared<engine::Gui>(this, main_window));
    return true;
}
