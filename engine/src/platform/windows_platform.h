#pragma once

#include "platform/platform.h"

namespace engine
{
    class WindowsPlatform : public Platform
    {
    public:
        WindowsPlatform(int argc, char *argv[]);
        virtual ~WindowsPlatform() = default;

        virtual bool Initialize(std::unique_ptr<Application> &&app) override;
        virtual void CreatePlatformWindow() override;
        virtual const char *GetSurfaceExtension() override;
    };
}