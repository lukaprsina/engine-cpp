#include "core/gui.h"

#include "window/window.h"
#include "vulkan_api/device.h"
#include "core/application.h"
#include "vulkan_api/render_context.h"
#include "vulkan_api/core/pipeline_layout.h"
#include "renderer/shader.h"
#include "events/key_event.h"
#include "vulkan_api/core/image.h"
#include "vulkan_api/core/image_view.h"
#include "vulkan_api/initializers.h"
#include "vulkan_api/rendering/pipeline_state.h"
#include "vulkan_api/render_frame.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace engine
{
    namespace
    {
        void UploadDrawData(ImDrawData *draw_data, const uint8_t *vertex_data, const uint8_t *index_data)
        {
            ImDrawVert *vtx_dst = (ImDrawVert *)vertex_data;
            ImDrawIdx *idx_dst = (ImDrawIdx *)index_data;

            for (int n = 0; n < draw_data->CmdListsCount; n++)
            {
                const ImDrawList *cmd_list = draw_data->CmdLists[n];
                memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                vtx_dst += cmd_list->VtxBuffer.Size;
                idx_dst += cmd_list->IdxBuffer.Size;
            }
        }
    }

    const std::string Gui::default_font = "Roboto-Medium";

    Gui::Gui(Application &application,
             const Window &window,
             const float font_size,
             bool explicit_update)
        : m_Application(application), m_ExplicitUpdate(explicit_update)
    {
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();

        auto &extent = application.GetRenderContext().GetSurfaceExtent();
        io.DisplaySize.x = static_cast<float>(extent.width);
        io.DisplaySize.y = static_cast<float>(extent.height);
        io.FontGlobalScale = 1.0f;
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.KeyMap[ImGuiKey_Space] = static_cast<int>(Key::Space);
        io.KeyMap[ImGuiKey_Enter] = static_cast<int>(Key::Enter);
        io.KeyMap[ImGuiKey_LeftArrow] = static_cast<int>(Key::Left);
        io.KeyMap[ImGuiKey_RightArrow] = static_cast<int>(Key::Right);
        io.KeyMap[ImGuiKey_UpArrow] = static_cast<int>(Key::Up);
        io.KeyMap[ImGuiKey_DownArrow] = static_cast<int>(Key::Down);
        io.KeyMap[ImGuiKey_Tab] = static_cast<int>(Key::Tab);

        m_Fonts.emplace_back(default_font, font_size);

        unsigned char *font_data;
        int tex_width, tex_height;
        io.Fonts->GetTexDataAsRGBA32(&font_data, &tex_width, &tex_height);
        size_t upload_size = tex_width * tex_height * 4 * sizeof(char);

        auto &device = application.GetRenderContext().GetDevice();

        // Create target image for copy
        VkExtent3D font_extent{ToUint32_t(tex_width), ToUint32_t(tex_height), 1u};
        m_FontImage = std::make_unique<core::Image>(device, font_extent, VK_FORMAT_R8G8B8A8_UNORM,
                                                    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                    VMA_MEMORY_USAGE_GPU_ONLY);
        m_FontImageView = std::make_unique<core::ImageView>(*m_FontImage, VK_IMAGE_VIEW_TYPE_2D);

        {
            core::Buffer stage_buffer{device, upload_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, 0};
            stage_buffer.Update({font_data, font_data + upload_size});

            auto &command_buffer = device.RequestCommandBuffer();

            FencePool fence_pool{device};

            // Begin recording
            command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, 0);

            {
                // Prepare for transfer
                ImageMemoryBarrier memory_barrier{};
                memory_barrier.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
                memory_barrier.new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                memory_barrier.src_access_mask = 0;
                memory_barrier.dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
                memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_HOST_BIT;
                memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;

                command_buffer.CreateImageMemoryBarrier(*m_FontImageView, memory_barrier);
            }

            // Copy
            VkBufferImageCopy buffer_copy_region{};
            buffer_copy_region.imageSubresource.layerCount = m_FontImageView->GetSubresourceRange().layerCount;
            buffer_copy_region.imageSubresource.aspectMask = m_FontImageView->GetSubresourceRange().aspectMask;
            buffer_copy_region.imageExtent = m_FontImage->GetExtent();

            command_buffer.CopyBufferToImage(stage_buffer, *m_FontImage, {buffer_copy_region});

            {
                // Prepare for fragmen shader
                ImageMemoryBarrier memory_barrier{};
                memory_barrier.old_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                memory_barrier.new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                memory_barrier.src_access_mask = 0;
                memory_barrier.dst_access_mask = VK_ACCESS_SHADER_READ_BIT;
                memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

                command_buffer.CreateImageMemoryBarrier(*m_FontImageView, memory_barrier);
            }

            // End recording
            command_buffer.End();

            auto &queue = device.GetQueueFamilyByFlags(VK_QUEUE_GRAPHICS_BIT).GetQueues()[0];

            queue.Submit(command_buffer, device.RequestFence());

            // Wait for the command buffer to finish its work before destroying the staging buffer
            device.GetFencePool().Wait();
            device.GetFencePool().Reset();
            device.GetCommandPool().ResetPool();
        }

        // Create texture sampler
        VkSamplerCreateInfo sampler_info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        sampler_info.maxAnisotropy = 1.0f;
        sampler_info.magFilter = VK_FILTER_LINEAR;
        sampler_info.minFilter = VK_FILTER_LINEAR;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        ShaderSource vert_shader("imgui.vert");
        ShaderSource frag_shader("imgui.frag");

        std::vector<ShaderModule *> shader_modules;
        shader_modules.push_back(&device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_VERTEX_BIT, vert_shader, {}));
        shader_modules.push_back(&device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT, frag_shader, {}));

        m_PipelineLayout = &device.GetResourceCache().RequestPipelineLayout(shader_modules);

        m_Sampler = std::make_unique<core::Sampler>(device, sampler_info);

        if (explicit_update)
        {
            m_VertexBuffer = std::make_unique<core::Buffer>(application.GetRenderContext().GetDevice(), 1, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_TO_CPU);
            m_IndexBuffer = std::make_unique<core::Buffer>(application.GetRenderContext().GetDevice(), 1, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_TO_CPU);
        }
    }

    Gui::~Gui()
    {
        auto &device = m_Application.GetRenderContext().GetDevice();
        vkDestroyDescriptorPool(device.GetHandle(), m_DescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device.GetHandle(), m_DescriptorSetLayout, nullptr);
        vkDestroyPipeline(device.GetHandle(), m_Pipeline, nullptr);

        ImGui::DestroyContext();
    }

    void Gui::Prepare(const VkPipelineCache pipeline_cache, const VkRenderPass render_pass,
                      const std::vector<VkPipelineShaderStageCreateInfo> &shader_stages)
    {
        auto &device = m_Application.GetRenderContext().GetDevice();
        // Descriptor pool
        std::vector<VkDescriptorPoolSize> pool_sizes = {
            initializers::descriptor_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)};
        VkDescriptorPoolCreateInfo descriptorPoolInfo = initializers::descriptor_pool_create_info(pool_sizes, 2);
        VK_CHECK(vkCreateDescriptorPool(device.GetHandle(), &descriptorPoolInfo, nullptr, &m_DescriptorPool));

        // Descriptor set layout
        std::vector<VkDescriptorSetLayoutBinding> layout_bindings = {
            initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
        };
        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = initializers::descriptor_set_layout_create_info(layout_bindings);
        VK_CHECK(vkCreateDescriptorSetLayout(device.GetHandle(), &descriptor_set_layout_create_info, nullptr, &m_DescriptorSetLayout));

        // Descriptor set
        VkDescriptorSetAllocateInfo descriptor_allocation = initializers::descriptor_set_allocate_info(m_DescriptorPool, &m_DescriptorSetLayout, 1);
        VK_CHECK(vkAllocateDescriptorSets(device.GetHandle(), &descriptor_allocation, &m_DescriptorSet));
        VkDescriptorImageInfo font_descriptor = initializers::descriptor_image_info(
            m_Sampler->GetHandle(),
            m_FontImageView->GetHandle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        std::vector<VkWriteDescriptorSet> write_descriptor_sets = {
            initializers::write_descriptor_set(m_DescriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &font_descriptor)};
        vkUpdateDescriptorSets(device.GetHandle(), static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);

        // Setup graphics pipeline for UI rendering
        VkPipelineInputAssemblyStateCreateInfo input_assembly_state =
            initializers::pipeline_input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

        VkPipelineRasterizationStateCreateInfo rasterization_state =
            initializers::pipeline_rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

        // Enable blending
        VkPipelineColorBlendAttachmentState blend_attachment_state{};
        blend_attachment_state.blendEnable = VK_TRUE;
        blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
        blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo color_blend_state =
            initializers::pipeline_color_blend_state_create_info(1, &blend_attachment_state);

        VkPipelineDepthStencilStateCreateInfo depth_stencil_state =
            initializers::pipeline_depth_stencil_state_create_info(VK_FALSE, VK_FALSE, VK_COMPARE_OP_ALWAYS);

        VkPipelineViewportStateCreateInfo viewport_state =
            initializers::pipeline_viewport_state_create_info(1, 1, 0);

        VkPipelineMultisampleStateCreateInfo multisample_state =
            initializers::pipeline_multisample_state_create_info(VK_SAMPLE_COUNT_1_BIT);

        std::vector<VkDynamicState> dynamic_state_enables = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamic_state =
            initializers::pipeline_dynamic_state_create_info(dynamic_state_enables);

        VkGraphicsPipelineCreateInfo pipeline_create_info = initializers::pipeline_create_info(m_PipelineLayout->GetHandle(), render_pass);

        pipeline_create_info.pInputAssemblyState = &input_assembly_state;
        pipeline_create_info.pRasterizationState = &rasterization_state;
        pipeline_create_info.pColorBlendState = &color_blend_state;
        pipeline_create_info.pMultisampleState = &multisample_state;
        pipeline_create_info.pViewportState = &viewport_state;
        pipeline_create_info.pDepthStencilState = &depth_stencil_state;
        pipeline_create_info.pDynamicState = &dynamic_state;
        pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
        pipeline_create_info.pStages = shader_stages.data();
        pipeline_create_info.subpass = 0;

        // Vertex bindings an attributes based on ImGui vertex definition
        std::vector<VkVertexInputBindingDescription> vertex_input_bindings = {
            initializers::vertex_input_binding_description(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
        };
        std::vector<VkVertexInputAttributeDescription> vertex_input_attributes = {
            initializers::vertex_input_attribute_description(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),  // Location 0: Position
            initializers::vertex_input_attribute_description(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),   // Location 1: UV
            initializers::vertex_input_attribute_description(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)), // Location 0: Color
        };
        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = initializers::pipeline_vertex_input_state_create_info();
        vertex_input_state_create_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_input_bindings.size());
        vertex_input_state_create_info.pVertexBindingDescriptions = vertex_input_bindings.data();
        vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_input_attributes.size());
        vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_input_attributes.data();

        pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;

        VK_CHECK(vkCreateGraphicsPipelines(device.GetHandle(), pipeline_cache, 1, &pipeline_create_info, nullptr, &m_Pipeline));
    }

    void Gui::OnDraw(CommandBuffer &command_buffer)
    {
        // Vertex input state
        VkVertexInputBindingDescription vertex_input_binding{};
        vertex_input_binding.stride = ToUint32_t(sizeof(ImDrawVert));

        // Location 0: Position
        VkVertexInputAttributeDescription pos_attr{};
        pos_attr.format = VK_FORMAT_R32G32_SFLOAT;
        pos_attr.offset = ToUint32_t(offsetof(ImDrawVert, pos));

        // Location 1: UV
        VkVertexInputAttributeDescription uv_attr{};
        uv_attr.location = 1;
        uv_attr.format = VK_FORMAT_R32G32_SFLOAT;
        uv_attr.offset = ToUint32_t(offsetof(ImDrawVert, uv));

        // Location 2: Color
        VkVertexInputAttributeDescription col_attr{};
        col_attr.location = 2;
        col_attr.format = VK_FORMAT_R8G8B8A8_UNORM;
        col_attr.offset = ToUint32_t(offsetof(ImDrawVert, col));

        VertexInputState vertex_input_state{};
        vertex_input_state.bindings = {vertex_input_binding};
        vertex_input_state.attributes = {pos_attr, uv_attr, col_attr};

        command_buffer.GetPipelineState().SetVertexInputState(vertex_input_state);

        // Blend state
        ColorBlendAttachmentState color_attachment{};
        color_attachment.blend_enable = VK_TRUE;
        color_attachment.color_write_mask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
        color_attachment.src_color_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_attachment.dst_color_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_attachment.src_alpha_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

        ColorBlendState blend_state{};
        blend_state.attachments = {color_attachment};

        command_buffer.GetPipelineState().SetColorBlendState(blend_state);

        RasterizationState rasterization_state{};
        rasterization_state.cull_mode = VK_CULL_MODE_NONE;
        command_buffer.GetPipelineState().SetRasterizationState(rasterization_state);

        DepthStencilState depth_state{};
        depth_state.depth_test_enable = VK_FALSE;
        depth_state.depth_write_enable = VK_FALSE;
        command_buffer.GetPipelineState().SetDepthStencilState(depth_state);

        // Bind pipeline layout
        command_buffer.GetPipelineState().SetPipelineLayout(*m_PipelineLayout);

        command_buffer.GetResourceBindingState().BindImage(*m_FontImageView, *m_Sampler, 0, 0, 0);

        // Pre-rotation
        auto &io = ImGui::GetIO();
        auto push_transform = glm::mat4(1.0f);

        auto &swapchain = m_Application.GetRenderContext().GetSwapchain();

        if (swapchain != nullptr)
        {
            auto &transform = swapchain->GetProperties().pre_transform;

            glm::vec3 rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
            if (transform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR)
                push_transform = glm::rotate(push_transform, glm::radians(90.0f), rotation_axis);

            else if (transform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
                push_transform = glm::rotate(push_transform, glm::radians(270.0f), rotation_axis);

            else if (transform & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR)
                push_transform = glm::rotate(push_transform, glm::radians(180.0f), rotation_axis);
        }

        // GUI coordinate space to screen space
        push_transform = glm::translate(push_transform, glm::vec3(-1.0f, -1.0f, 0.0f));
        push_transform = glm::scale(push_transform, glm::vec3(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y, 0.0f));

        // Push constants
        command_buffer.PushConstants(push_transform);

        // If a render context is used, then use the frames buffer pools to allocate GUI vertex/index data from
        if (!m_ExplicitUpdate)
        {
            UpdateBuffers(command_buffer, m_Application.GetRenderContext().GetActiveFrame());
        }
        else
        {
            std::vector<std::reference_wrapper<const core::Buffer>> buffers;
            buffers.push_back(*m_VertexBuffer);
            command_buffer.BindVertexBuffers(0, buffers, {0});

            command_buffer.BindIndexBuffer(*m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
        }

        // Render commands
        ImDrawData *draw_data = ImGui::GetDrawData();
        int32_t vertex_offset = 0;
        uint32_t index_offset = 0;

        if (!draw_data || draw_data->CmdListsCount == 0)
        {
            return;
        }

        for (int32_t i = 0; i < draw_data->CmdListsCount; i++)
        {
            const ImDrawList *cmd_list = draw_data->CmdLists[i];
            for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
            {
                const ImDrawCmd *cmd = &cmd_list->CmdBuffer[j];
                VkRect2D scissor_rect;
                scissor_rect.offset.x = std::max(static_cast<int32_t>(cmd->ClipRect.x), 0);
                scissor_rect.offset.y = std::max(static_cast<int32_t>(cmd->ClipRect.y), 0);
                scissor_rect.extent.width = static_cast<uint32_t>(cmd->ClipRect.z - cmd->ClipRect.x);
                scissor_rect.extent.height = static_cast<uint32_t>(cmd->ClipRect.w - cmd->ClipRect.y);

                // Adjust for pre-rotation if necessary
                if (swapchain != nullptr)
                {
                    auto &transform = swapchain->GetProperties().pre_transform;
                    if (transform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR)
                    {
                        scissor_rect.offset.x = static_cast<uint32_t>(io.DisplaySize.y - cmd->ClipRect.w);
                        scissor_rect.offset.y = static_cast<uint32_t>(cmd->ClipRect.x);
                        scissor_rect.extent.width = static_cast<uint32_t>(cmd->ClipRect.w - cmd->ClipRect.y);
                        scissor_rect.extent.height = static_cast<uint32_t>(cmd->ClipRect.z - cmd->ClipRect.x);
                    }
                    else if (transform & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR)
                    {
                        scissor_rect.offset.x = static_cast<uint32_t>(io.DisplaySize.x - cmd->ClipRect.z);
                        scissor_rect.offset.y = static_cast<uint32_t>(io.DisplaySize.y - cmd->ClipRect.w);
                        scissor_rect.extent.width = static_cast<uint32_t>(cmd->ClipRect.z - cmd->ClipRect.x);
                        scissor_rect.extent.height = static_cast<uint32_t>(cmd->ClipRect.w - cmd->ClipRect.y);
                    }
                    else if (transform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
                    {
                        scissor_rect.offset.x = static_cast<uint32_t>(cmd->ClipRect.y);
                        scissor_rect.offset.y = static_cast<uint32_t>(io.DisplaySize.x - cmd->ClipRect.z);
                        scissor_rect.extent.width = static_cast<uint32_t>(cmd->ClipRect.w - cmd->ClipRect.y);
                        scissor_rect.extent.height = static_cast<uint32_t>(cmd->ClipRect.z - cmd->ClipRect.x);
                    }
                }

                command_buffer.SetScissor(0, {scissor_rect});
                command_buffer.DrawIndexed(cmd->ElemCount, 1, index_offset, vertex_offset, 0);
                index_offset += cmd->ElemCount;
            }
            vertex_offset += cmd_list->VtxBuffer.Size;
        }
    }

    /* void Gui::Draw(VkCommandBuffer command_buffer)
    {
        auto &io = ImGui::GetIO();
        ImDrawData *draw_data = ImGui::GetDrawData();
        int32_t vertex_offset = 0;
        int32_t index_offset = 0;

        if ((!draw_data) || (draw_data->CmdListsCount == 0))
        {
            return;
        }

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout->get_handle(), 0, 1, &descriptor_set, 0, NULL);

        // Push constants
        auto push_transform = glm::mat4(1.0f);
        push_transform = glm::translate(push_transform, glm::vec3(-1.0f, -1.0f, 0.0f));
        push_transform = glm::scale(push_transform, glm::vec3(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y, 0.0f));
        vkCmdPushConstants(command_buffer, pipeline_layout->get_handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &push_transform);

        VkDeviceSize offsets[1] = {0};

        VkBuffer vertex_buffer_handle = vertex_buffer->get_handle();
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer_handle, offsets);

        VkBuffer index_buffer_handle = index_buffer->get_handle();
        vkCmdBindIndexBuffer(command_buffer, index_buffer_handle, 0, VK_INDEX_TYPE_UINT16);

        for (int32_t i = 0; i < draw_data->CmdListsCount; i++)
        {
            const ImDrawList *cmd_list = draw_data->CmdLists[i];
            for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
            {
                const ImDrawCmd *cmd = &cmd_list->CmdBuffer[j];
                VkRect2D scissor_rect;
                scissor_rect.offset.x = std::max(static_cast<int32_t>(cmd->ClipRect.x), 0);
                scissor_rect.offset.y = std::max(static_cast<int32_t>(cmd->ClipRect.y), 0);
                scissor_rect.extent.width = static_cast<uint32_t>(cmd->ClipRect.z - cmd->ClipRect.x);
                scissor_rect.extent.height = static_cast<uint32_t>(cmd->ClipRect.w - cmd->ClipRect.y);

                vkCmdSetScissor(command_buffer, 0, 1, &scissor_rect);
                vkCmdDrawIndexed(command_buffer, cmd->ElemCount, 1, index_offset, vertex_offset, 0);
                index_offset += cmd->ElemCount;
            }
            vertex_offset += cmd_list->VtxBuffer.Size;
        }
    } */

    void Gui::UpdateBuffers(CommandBuffer &command_buffer, RenderFrame &render_frame)
    {
        ImDrawData *draw_data = ImGui::GetDrawData();

        if (!draw_data)
        {
            return;
        }

        size_t vertex_buffer_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
        size_t index_buffer_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);

        if ((vertex_buffer_size == 0) || (index_buffer_size == 0))
        {
            return;
        }

        std::vector<uint8_t> vertex_data(vertex_buffer_size);
        std::vector<uint8_t> index_data(index_buffer_size);

        UploadDrawData(draw_data, vertex_data.data(), index_data.data());

        // auto &render_frame = m_Application.GetRenderContext().GetActiveFrame();
        auto vertex_allocation = render_frame.AllocateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_buffer_size);

        vertex_allocation.Update(vertex_data);

        std::vector<std::reference_wrapper<const core::Buffer>> buffers;
        buffers.emplace_back(std::ref(vertex_allocation.GetBuffer()));

        std::vector<VkDeviceSize> offsets{vertex_allocation.GetOffset()};

        command_buffer.BindVertexBuffers(0, buffers, offsets);

        auto index_allocation = render_frame.AllocateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, index_buffer_size);

        index_allocation.Update(index_data);

        command_buffer.BindIndexBuffer(index_allocation.GetBuffer(), index_allocation.GetOffset(), VK_INDEX_TYPE_UINT16);
    }

    void Gui::OnUpdate(float delta_time)
    {
        NewFrame();
        ImGui::ShowDemoWindow();
        Update(delta_time);
    }

    void Gui::NewFrame()
    {
        ImGui::NewFrame();
    }

    void Gui::Update(float delta_time)
    {
        ImGuiIO &io = ImGui::GetIO();
        auto extent = m_Application.GetRenderContext().GetSurfaceExtent();
        Resize(extent.width, extent.height);
        io.DeltaTime = delta_time;
        ImGui::Render();
    }

    void Gui::Resize(const uint32_t width, const uint32_t height) const
    {
        auto &io = ImGui::GetIO();
        io.DisplaySize.x = static_cast<float>(width);
        io.DisplaySize.y = static_cast<float>(height);
    }
}
