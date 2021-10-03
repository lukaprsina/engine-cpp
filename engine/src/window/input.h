#pragma once

#include "common/glm.h"
#include "events/key_event.h"
#include "events/mouse_event.h"

namespace engine
{
    class Window;

    class Input
    {
    public:
        Input(Window *window);
        bool IsKeyPressed(KeyCode key);
        bool IsMouseButtonPressed(MouseCode button);
        glm::vec2 GetMousePosition();
        float GetMouseX();
        float GetMouseY();

    private:
        Window *m_Window;
    };
}
