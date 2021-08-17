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
        void NextSubpass();
        void EndRenderPass();
        void CopyBufferToImage(const core::Buffer &buffer, const core::Image &image, const std::vector<VkBufferImageCopy> &regions);

        const VkCommandBuffer &GetHandle() const { return m_Handle; }

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
}
