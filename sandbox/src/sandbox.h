#pragma once

#include "core/application.h"

class Sandbox : public engine::Application
{
    Sandbox() = default;
    ~Sandbox() = default;
};

engine::Application *engine::CreateApplication()
{
    return new Sandbox();
}