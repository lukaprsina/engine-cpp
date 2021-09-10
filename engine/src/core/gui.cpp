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

#include <imgui.h>
#include <imgui_internal.h>

namespace engine
{
    const std::string Gui::default_font = "Roboto-Regular";

    Gui::Gui(Application &application,
             const Window &window,
             const float font_size,
             bool explicit_update)
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
    }

    void Gui::Prepare(const VkPipelineCache pipeline_cache, const VkRenderPass render_pass,
                      const std::vector<VkPipelineShaderStageCreateInfo> &shader_stages)
    {
        // Descriptor pool
        std::vector<VkDescriptorPoolSize> pool_sizes = {
            initializers::descriptor_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)};
        VkDescriptorPoolCreateInfo descriptorPoolInfo = initializers::descriptor_pool_create_info(pool_sizes, 2);
        VK_CHECK(vkCreateDescriptorPool(sample.get_render_context().get_device().get_handle(), &descriptorPoolInfo, nullptr, &descriptor_pool));

        // Descriptor set layout
        std::vector<VkDescriptorSetLayoutBinding> layout_bindings = {
            initializers::descriptor_set_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
        };
        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = initializers::descriptor_set_layout_create_info(layout_bindings);
        VK_CHECK(vkCreateDescriptorSetLayout(sample.get_render_context().get_device().get_handle(), &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout));

        // Descriptor set
        VkDescriptorSetAllocateInfo descriptor_allocation = initializers::descriptor_set_allocate_info(descriptor_pool, &descriptor_set_layout, 1);
        VK_CHECK(vkAllocateDescriptorSets(sample.get_render_context().get_device().get_handle(), &descriptor_allocation, &descriptor_set));
        VkDescriptorImageInfo font_descriptor = initializers::descriptor_image_info(
            sampler->get_handle(),
            font_image_view->get_handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        std::vector<VkWriteDescriptorSet> write_descriptor_sets = {
            initializers::write_descriptor_set(descriptor_set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &font_descriptor)};
        vkUpdateDescriptorSets(sample.get_render_context().get_device().get_handle(), static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);

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

        VkGraphicsPipelineCreateInfo pipeline_create_info = initializers::pipeline_create_info(pipeline_layout->get_handle(), render_pass);

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

        VK_CHECK(vkCreateGraphicsPipelines(sample.get_render_context().get_device().get_handle(), pipeline_cache, 1, &pipeline_create_info, nullptr, &pipeline));
    }
}
