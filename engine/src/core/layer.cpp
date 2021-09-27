#include "core/layer.h"

#include "window/window.h"

namespace engine
{
    Layer::Layer(Application *application)
        : m_Application(application)
    {
    }

    void Layer::SetScene(Scene *scene)
    {
        m_Scene = scene;
        m_Window->AddScene(scene);
    }
}