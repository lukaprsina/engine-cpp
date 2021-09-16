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
        virtual ~UnixPlatform() = default;

        virtual bool Initialize(std::unique_ptr<Application> &&app) override;
        virtual VkSurfaceKHR CreatePlatformWindow(/* Instance &instance */) override;
        virtual const char *GetSurfaceExtension() override;

    private:
        UnixType m_Type;
    };
}