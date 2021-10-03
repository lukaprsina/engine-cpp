#pragma once

#include "core/application.h"
#include "common/base.h"
#include "core/layer.h"
#include "platform/platform.h"

#include <memory>

class Game : public engine::Layer
{
public:
    Game(engine::Application *application);
    void OnAttach() override;
};

class Simple : public engine::Layer
{
public:
    Simple(engine::Application *application);
    void OnAttach() override;
};

class Sandbox : public engine::Application
{
public:
    Sandbox(engine::Platform *platform);
    bool Prepare() override;
};

ENG_LOAD_APP(Sandbox)