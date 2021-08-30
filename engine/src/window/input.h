#pragma once

#include "common/glm.h"
#include "events/key_event.h"
#include "events/mouse_event.h"

namespace engine
{
    class Input
    {
    public:
        static bool IsKeyPressed(KeyCode key);
        static bool IsMouseButtonPressed(MouseCode button);
        static glm::vec2 GetMousePosition();

        static void *m_WindowPointer;
    };
}
