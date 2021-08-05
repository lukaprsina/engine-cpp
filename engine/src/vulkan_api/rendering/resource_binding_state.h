#pragma once

#include "vulkan_api/core/image_view.h"
#include "vulkan_api/core/sampler.h"
#include "vulkan_api/core/buffer.h"

namespace engine
{
    struct ResourceInfo
    {
        bool dirty{false};
        const core::Buffer *buffer{nullptr};
        VkDeviceSize offset{0};
        VkDeviceSize range{0};
        const core::ImageView *image_view{nullptr};
        const core::Sampler *sampler{nullptr};
    };

    class ResourceSet
    {
    public:
        void BindBuffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t binding, uint32_t array_element);
        void BindImage(const core::ImageView &image_view, const core::Sampler &sampler, uint32_t binding, uint32_t array_element);
        void BindImage(const core::ImageView &image_view, uint32_t binding, uint32_t array_element);
        void BindInput(const core::ImageView &image_view, uint32_t binding, uint32_t array_element);

        void Reset();
        bool IsDirty() const { return m_Dirty; }
        void ClearDirty() { m_Dirty = false; }
        void ClearDirty(uint32_t binding, uint32_t array_element);
        const BindingMap<ResourceInfo> &GetResourceBindings() const;

    private:
        bool m_Dirty{false};
        BindingMap<ResourceInfo> m_ResourceBindings;
    };

    class ResourceBindingState
    {
    public:
        void BindBuffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t set, uint32_t binding, uint32_t array_element);
        void BindImage(const core::ImageView &image_view, const core::Sampler &sampler, uint32_t set, uint32_t binding, uint32_t array_element);
        void BindImage(const core::ImageView &image_view, uint32_t set, uint32_t binding, uint32_t array_element);
        void BindInput(const core::ImageView &image_view, uint32_t set, uint32_t binding, uint32_t array_element);

        void Reset();
        bool IsDirty() const { return m_Dirty; }
        void ClearDirty() { m_Dirty = false; }
        void ClearDirty(uint32_t set);
        const std::unordered_map<uint32_t, ResourceSet> &GetResourceSets() const;

    private:
        bool m_Dirty{false};
        std::unordered_map<uint32_t, ResourceSet> m_ResourceSets;
    };
}
