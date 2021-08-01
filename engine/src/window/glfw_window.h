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

        virtual ~GlfwWindow();

        virtual void ProcessEvents() override;
        virtual VkSurfaceKHR CreateSurface(Instance &instance) override;
        virtual bool ShouldClose() const override;
        virtual void Close() override;

    private:
        GLFWwindow *m_Handle{nullptr};
    };
}