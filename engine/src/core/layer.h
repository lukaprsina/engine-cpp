#pragma once

#include "events/event.h"

namespace engine
{
	class Scene;
	class Window;
	class Device;
	class Application;
	class WindowCloseEvent;
	class WindowResizeEvent;
	class KeyPressedEvent;
	class Event;
	class Entity;
	class CommandBuffer;

	class Layer
	{
	public:
		Layer(Application *application, const std::string &name);
		virtual ~Layer();

		void AddFreeCamera(VkExtent2D extent, Window *window);

		virtual void OnAttach() {}
		virtual void OnDetach();
		virtual void OnUpdate(float delta_time) {}
		virtual void Draw(CommandBuffer &command_buffer) {}
		virtual void OnEvent(Event &event);
		virtual bool OnWindowClose(WindowCloseEvent &event);
		virtual bool OnResize(WindowResizeEvent &event);
		virtual bool OnKeyPressed(KeyPressedEvent &event);

		Application &GetApp() { return m_Application; }
		std::string GetName() { return m_Name; }

		void SetScene(Scene *scene);
		Scene *GetScene() { return m_Scene; }

		void SetWindow(Window *Window);
		Window *GetWindow() { return m_Window; }

		void SetCamera(Entity *Camera);
		Entity *GetCamera() { return m_Camera; }

	private:
		std::string m_Name{};
		Application &m_Application;
		Scene *m_Scene{};
		Entity *m_Camera{};
		Window *m_Window{};
	};
}