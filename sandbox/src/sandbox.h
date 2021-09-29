#pragma once

#include "core/application.h"
#include "core/layer.h"
#include "platform/platform.h"

#include <memory>

class Game : public engine::Layer
{
public:
    Game(engine::Application *application);
    void OnAttach() override;
};

class Sandbox : public engine::Application
{
public:
    Sandbox(engine::Platform *platform);
    bool Prepare() override;
    void DestroyLayer(engine::Layer *layer) override;

    // TODO: sandbox doesn't own the unique_ptr of layers
    friend class Game;

private:
    std::unique_ptr<Game> m_Game;
    std::unique_ptr<Game> m_PopUp;
};

std::unique_ptr<engine::Application> engine::CreateApplication(engine::Platform *platform)
{
    auto app = std::make_unique<Sandbox>(platform);
    return std::move(app);
}