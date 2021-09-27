#pragma once

#include "core/application.h"
#include "core/layer.h"
#include "platform/platform.h"

#include <memory>

class Game : public engine::Layer
{
public:
    Game(engine::Application *application);
    void OnAttach();
};

class Sandbox : public engine::Application
{
public:
    Sandbox(engine::Platform *platform);
    bool Prepare() override;

private:
    std::unique_ptr<engine::Layer> m_Game;
    std::unique_ptr<engine::Layer> m_PopUp;
};

std::unique_ptr<engine::Application> engine::CreateApplication(engine::Platform *platform)
{
    auto app = std::make_unique<Sandbox>(platform);
    return std::move(app);
}