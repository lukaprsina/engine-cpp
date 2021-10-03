#include "core/layer.h"

#include "window/window.h"
#include "core/application.h"
#include "events/application_event.h"
#include "events/key_event.h"
#include "scene/scripts/free_camera.h"
#include "scene/scene.h"

namespace engine
{
    Layer::Layer(Application *application)
        : m_Application(application)
    {
    }

    Layer::~Layer()
    {
    }

    void Layer::OnEvent(Event &event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>(ENG_BIND_EVENT_FN(Layer::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(ENG_BIND_EVENT_FN(Layer::OnResize));
        dispatcher.Dispatch<KeyPressedEvent>(ENG_BIND_EVENT_FN(Layer::OnKeyPressed));
    }

    bool Layer::OnWindowClose(WindowCloseEvent & /*event*/)
    {
        return false;
    }

    bool Layer::OnResize(WindowResizeEvent &event)
    {
        auto view = m_Scene->GetRegistry().view<sg::FreeCamera>();
        for (auto &entity : view)
        {
            auto &free_camera = view.get<sg::FreeCamera>(entity);
            free_camera.Resize(event.GetWidth(), event.GetHeight());
        }

        return false;
    }

    bool Layer::OnKeyPressed(KeyPressedEvent &event)
    {
        bool alt_enter = m_Window->GetInput().IsKeyPressed(Key::LeftAlt) && m_Window->GetInput().IsKeyPressed(Key::Enter);

        if (event.GetKeyCode() == Key::F11 || alt_enter)
        {
            auto window_settings = m_Window->GetSettings();
            window_settings.fullscreen = !window_settings.fullscreen;
            m_Window->SetSettings(window_settings);
        }

        return false;
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
}