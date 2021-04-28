#pragma once

#include "window/window.h"

struct GLFWwindow;

namespace engine
{
    class GlfwWindow : public Window
    {
    public:
        GlfwWindow(Platform &platform,
                   uint32_t width = 1280,
                   uint32_t height = 720);

        virtual ~GlfwWindow();

        virtual void ProcessEvents() override;
        virtual VkSurfaceKHR CreateSurface(Instance &instance) override;
        virtual bool ShouldClose() const override;
        virtual void Close() override;

    private:
        GLFWwindow *m_Handle = nullptr;
    };
}