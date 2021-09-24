#pragma once

#include "events/event.h"

namespace engine
{
	class RenderContext;
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

		RenderContext &CreateRenderContext(Device &device,
										   std::vector<VkPresentModeKHR> &present_mode_priority,
										   std::vector<VkSurfaceFormatKHR> &surface_format_priority);
		RenderContext &GetRenderContext();
		void DeleteRenderContext();

	private:
		Application *m_Application;
		std::unique_ptr<RenderContext> m_RenderContext{};
		Scene *m_Scene;
		Window *m_Window;
	};
}