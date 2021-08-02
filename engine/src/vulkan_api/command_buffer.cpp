#include "vulkan_api/command_buffer.h"

#include "vulkan_api/command_pool.h"
#include "vulkan_api/device.h"

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
        assert(!IsRecording() && "Command buffer is already recording, please call end before beginning again");

        if (IsRecording())
        {
            return VK_NOT_READY;
        }

        m_State = State::Recording;

        // Reset state
        /* m_PipelineState.reset();
        m_ResourceBindingState.reset();
        m_DescriptorSetLayoutBindingState.clear();
        m_StoredPushConstants.clear(); */

        VkCommandBufferBeginInfo begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        VkCommandBufferInheritanceInfo inheritance = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
        begin_info.flags = flags;

        if (m_Level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
        {
            /* assert((m_RenderPass && m_Framebuffer) && "Render pass and framebuffer must be provided when calling begin from a secondary one");

            m_CurrentRenderPass.render_pass = m_RenderPass;
            m_CurrentRenderPass.framebuffer = m_Framebuffer;

            inheritance.renderPass = m_CurrentRenderPass.render_pass->get_handle();
            inheritance.framebuffer = m_CurrentRenderPass.framebuffer->get_handle();
            inheritance.subpass = m_SubpassIndex; */

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
}
