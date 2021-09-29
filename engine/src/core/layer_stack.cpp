#include "core/layer_stack.h"

#include "core/application.h"

namespace engine
{
	LayerStack::LayerStack(Application &application)
		: m_Application(application)
	{
	}

	void LayerStack::PushLayer(const std::string &name, const std::shared_ptr<Layer> layer)
	{
		m_Layers[name] = layer;
	}

	/* void LayerStack::PopLayer(Layer *layer)
	{
		layer->OnDetach();

		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
		}
	}

	void LayerStack::BringLayerToFront(Layer *layer)
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