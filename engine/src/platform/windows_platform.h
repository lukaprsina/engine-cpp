#pragma once

#include "platform/platform.h"

namespace engine
{
    class WindowsPlatform : public Platform
    {
    public:
        WindowsPlatform(HINSTANCE hInstance, HINSTANCE hPrevInstance,
            PSTR lpCmdLine, INT nCmdShow);
        virtual ~WindowsPlatform() = default;

        bool Initialize(std::unique_ptr<Application>&& app) override;
        Window* CreatePlatformWindow() override;        
        const char* GetSurfaceExtension() override;
    };
}