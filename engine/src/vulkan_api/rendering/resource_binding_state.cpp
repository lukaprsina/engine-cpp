#include "vulkan_api/rendering/resource_binding_state.h"

namespace engine
{
    void ResourceBindingState::BindBuffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t set, uint32_t binding, uint32_t array_element)
    {
        m_ResourceSets[set].BindBuffer(buffer, offset, range, binding, array_element);

        m_Dirty = true;
    }

    void ResourceBindingState::BindImage(const core::ImageView &image_view, const core::Sampler &sampler, uint32_t set, uint32_t binding, uint32_t array_element)
    {
        m_ResourceSets[set].BindImage(image_view, sampler, binding, array_element);

        m_Dirty = true;
    }

    void ResourceBindingState::BindImage(const core::ImageView &image_view, uint32_t set, uint32_t binding, uint32_t array_element)
    {
        m_ResourceSets[set].BindImage(image_view, binding, array_element);

        m_Dirty = true;
    }

    void ResourceBindingState::BindInput(const core::ImageView &image_view, uint32_t set, uint32_t binding, uint32_t array_element)
    {
        m_ResourceSets[set].BindInput(image_view, binding, array_element);

        m_Dirty = true;
    }

    void ResourceBindingState::Reset()
    {
        ClearDirty();
        m_ResourceSets.clear();
    }

    void ResourceBindingState::ClearDirty(uint32_t set)
    {
        m_ResourceSets[set].ClearDirty();
    }

    const std::unordered_map<uint32_t, ResourceSet> &ResourceBindingState::GetResourceSets() const
    {
        return m_ResourceSets;
    }

    void ResourceSet::BindBuffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t binding, uint32_t array_element)
    {
        m_ResourceBindings[binding][array_element].dirty = true;
        m_ResourceBindings[binding][array_element].buffer = &buffer;
        m_ResourceBindings[binding][array_element].offset = offset;
        m_ResourceBindings[binding][array_element].range = range;

        m_Dirty = true;
    }

    void ResourceSet::BindImage(const core::ImageView &image_view, const core::Sampler &sampler, uint32_t binding, uint32_t array_element)
    {
        m_ResourceBindings[binding][array_element].dirty = true;
        m_ResourceBindings[binding][array_element].image_view = &image_view;
        m_ResourceBindings[binding][array_element].sampler = &sampler;

        m_Dirty = true;
    }

    void ResourceSet::BindImage(const core::ImageView &image_view, uint32_t binding, uint32_t array_element)
    {
        m_ResourceBindings[binding][array_element].dirty = true;
        m_ResourceBindings[binding][array_element].image_view = &image_view;
        m_ResourceBindings[binding][array_element].sampler = nullptr;

        m_Dirty = true;
    }

    void ResourceSet::BindInput(const core::ImageView &image_view, const uint32_t binding, const uint32_t array_element)
    {
        m_ResourceBindings[binding][array_element].dirty = true;
        m_ResourceBindings[binding][array_element].image_view = &image_view;

        m_Dirty = true;
    }

    void ResourceSet::Reset()
    {
        ClearDirty();

        m_ResourceBindings.clear();
    }

    void ResourceSet::ClearDirty(uint32_t binding, uint32_t array_element)
    {
        m_ResourceBindings[binding][array_element].dirty = false;
    }

    const BindingMap<ResourceInfo> &ResourceSet::GetResourceBindings() const
    {
        return m_ResourceBindings;
    }
}
