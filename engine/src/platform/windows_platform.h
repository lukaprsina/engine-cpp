#pragma once

#include "platform/platform.h"
#include <Windows.h>

namespace engine
{
    class WindowsPlatform : public Platform
    {
    public:
        WindowsPlatform(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	                PSTR lpCmdLine, INT nCmdShow);
        virtual ~WindowsPlatform() = default;

        virtual bool Initialize(std::unique_ptr<Application> &&app) override;
        virtual void CreatePlatformWindow() override;    
    };
}