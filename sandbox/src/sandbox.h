#pragma once

#include "core/application.h"

#include <memory>

namespace engine
{
    class Platform;
}
class Sandbox : public engine::Application
{
    Sandbox(engine::Platform *platform);
};

std::unique_ptr<engine::Application> engine::CreateApplication(engine::Platform *platform)
{
    return std::make_unique<Sandbox>(platform);
}