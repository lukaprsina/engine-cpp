#include "vulkan_api/command_buffer.h"

#include "vulkan_api/command_pool.h"
#include "vulkan_api/subpasses/subpass.h"
#include "vulkan_api/device.h"
#include "vulkan_api/render_target.h"
#include "vulkan_api/core/framebuffer.h"
#include "vulkan_api/core/image_view.h"
#include "vulkan_api/core/render_pass.h"

namespace engine
{
    CommandBuffer::CommandBuffer(CommandPool &command_pool, VkCommandBufferLevel level)
        : m_Level{level},
          m_CommandPool{command_pool},
          m_MaxPushConstantsSize{command_pool.GetDevice().GetGPU().GetProperties().limits.maxPushConstantsSize}
    {
        VkCommandBufferAllocateInfo allocate_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};

        allocate_info.commandPool = command_pool.GetHandle();
        allocate_info.commandBufferCount = 1;
        allocate_info.level = level;

        VkResult result = vkAllocateCommandBuffers(command_pool.GetDevice().GetHandle(), &allocate_info, &m_Handle);

        if (result != VK_SUCCESS)
        {
            throw VulkanException{result, "Failed to allocate command buffer"};
        }
    }

    CommandBuffer::~CommandBuffer()
    {
        if (m_Handle != VK_NULL_HANDLE)
        {
            vkFreeCommandBuffers(m_CommandPool.GetDevice().GetHandle(),
                                 m_CommandPool.GetHandle(), 1, &m_Handle);
        }
    }

    CommandBuffer::CommandBuffer(CommandBuffer &&other)
        : m_Level{other.m_Level},
          m_State{other.m_State},
          m_CommandPool{other.m_CommandPool},
          m_Handle{other.m_Handle},
          m_UpdateAfterBind{other.m_UpdateAfterBind}
    {
        other.m_Handle = VK_NULL_HANDLE;
        other.m_State = State::Invalid;
    }

    VkResult CommandBuffer::Begin(VkCommandBufferUsageFlags flags, CommandBuffer *primary_cmd_buf)
    {
        if (m_Level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
        {
            assert(primary_cmd_buf && "A primary command buffer pointer must be provided when calling begin from a secondary one");
            auto render_pass_binding = primary_cmd_buf->GetCurrentRenderPass();

            return Begin(flags, render_pass_binding.render_pass, render_pass_binding.framebuffer, primary_cmd_buf->GetCurrentSubpassIndex());
        }

        return Begin(flags, nullptr, nullptr, 0);
    }

    VkResult CommandBuffer::Begin(VkCommandBufferUsageFlags flags, const RenderPass *render_pass, const Framebuffer *framebuffer, uint32_t subpass_index)
    {
        assert(!IsRecording() && "Command buffer is already recording, please call end before beginning again");

        if (IsRecording())
        {
            return VK_NOT_READY;
        }

        m_State = State::Recording;

        // Reset state
        m_PipelineState.Reset();
        m_ResourceBindingState.Reset();
        m_DescriptorSetLayoutBindingState.clear();
        m_StoredPushConstants.clear();

        VkCommandBufferBeginInfo begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        VkCommandBufferInheritanceInfo inheritance = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
        begin_info.flags = flags;

        if (m_Level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
        {
            assert((render_pass && framebuffer) && "Render pass and framebuffer must be provided when calling begin from a secondary one");

            m_CurrentRenderPass.render_pass = render_pass;
            m_CurrentRenderPass.framebuffer = framebuffer;

            inheritance.renderPass = m_CurrentRenderPass.render_pass->GetHandle();
            inheritance.framebuffer = m_CurrentRenderPass.framebuffer->GetHandle();
            inheritance.subpass = subpass_index;

            begin_info.pInheritanceInfo = &inheritance;
        }

        return vkBeginCommandBuffer(GetHandle(), &begin_info);
    }

    VkResult CommandBuffer::End()
    {
        assert(IsRecording() && "Command buffer is not recording, please call begin before end");

        if (!IsRecording())
        {
            return VK_NOT_READY;
        }

        vkEndCommandBuffer(m_Handle);

        m_State = State::Executable;

        return VK_SUCCESS;
    }

    void CommandBuffer::BeginRenderPass(const RenderTarget &render_target, const std::vector<LoadStoreInfo> &load_store_infos, const std::vector<VkClearValue> &clear_values, const std::vector<std::unique_ptr<Subpass>> &subpasses, VkSubpassContents contents)
    {
        // Reset state
        m_PipelineState.Reset();
        m_ResourceBindingState.Reset();
        m_DescriptorSetLayoutBindingState.clear();

        auto &render_pass = GetRenderPass(render_target, load_store_infos, subpasses);
        auto &framebuffer = m_CommandPool.GetDevice().GetResourceCache().RequestFramebuffer(render_target, render_pass);

        BeginRenderPass(render_target, render_pass, framebuffer, clear_values, contents);
    }

    void CommandBuffer::BeginRenderPass(const RenderTarget &render_target, const RenderPass &render_pass, const Framebuffer &framebuffer, const std::vector<VkClearValue> &clear_values, VkSubpassContents contents)
    {
        m_CurrentRenderPass.render_pass = &render_pass;
        m_CurrentRenderPass.framebuffer = &framebuffer;

        // Begin render pass
        VkRenderPassBeginInfo begin_info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        begin_info.renderPass = m_CurrentRenderPass.render_pass->GetHandle();
        begin_info.framebuffer = m_CurrentRenderPass.framebuffer->GetHandle();
        begin_info.renderArea.extent = render_target.GetExtent();
        begin_info.clearValueCount = ToUint32_t(clear_values.size());
        begin_info.pClearValues = clear_values.data();

        const auto &framebuffer_extent = m_CurrentRenderPass.framebuffer->GetExtent();

        // Test the requested render area to confirm that it is optimal and could not cause a performance reduction
        if (!IsRenderSizeOptimal(framebuffer_extent, begin_info.renderArea))
        {
            // Only prints the warning if the framebuffer or render area are different since the last time the render size was not optimal
            if (framebuffer_extent.width != m_LastFramebufferExtent.width || framebuffer_extent.height != m_LastFramebufferExtent.height ||
                begin_info.renderArea.extent.width != m_LastRenderAreaExtent.width || begin_info.renderArea.extent.height != m_LastRenderAreaExtent.height)
            {
                ENG_CORE_WARN("Render target extent is not an optimal size, this may result in reduced performance.");
            }

            m_LastFramebufferExtent = m_CurrentRenderPass.framebuffer->GetExtent();
            m_LastRenderAreaExtent = begin_info.renderArea.extent;
        }

        vkCmdBeginRenderPass(m_Handle, &begin_info, contents);

        // Update blend state attachments for first subpass
        auto blend_state = m_PipelineState.GetColorBlendState();
        blend_state.attachments.resize(m_CurrentRenderPass.render_pass->GetColorOutputCount(m_PipelineState.GetSubpassIndex()));
        m_PipelineState.SetColorBlendState(blend_state);
    }

    RenderPass &CommandBuffer::GetRenderPass(const RenderTarget &render_target,
                                             const std::vector<LoadStoreInfo> &load_store_infos,
                                             const std::vector<std::unique_ptr<Subpass>> &subpasses)
    {
        // Create render pass
        assert(subpasses.size() > 0 && "Cannot create a render pass without any subpass");

        std::vector<SubpassInfo> subpass_infos(subpasses.size());
        auto subpass_info_it = subpass_infos.begin();
        for (auto &subpass : subpasses)
        {
            subpass_info_it->input_attachments = subpass->GetInputAttachments();
            subpass_info_it->output_attachments = subpass->GetOutputAttachments();
            subpass_info_it->color_resolve_attachments = subpass->GetColorResolveAttachments();
            subpass_info_it->disable_depth_stencil_attachment = subpass->GetDisableDepthStencilAttachment();
            subpass_info_it->depth_stencil_resolve_mode = subpass->GetDepthStencilResolveMode();
            subpass_info_it->depth_stencil_resolve_attachment = subpass->GetDepthStencilResolveAttachment();

            ++subpass_info_it;
        }

        return m_CommandPool.GetDevice().GetResourceCache().RequestRenderPass(render_target.GetAttachments(), load_store_infos, subpass_infos);
    }

    void CommandBuffer::NextSubpass()
    {
        // Increment subpass index
        m_PipelineState.SetSubpassIndex(m_PipelineState.GetSubpassIndex() + 1);

        // Update blend state attachments
        auto blend_state = m_PipelineState.GetColorBlendState();
        blend_state.attachments.resize(m_CurrentRenderPass.render_pass->GetColorOutputCount(m_PipelineState.GetSubpassIndex()));
        m_PipelineState.SetColorBlendState(blend_state);

        // Reset descriptor sets
        m_ResourceBindingState.Reset();
        m_DescriptorSetLayoutBindingState.clear();

        // Clear stored push constants
        m_StoredPushConstants.clear();

        vkCmdNextSubpass(GetHandle(), VK_SUBPASS_CONTENTS_INLINE);
    }

    VkResult CommandBuffer::Reset(ResetMode reset_mode)
    {
        VkResult result = VK_SUCCESS;

        assert(reset_mode == m_CommandPool.GetResetMode() && "Command buffer reset mode must match the one used by the pool to allocate it");

        m_State = State::Initial;

        if (reset_mode == ResetMode::ResetIndividually)
        {
            result = vkResetCommandBuffer(m_Handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        }

        return result;
    }

    void CommandBuffer::CreateImageMemoryBarrier(const core::ImageView &image_view, const ImageMemoryBarrier &memory_barrier)
    {
        auto subresource_range = image_view.GetSubresourceRange();
        auto format = image_view.GetFormat();
        if (IsDepthOnlyFormat(format))
        {
            subresource_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        else if (IsDepthStencilFormat(format))
        {
            subresource_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        VkImageMemoryBarrier image_memory_barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        image_memory_barrier.oldLayout = memory_barrier.old_layout;
        image_memory_barrier.newLayout = memory_barrier.new_layout;
        image_memory_barrier.image = image_view.GetImage().GetHandle();
        image_memory_barrier.subresourceRange = subresource_range;
        image_memory_barrier.srcAccessMask = memory_barrier.src_access_mask;
        image_memory_barrier.dstAccessMask = memory_barrier.dst_access_mask;
        image_memory_barrier.srcQueueFamilyIndex = memory_barrier.old_queue_family;
        image_memory_barrier.dstQueueFamilyIndex = memory_barrier.new_queue_family;

        VkPipelineStageFlags src_stage_mask = memory_barrier.src_stage_mask;
        VkPipelineStageFlags dst_stage_mask = memory_barrier.dst_stage_mask;

        vkCmdPipelineBarrier(
            m_Handle,
            src_stage_mask,
            dst_stage_mask,
            0,
            0, nullptr,
            0, nullptr,
            1,
            &image_memory_barrier);
    }

    const bool CommandBuffer::IsRenderSizeOptimal(const VkExtent2D &framebuffer_extent, const VkRect2D &render_area)
    {
        auto render_area_granularity = m_CurrentRenderPass.render_pass->GetRenderAreaGranularity();

        return ((render_area.offset.x % render_area_granularity.width == 0) && (render_area.offset.y % render_area_granularity.height == 0) &&
                ((render_area.extent.width % render_area_granularity.width == 0) || (render_area.offset.x + render_area.extent.width == framebuffer_extent.width)) &&
                ((render_area.extent.height % render_area_granularity.height == 0) || (render_area.offset.y + render_area.extent.height == framebuffer_extent.height)));
    }

    void CommandBuffer::SetViewport(uint32_t first_viewport, const std::vector<VkViewport> &viewports)
    {
        vkCmdSetViewport(m_Handle, first_viewport, ToUint32_t(viewports.size()), viewports.data());
    }

    void CommandBuffer::SetScissor(uint32_t first_scissor, const std::vector<VkRect2D> &scissors)
    {
        vkCmdSetScissor(m_Handle, first_scissor, ToUint32_t(scissors.size()), scissors.data());
    }

    void CommandBuffer::EndRenderPass()
    {
        vkCmdEndRenderPass(m_Handle);
    }

    void CommandBuffer::CopyBufferToImage(const core::Buffer &buffer, const core::Image &image, const std::vector<VkBufferImageCopy> &regions)
    {
        vkCmdCopyBufferToImage(m_Handle, buffer.GetHandle(),
                               image.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               ToUint32_t(regions.size()), regions.data());
    }
}
