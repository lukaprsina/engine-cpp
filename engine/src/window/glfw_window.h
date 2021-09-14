#pragma once

#include "window/window.h"
#include "events/key_event.h"

struct GLFWwindow;

namespace engine
{
    class Instance;
    class Device;

    class GlfwWindow : public Window
    {
    public:
        GlfwWindow(Platform &platform,
                   WindowSettings &settings,
                   Instance &instance,
                   VkSurfaceKHR &surface);

        ~GlfwWindow();

        void CreateRenderContext(Device &device,
                                 std::vector<VkPresentModeKHR> &present_mode_priority,
                                 std::vector<VkSurfaceFormatKHR> &surface_format_priority);
        static void Init();
        void ProcessEvents() override;
        VkSurfaceKHR CreateSurface(Instance &instance) override;
        bool ShouldClose() const override;
        void Close() override;
        void *GetNativeWindow() override { return reinterpret_cast<void *>(m_Handle); }

    private:
        GLFWwindow *m_Handle{nullptr};
        VkSurfaceKHR m_Surface;
    };
}