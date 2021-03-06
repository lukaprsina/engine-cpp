#pragma once

#include "vulkan_api/rendering/pipeline_state.h"
#include "vulkan_api/rendering/resource_binding_state.h"
#include "vulkan_api/core/descriptor_set_layout.h"

namespace engine
{
    class CommandPool;
    class Framebuffer;
    class RenderPass;
    class Subpass;
    class RenderTarget;
    class PipelineState;
    class ResourceBindingState;
    struct LightingState;

    namespace core
    {
        class ImageView;
    }

    class CommandBuffer
    {
    public:
        CommandBuffer(CommandPool &command_pool, VkCommandBufferLevel level);
        ~CommandBuffer();
        CommandBuffer(CommandBuffer &&other);

        enum class ResetMode
        {
            ResetPool,
            ResetIndividually,
            AlwaysAllocate,
        };

        enum class State
        {
            Invalid,
            Initial,
            Recording,
            Executable,
        };

        struct RenderPassBinding
        {
            const RenderPass *render_pass;
            const Framebuffer *framebuffer;
        };

        VkResult Begin(VkCommandBufferUsageFlags flags, CommandBuffer *primary_cmd_buf = nullptr);
        VkResult Begin(VkCommandBufferUsageFlags flags, const RenderPass *render_pass, const Framebuffer *framebuffer, uint32_t subpass_index);
        VkResult End();
        void Flush(VkPipelineBindPoint pipeline_bind_point);
        void FlushPipelineState(VkPipelineBindPoint pipeline_bind_point);
        void FlushDescriptorState(VkPipelineBindPoint pipeline_bind_point);
        void FlushPushConstants();

        bool IsRecording() const { return (m_State == State::Recording); }
        VkResult Reset(ResetMode reset_mode);

        void CreateImageMemoryBarrier(const core::ImageView &image_view, const ImageMemoryBarrier &memory_barrier);
        const bool IsRenderSizeOptimal(const VkExtent2D &framebuffer_extent, const VkRect2D &render_area);
        void SetViewport(uint32_t first_viewport, const std::vector<VkViewport> &viewports);
        void SetScissor(uint32_t first_scissor, const std::vector<VkRect2D> &scissors);
        void BeginRenderPass(const RenderTarget &render_target, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<VkClearValue> &clear_values, const std::vector<std::unique_ptr<Subpass>> &subpasses, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
        void BeginRenderPass(const RenderTarget &render_target, const RenderPass &render_pass, const Framebuffer &framebuffer, const std::vector<VkClearValue> &clear_values, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
        RenderPass &GetRenderPass(const RenderTarget &render_target,
                                  const std::vector<LoadStoreInfo> &load_store_infos,
                                  const std::vector<std::unique_ptr<Subpass>> &subpasses);

        template <typename T>
        void PushConstants(const T &value)
        {
            auto data = ToBytes(value);

            uint32_t size = ToUint32_t(m_StoredPushConstants.size() + data.size());

            if (size > m_MaxPushConstantsSize)
            {
                ENG_CORE_ERROR("Push constant limit exceeded ({} / {} bytes)", size, m_MaxPushConstantsSize);
                throw std::runtime_error("Cannot overflow push constant limit");
            }

            m_StoredPushConstants.insert(m_StoredPushConstants.end(), data.begin(), data.end());
        }

        void PushConstants(const std::vector<uint8_t> &values);

        void BindBuffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t set, uint32_t binding, uint32_t array_element);
        void BindLighting(LightingState &lighting_state, uint32_t set, uint32_t binding);

        void BindVertexBuffers(uint32_t first_binding,
                               const std::vector<std::reference_wrapper<const core::Buffer>> &buffers,
                               const std::vector<VkDeviceSize> &offsets);

        void BindIndexBuffer(const core::Buffer &buffer, VkDeviceSize offset, VkIndexType index_type);

        void NextSubpass();
        void CopyBufferToImage(const core::Buffer &buffer, const core::Image &image, const std::vector<VkBufferImageCopy> &regions);
        void EndRenderPass();

        void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);
        void DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance);

        template <class T>
        void SetSpecializationConstant(uint32_t constant_id, const T &data);

        void SetSpecializationConstant(uint32_t constant_id, const std::vector<uint8_t> &data);

        Device &GetDevice() const;
        const VkCommandBuffer &GetHandle() const { return m_Handle; }
        PipelineState &GetPipelineState() { return m_PipelineState; }
        ResourceBindingState &GetResourceBindingState() { return m_ResourceBindingState; }

        const VkCommandBufferLevel m_Level;

    private:
        State m_State{State::Initial};
        CommandPool &m_CommandPool;
        VkCommandBuffer m_Handle{VK_NULL_HANDLE};

        RenderPassBinding m_CurrentRenderPass;
        PipelineState m_PipelineState;
        ResourceBindingState m_ResourceBindingState;
        std::vector<uint8_t> m_StoredPushConstants;
        uint32_t m_MaxPushConstantsSize{0};

        VkExtent2D m_LastFramebufferExtent{};
        VkExtent2D m_LastRenderAreaExtent{};

        // If true, it becomes the responsibility of the caller to update ANY descriptor bindings
        // that contain update after bind, as they wont be implicitly updated
        bool m_UpdateAfterBind{false};
        std::unordered_map<uint32_t, DescriptorSetLayout *> m_DescriptorSetLayoutBindingState;

        const CommandBuffer::RenderPassBinding &GetCurrentRenderPass() const { return m_CurrentRenderPass; }
        const uint32_t GetCurrentSubpassIndex() const { return m_PipelineState.GetSubpassIndex(); }
    };

    template <class T>
    inline void CommandBuffer::SetSpecializationConstant(uint32_t constant_id, const T &data)
    {
        SetSpecializationConstant(constant_id, ToBytes(data));
    }

    template <>
    inline void CommandBuffer::SetSpecializationConstant<bool>(std::uint32_t constant_id, const bool &data)
    {
        SetSpecializationConstant(constant_id, ToBytes(ToUint32_t(data)));
    }
}
