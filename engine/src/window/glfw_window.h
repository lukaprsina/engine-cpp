#pragma once

#include "window/window.h"
#include "events/key_event.h"

struct GLFWwindow;

namespace engine
{
    class GlfwWindow : public Window
    {
    public:
        GlfwWindow(Platform &platform,
                   WindowSettings &settings);

        ~GlfwWindow();

        static void Init();
        static void DeInit();
        void ProcessEvents() override;
        VkSurfaceKHR CreateSurface(Instance &instance, PhysicalDevice &physical_device) override;
        bool ShouldClose() const override;
        void Close() override;
        void *GetNativeWindow() override { return reinterpret_cast<void *>(m_Handle); }

    private:
        GLFWwindow *m_Handle{nullptr};
    };
}