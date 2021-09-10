#pragma once

#include "events/event.h"

namespace engine
{
	class CommandBuffer;

	class Layer
	{
	public:
		Layer() = default;
		virtual ~Layer() = default;

		virtual void OnAttach(){};
		virtual void OnDetach() {}
		virtual void OnUpdate(float delta_time) = 0;
		virtual void OnEvent(Event &event) {}
	};
}