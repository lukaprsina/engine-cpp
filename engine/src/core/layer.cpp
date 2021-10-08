#include "core/layer.h"

#include "window/window.h"
#include "core/application.h"
#include "events/application_event.h"
#include "events/key_event.h"
#include "scene/scripts/free_camera.h"
#include "scene/scene.h"
#include "scene/components/perspective_camera.h"
#include "scene/components/transform.h"
#include "scene/entity.h"

namespace engine
{
    Layer::Layer(Application *application)
        : m_Application(application)
    {
    }

    Layer::~Layer()
    {
    }

    void Layer::AddFreeCamera(VkExtent2D extent, Window *window)
    {
        auto &cameras = m_Scene->GetCameras();
        static int camera_counter = 0;
        if (cameras.size() > camera_counter)
        {
            m_Camera = cameras[camera_counter].get();
        }
        else
        {
            auto entity = m_Scene->CreateEntity();
            entity.AddComponent<sg::PerspectiveCamera>("Camera");
            entity.AddComponent<sg::Transform>(entity);
            cameras.emplace_back(std::make_unique<Entity>(entity));
            m_Camera = cameras.back().get();
        }

        camera_counter++;
        auto free_camera_script = m_Camera->AddComponent<sg::FreeCamera>(GetScene(), window);
        auto &perspective_camera = m_Camera->GetComponent<sg::PerspectiveCamera>();
        free_camera_script.Resize(perspective_camera, extent.width, extent.height);
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
        auto view = m_Scene->GetRegistry().view<sg::FreeCamera, sg::PerspectiveCamera>();
        for (auto &entity : view)
        {
            auto &[free_camera, perspective_camera] = view.get<sg::FreeCamera, sg::PerspectiveCamera>(entity);
            free_camera.Resize(perspective_camera, event.GetWidth(), event.GetHeight());
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
        if (m_Camera)
            m_Scene->GetRegistry().destroy(m_Camera->GetHandle());
    }
}