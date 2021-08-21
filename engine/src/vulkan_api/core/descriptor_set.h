#pragma once

namespace engine
{
    class Device;
    class DescriptorSetLayout;
    class DescriptorPool;

    class DescriptorSet
    {
    public:
        DescriptorSet(Device &device,
                      DescriptorSetLayout &descriptor_set_layout,
                      DescriptorPool &descriptor_pool,
                      const BindingMap<VkDescriptorBufferInfo> &buffer_infos = {},
                      const BindingMap<VkDescriptorImageInfo> &image_infos = {});

        DescriptorSet(const DescriptorSet &) = delete;

        DescriptorSet(DescriptorSet &&other);

        // The descriptor set handle is managed by the pool, and will be destroyed when the pool is reset
        ~DescriptorSet() = default;

        DescriptorSet &operator=(const DescriptorSet &) = delete;

        DescriptorSet &operator=(DescriptorSet &&) = delete;

        /* void Reset(const BindingMap<VkDescriptorBufferInfo> &new_buffer_infos = {},
                   const BindingMap<VkDescriptorImageInfo> &new_image_infos = {}); */

        void Update(const std::vector<uint32_t> &bindings_to_update = {});

        const DescriptorSetLayout &GetLayout() const { return m_DescriptorSetLayout; }
        VkDescriptorSet GetHandle() const { return m_Handle; }
        BindingMap<VkDescriptorBufferInfo> &GetBufferInfos() { return m_BufferInfos; }
        BindingMap<VkDescriptorImageInfo> &GetImageInfos() { return m_ImageInfos; }

        void Prepare();

    private:
        Device &m_Device;
        DescriptorSetLayout &m_DescriptorSetLayout;
        DescriptorPool &m_DescriptorPool;
        BindingMap<VkDescriptorBufferInfo> m_BufferInfos;
        BindingMap<VkDescriptorImageInfo> m_ImageInfos;
        VkDescriptorSet m_Handle{VK_NULL_HANDLE};
        std::vector<VkWriteDescriptorSet> m_WriteDescriptorSets;
        std::unordered_map<uint32_t, size_t> m_UpdatedBindings;
    };
}
