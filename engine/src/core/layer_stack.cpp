#include "core/layer_stack.h"

namespace engine
{
	void LayerStack::PushLayer(Layer *layer)
	{
		m_Layers.emplace_back(layer);
	}

	void LayerStack::PopLayer(Layer *layer)
	{
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
	}
}