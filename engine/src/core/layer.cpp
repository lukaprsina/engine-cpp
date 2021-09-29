#include "core/layer.h"

#include "window/window.h"
#include "core/application.h"

namespace engine
{
    Layer::Layer(Application *application)
        : m_Application(application)
    {
    }

    Layer::~Layer()
    {
    }

    void Layer::SetScene(Scene *scene)
    {
        m_Scene = scene;
        m_Window->AddScene(scene);
    }

    void Layer::SetWindow(Window *window)
    {
        m_Window = window;
        m_Window->AddLayer(this);
    }

    void Layer::OnDetach()
    {
    }

    void Layer::OnWindowClose(Window &window)
    {
        m_Application->GetLayerStack().PopLayer(this);
    }
}