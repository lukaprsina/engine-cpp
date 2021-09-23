#pragma once

#include "events/event.h"

namespace engine
{
	class RenderContext;
	class Scene;
	class Window;
	class Device;

	class Layer
	{
	public:
		Layer() = default;
		virtual ~Layer() = default;

		virtual void OnAttach(){};
		virtual void OnDetach() {}
		virtual void OnUpdate(float delta_time) = 0;
		virtual void OnEvent(Event &event) {}

		void SetActiveScene(Scene *scene) { m_Scene = scene; }
		Scene *GetActiveScene() { return m_Scene; }

		void SetActiveWindow(Window *Window) { m_Window = Window; }
		Window *GetActiveWindow() { return m_Window; }

		RenderContext &CreateRenderContext(Device &device,
										   std::vector<VkPresentModeKHR> &present_mode_priority,
										   std::vector<VkSurfaceFormatKHR> &surface_format_priority);
		RenderContext &GetRenderContext();
		void DeleteRenderContext();

	private:
		std::unique_ptr<RenderContext> m_RenderContext{};
		Scene *m_Scene;
		Window *m_Window;
	};
}