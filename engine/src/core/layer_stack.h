#pragma once

#include "core/layer.h"

namespace engine
{
	class Application;

	class LayerStack
	{
	public:
		LayerStack(Application &application);
		void PushLayer(const std::string &name, const std::shared_ptr<Layer> layer);
		void PopLayer(Layer *layer);
		// void PopLayer(const std::string &name);

		// void BringLayerToFront(Layer *layer);
		// void SendLayerToBack(Layer *layer);

		// void BringLayerForward(Layer *layer, unsigned int count = 1);
		// void SendLayerBackward(Layer *layer, unsigned int count = 1);

		std::map<std::string, std::shared_ptr<Layer>> &GetLayers() { return m_Layers; }

	private:
		Application &m_Application;
		std::map<std::string, std::shared_ptr<Layer>> m_Layers;
		std::vector<std::string> m_DestroyedLayers{};
	};
}