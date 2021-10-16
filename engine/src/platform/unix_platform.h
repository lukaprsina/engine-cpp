#pragma once

#include "platform/platform.h"

namespace engine
{
    enum class UnixType
    {
        Mac,
        Linux
    };

    class UnixPlatform : public Platform
    {
    public:
        UnixPlatform(const UnixType &type, int argc, char *argv[]);
        ~UnixPlatform() = default;

        bool Initialize(std::unique_ptr<Application> &&app) override;
        Window *CreatePlatformWindow(WindowSettings settings = WindowSettings()) override;
        void Terminate(ExitCode) override;
        const char *GetSurfaceExtension() override;

    private:
        UnixType m_Type;
    };
}