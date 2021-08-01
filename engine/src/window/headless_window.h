#pragma once

#include "window/window.h"

namespace engine
{
    class HeadlessWindow : public Window
    {
    public:
        HeadlessWindow(Platform &platform,
                       WindowSettings &settings);

        virtual ~HeadlessWindow();

        virtual VkSurfaceKHR CreateSurface(Instance &instance) override;
        virtual bool ShouldClose() const override;
        virtual void Close() override;

    private:
        bool m_Closed{false};
    };
}