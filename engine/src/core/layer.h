#pragma once

#include "events/event.h"

namespace engine
{
	class Scene;
	class Window;
	class Device;
	class Application;

	class Layer
	{
	public:
		Layer(Application *application);
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(float delta_time) {}
		virtual void OnEvent(Event &event) {}

		Application *GetApp() { return m_Application; }

		void SetScene(Scene *scene) { m_Scene = scene; }
		Scene *GetScene() { return m_Scene; }

		void SetWindow(Window *Window) { m_Window = Window; }
		Window *GetWindow() { return m_Window; }

	private:
		Application *m_Application;
		Scene *m_Scene;
		Window *m_Window;
	};
}