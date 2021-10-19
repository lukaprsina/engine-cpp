#include "vulkan_api/command_buffer.h"

#include "vulkan_api/command_pool.h"
#include "vulkan_api/subpasses/subpass.h"
#include "vulkan_api/rendering/render_pipeline.h"
#include "vulkan_api/device.h"
#include "vulkan_api/render_target.h"
#include "vulkan_api/core/framebuffer.h"
#include "vulkan_api/core/image_view.h"
#include "vulkan_api/core/render_pass.h"
#include "vulkan_api/subpasses/subpass.h"

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
            ENG_ASSERT(primary_cmd_buf, "A primary command buffer pointer must be provided when calling begin from a secondary one");
            auto render_pass_binding = primary_cmd_buf->GetCurrentRenderPass();

            return Begin(flags, render_pass_binding.render_pass, render_pass_binding.framebuffer, primary_cmd_buf->GetCurrentSubpassIndex());
        }

        return Begin(flags, nullptr, nullptr, 0);
    }

    VkResult CommandBuffer::Begin(VkCommandBufferUsageFlags flags, const RenderPass *render_pass, const Framebuffer *framebuffer, uint32_t subpass_index)
    {
        ENG_ASSERT(!IsRecording(), "Command buffer is already recording, please call end before beginning again");

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
            ENG_ASSERT((render_pass && framebuffer), "Render pass and framebuffer must be provided when calling begin from a secondary one");

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
        ENG_ASSERT(IsRecording(), "Command buffer is not recording, please call begin before end");

        if (!IsRecording())
        {
            return VK_NOT_READY;
        }

        vkEndCommandBuffer(m_Handle);

        m_State = State::Executable;

        return VK_SUCCESS;
    }

    void CommandBuffer::Flush(VkPipelineBindPoint pipeline_bind_point)
    {
        FlushPipelineState(pipeline_bind_point);

        FlushPushConstants();

        FlushDescriptorState(pipeline_bind_point);
    }

    void CommandBuffer::FlushPipelineState(VkPipelineBindPoint pipeline_bind_point)
    {
        // Create a new pipeline only if the graphics state changed
        if (!m_PipelineState.IsDirty())
        {
            return;
        }

        m_PipelineState.ClearDirty();

        // Create and bind pipeline
        if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS)
        {
            m_PipelineState.SetRenderPass(*m_CurrentRenderPass.render_pass);
            auto &pipeline = GetDevice().GetResourceCache().RequestGraphicsPipeline(m_PipelineState);

            vkCmdBindPipeline(m_Handle,
                              pipeline_bind_point,
                              pipeline.GetHandle());
        }
        else if (pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE)
        {
            auto &pipeline = GetDevice().GetResourceCache().RequestComputePipeline(m_PipelineState);

            vkCmdBindPipeline(m_Handle,
                              pipeline_bind_point,
                              pipeline.GetHandle());
        }
        else
        {
            throw "Only graphics and compute pipeline bind points are supported now";
        }
    }

    void CommandBuffer::FlushDescriptorState(VkPipelineBindPoint pipeline_bind_point)
    {
        ENG_ASSERT(m_CommandPool.GetRenderFrame(), "The command pool must be associated to a render frame");

        const auto &pipeline_layout = m_PipelineState.GetPipelineLayout();

        std::unordered_set<uint32_t> update_descriptor_sets;

        // Iterate over the shader sets to check if they have already been bound
        // If they have, add the set so that the command buffer later updates it
        for (auto &set_it : pipeline_layout.GetShaderSets())
        {
            uint32_t descriptor_set_id = set_it.first;

            auto descriptor_set_layout_it = m_DescriptorSetLayoutBindingState.find(descriptor_set_id);

            if (descriptor_set_layout_it != m_DescriptorSetLayoutBindingState.end())
            {
                if (descriptor_set_layout_it->second->GetHandle() != pipeline_layout.GetDescriptorSetLayout(descriptor_set_id).GetHandle())
                {
                    update_descriptor_sets.emplace(descriptor_set_id);
                }
            }
        }

        // Validate that the bound descriptor set layouts exist in the pipeline layout
        for (auto set_it = m_DescriptorSetLayoutBindingState.begin(); set_it != m_DescriptorSetLayoutBindingState.end();)
        {
            if (!pipeline_layout.HasDescriptorSetLayout(set_it->first))
            {
                set_it = m_DescriptorSetLayoutBindingState.erase(set_it);
            }
            else
            {
                ++set_it;
            }
        }

        // Check if a descriptor set needs to be created
        if (m_ResourceBindingState.IsDirty() || !update_descriptor_sets.empty())
        {
            m_ResourceBindingState.ClearDirty();

            // Iterate over all of the resource sets bound by the command buffer
            for (auto &resource_set_it : m_ResourceBindingState.GetResourceSets())
            {
                uint32_t descriptor_set_id = resource_set_it.first;
                auto &resource_set = resource_set_it.second;

                // Don't update resource set if it's not in the update list OR its state hasn't changed
                if (!resource_set.IsDirty() && (update_descriptor_sets.find(descriptor_set_id) == update_descriptor_sets.end()))
                {
                    continue;
                }

                // Clear dirty flag for resource set
                m_ResourceBindingState.ClearDirty(descriptor_set_id);

                // Skip resource set if a descriptor set layout doesn't exist for it
                if (!pipeline_layout.HasDescriptorSetLayout(descriptor_set_id))
                {
                    continue;
                }

                auto &descriptor_set_layout = pipeline_layout.GetDescriptorSetLayout(descriptor_set_id);

                // Make descriptor set layout bound for current set
                m_DescriptorSetLayoutBindingState[descriptor_set_id] = &descriptor_set_layout;

                BindingMap<VkDescriptorBufferInfo> buffer_infos;
                BindingMap<VkDescriptorImageInfo> image_infos;

                std::vector<uint32_t> dynamic_offsets;

                // The bindings we want to update before binding, if empty we update all bindings
                std::vector<uint32_t> bindings_to_update;

                // Iterate over all resource bindings
                for (auto &binding_it : resource_set.GetResourceBindings())
                {
                    auto binding_index = binding_it.first;
                    auto &binding_resources = binding_it.second;

                    // Check if binding exists in the pipeline layout
                    if (auto binding_info = descriptor_set_layout.GetLayoutBinding(binding_index))
                    {
                        // If update after bind is enabled, we store the binding index of each binding that need to be updated before being bound
                        if (m_UpdateAfterBind && !(descriptor_set_layout.GetLayoutBindingFlag(binding_index) & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT))
                        {
                            bindings_to_update.push_back(binding_index);
                        }

                        // Iterate over all binding resources
                        for (auto &element_it : binding_resources)
                        {
                            auto array_element = element_it.first;
                            auto &resource_info = element_it.second;

                            // Pointer references
                            auto &buffer = resource_info.buffer;
                            auto &sampler = resource_info.sampler;
                            auto &image_view = resource_info.image_view;

                            // Get buffer info
                            if (buffer != nullptr && IsBufferDescriptorType(binding_info->descriptorType))
                            {
                                VkDescriptorBufferInfo buffer_info{};

                                buffer_info.buffer = resource_info.buffer->GetHandle();
                                buffer_info.offset = resource_info.offset;
                                buffer_info.range = resource_info.range;

                                if (IsDynamicBufferDescriptorType(binding_info->descriptorType))
                                {
                                    dynamic_offsets.push_back(ToUint32_t(buffer_info.offset));

                                    buffer_info.offset = 0;
                                }

                                buffer_infos[binding_index][array_element] = std::move(buffer_info);
                            }

                            // Get image info
                            else if (image_view != nullptr || sampler != VK_NULL_HANDLE)
                            {
                                // Can be null for input attachments
                                VkDescriptorImageInfo image_info{};
                                image_info.sampler = sampler ? sampler->GetHandle() : VK_NULL_HANDLE;
                                image_info.imageView = image_view->GetHandle();

                                if (image_view != nullptr)
                                {
                                    // Add image layout info based on descriptor type
                                    switch (binding_info->descriptorType)
                                    {
                                    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                                        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                                        break;
                                    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                                        if (IsDepthStencilFormat(image_view->GetFormat()))
                                        {
                                            image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                                        }
                                        else
                                        {
                                            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                                        }
                                        break;
                                    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                                        image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                                        break;

                                    default:
                                        continue;
                                    }
                                }

                                image_infos[binding_index][array_element] = std::move(image_info);
                            }
                        }
                    }
                }

                // Request a descriptor set from the render frame, and write the buffer infos and image infos of all the specified bindings
                auto &descriptor_set = m_CommandPool.GetRenderFrame()->RequestDescriptorSet(descriptor_set_layout, buffer_infos, image_infos, m_CommandPool.GetThreadIndex());
                descriptor_set.Update(bindings_to_update);

                VkDescriptorSet descriptor_set_handle = descriptor_set.GetHandle();

                // Bind descriptor set
                vkCmdBindDescriptorSets(m_Handle,
                                        pipeline_bind_point,
                                        pipeline_layout.GetHandle(),
                                        descriptor_set_id,
                                        1, &descriptor_set_handle,
                                        ToUint32_t(dynamic_offsets.size()),
                                        dynamic_offsets.data());
            }
        }
    }

    void CommandBuffer::FlushPushConstants()
    {
        if (m_StoredPushConstants.empty())
        {
            return;
        }

        const PipelineLayout &pipeline_layout = m_PipelineState.GetPipelineLayout();

        VkShaderStageFlags shader_stage = pipeline_layout.GetPushConstantRangeStage(ToUint32_t(m_StoredPushConstants.size()));

        if (shader_stage)
        {
            vkCmdPushConstants(m_Handle, pipeline_layout.GetHandle(), shader_stage, 0, ToUint32_t(m_StoredPushConstants.size()), m_StoredPushConstants.data());
        }
        else
        {
            ENG_CORE_WARN("Push constant range [{}, {}] not found", 0, m_StoredPushConstants.size());
        }

        m_StoredPushConstants.clear();
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
        ENG_ASSERT(subpasses.size() > 0, "Cannot create a render pass without any subpass");

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

    void CommandBuffer::PushConstants(const std::vector<uint8_t> &values)
    {
        uint32_t push_constant_size = ToUint32_t(m_StoredPushConstants.size() + values.size());

        if (push_constant_size > m_MaxPushConstantsSize)
        {
            ENG_CORE_ERROR("Push constant limit of {} exceeded (pushing {} bytes for a total of {} bytes)", m_MaxPushConstantsSize, values.size(), push_constant_size);
            throw std::runtime_error("Push constant limit exceeded.");
        }
        else
        {
            m_StoredPushConstants.insert(m_StoredPushConstants.end(), values.begin(), values.end());
        }
    }

    void CommandBuffer::BindBuffer(const core::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t set, uint32_t binding, uint32_t array_element)
    {
        m_ResourceBindingState.BindBuffer(buffer, offset, range, set,
                                          binding, array_element);
    }

    void CommandBuffer::BindLighting(LightingState &lighting_state, uint32_t set, uint32_t binding)
    {
        auto &buffer = lighting_state.light_buffer;
        BindBuffer(buffer.GetBuffer(), buffer.GetOffset(),
                   buffer.GetSize(), set, binding, 0);

        SetSpecializationConstant(0, ToUint32_t(lighting_state.directional_lights.size()));
        SetSpecializationConstant(1, ToUint32_t(lighting_state.point_lights.size()));
        SetSpecializationConstant(2, ToUint32_t(lighting_state.spot_lights.size()));
    }

    void CommandBuffer::BindVertexBuffers(uint32_t first_binding,
                                          const std::vector<std::reference_wrapper<const core::Buffer>> &buffers,
                                          const std::vector<VkDeviceSize> &offsets)
    {
        std::vector<VkBuffer> buffer_handles(buffers.size(), VK_NULL_HANDLE);
        std::transform(buffers.begin(), buffers.end(), buffer_handles.begin(),
                       [](const core::Buffer &buffer)
                       { return buffer.GetHandle(); });
        vkCmdBindVertexBuffers(m_Handle, first_binding, ToUint32_t(buffer_handles.size()), buffer_handles.data(), offsets.data());
    }

    void CommandBuffer::BindIndexBuffer(const core::Buffer &buffer, VkDeviceSize offset, VkIndexType index_type)
    {
        vkCmdBindIndexBuffer(m_Handle, buffer.GetHandle(), offset, index_type);
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

        ENG_ASSERT(reset_mode == m_CommandPool.GetResetMode(), "Command buffer reset mode must match the one used by the pool to allocate it");

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

    void CommandBuffer::CopyBufferToImage(const core::Buffer &buffer, const core::Image &image, const std::vector<VkBufferImageCopy> &regions)
    {
        vkCmdCopyBufferToImage(m_Handle, buffer.GetHandle(),
                               image.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               ToUint32_t(regions.size()), regions.data());
    }

    void CommandBuffer::EndRenderPass()
    {
        vkCmdEndRenderPass(m_Handle);
    }

    void CommandBuffer::Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
    {
        Flush(VK_PIPELINE_BIND_POINT_GRAPHICS);
        vkCmdDraw(m_Handle, vertex_count, instance_count, first_vertex, first_instance);
    }

    void CommandBuffer::DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance)
    {
        Flush(VK_PIPELINE_BIND_POINT_GRAPHICS);
        vkCmdDrawIndexed(m_Handle, index_count, instance_count, first_index, vertex_offset, first_instance);
    }

    void CommandBuffer::SetSpecializationConstant(uint32_t constant_id, const std::vector<uint8_t> &data)
    {
        m_PipelineState.SetSpecializationConstant(constant_id, data);
    }

    Device &CommandBuffer::GetDevice() const
    {
        return m_CommandPool.GetDevice();
    }
}
