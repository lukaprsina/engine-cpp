#include "core/layer_stack.h"

#include "core/application.h"
#include "scene/scene.h" 

namespace engine
{
	LayerStack::LayerStack(Application &application)
		: m_Application(application)
	{
	}

	void LayerStack::PushLayer(const std::shared_ptr<Layer> layer)
	{
		m_Layers[layer->GetName()] = layer;
	}

	void LayerStack::PopLayer(Layer *layer)
	{
		layer->OnDetach();

		int i = 0;
		for (auto &scene : m_Application.GetScenes())
		{
			if (scene.get() == layer->GetScene())
				i++;
		}

		if (i <= 1)
		{
			for (auto &scene : m_Application.GetScenes())
			{
				if (scene.get() == layer->GetScene())
					scene.reset();
			}
		}

		for (auto &layer_pair : m_Layers)
		{
			if (layer == layer_pair.second.get())
				m_DestroyedLayers.emplace_back(layer_pair.first);
		}

		for (std::string &name : m_DestroyedLayers)
			m_Layers.erase(name);

		m_DestroyedLayers.clear();
	}

	/* void LayerStack::BringLayerToFront(Layer *layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
			m_Layers.emplace_back(layer);
		}
	}

	void LayerStack::SendLayerToBack(Layer *layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
			m_Layers.emplace(m_Layers.begin(), layer);
		}
	}

	void LayerStack::BringLayerForward(Layer *layer, unsigned int count)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end())
		{
			auto distance = it - m_Layers.begin();
			m_Layers.erase(it);

			m_Layers.insert(m_Layers.begin() + distance + count, layer);
		}
	}

	void LayerStack::SendLayerBackward(Layer *layer, unsigned int count)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end())
		{
			auto distance = it - m_Layers.begin();
			m_Layers.erase(it);

			m_Layers.insert(m_Layers.begin() + distance - count, layer);
		}
	} */
}