#pragma once

#include "events/event.h"
#include "vulkan_api/rendering/render_pipeline.h"

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
		virtual void OnEvent(Event &event);
		virtual bool OnWindowClose(WindowCloseEvent &event);
		virtual bool OnResize(WindowResizeEvent &event);
		virtual bool OnKeyPressed(KeyPressedEvent &event);

		Application &GetApp() { return m_Application; }
		std::string GetName() { return m_Name; }

		void SetScene(Scene *scene);
		Scene *GetScene() { return m_Scene; }

		void SetWindow(Window *window);
		Window *GetWindow() { return m_Window; }

		void SetCamera(Entity *camera) { m_Camera = camera; }
		Entity *GetCamera() { return m_Camera; }

		void SetRenderPipeline(RenderPipeline *render_pipeline) { m_RenderPipeline = render_pipeline; }
		RenderPipeline *GetRenderPipeline() { return m_RenderPipeline; }

		bool IsInitialized() { return m_Initialized; }

		friend class Application;
		friend class Platform;

	private:
		Application &m_Application;
		std::string m_Name{};
		Scene *m_Scene{};
		Entity *m_Camera{};
		Window *m_Window{};
		RenderPipeline *m_RenderPipeline{};

		bool m_Initialized{false};
	};
}