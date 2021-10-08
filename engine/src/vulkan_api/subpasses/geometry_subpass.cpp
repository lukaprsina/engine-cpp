#include "vulkan_api/subpasses/geometry_subpass.h"

#include "scene/scene.h"
#include "scene/components/mesh.h"
#include "scene/components/image.h"
#include "scene/components/sampler.h"
#include "scene/components/texture.h"
#include "scene/components/pbr_material.h"
#include "scene/components/submesh.h"
#include "core/layer.h"
#include "scene/components/perspective_camera.h"
#include "vulkan_api/device.h"

namespace engine
{
    GeometrySubpass::GeometrySubpass(ShaderSource &&vertex_shader, ShaderSource &&fragment_shader, Scene &scene)
        : Subpass(std::move(vertex_shader),
                  std::move(fragment_shader)),
          m_Scene(scene)
    {
    }

    GeometrySubpass::~GeometrySubpass()
    {
    }

    void GeometrySubpass::Prepare(Device &device)
    {

        auto view = m_Scene.GetRegistry().view<sg::Mesh>();

        for (auto &entity : view)
        {
            auto &mesh = view.get<sg::Mesh>(entity);

            for (auto &submesh : mesh.GetSubmeshes())
            {
                auto &variant = submesh->GetMutShaderVariant();

                device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_VERTEX_BIT, m_VertexShader, variant);
                device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT, m_FragmentShader, variant);
            }
        }
    }

    void GeometrySubpass::Draw(RenderContext &render_context, Layer &layer, CommandBuffer &command_buffer)
    {
        std::multimap<float, std::pair<sg::Submesh *, sg::Transform *>> opaque_nodes;
        std::multimap<float, std::pair<sg::Submesh *, sg::Transform *>> transparent_nodes;

        GetSortedNodes(opaque_nodes, transparent_nodes, layer.GetCamera());

        for (auto node_it = opaque_nodes.begin(); node_it != opaque_nodes.end(); node_it++)
        {
            auto &transform = *(node_it->second.second);
            UpdateUniform(render_context, command_buffer, transform, layer.GetCamera(), m_ThreadIndex);

            // Invert the front face if the mesh was flipped
            const auto &scale = transform.GetScale();
            bool flipped = scale.x * scale.y * scale.z < 0;
            VkFrontFace front_face = flipped ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;

            DrawSubmesh(command_buffer, *node_it->second.first, front_face);
        }

        // Enable alpha blending
        ColorBlendAttachmentState color_blend_attachment{};
        color_blend_attachment.blend_enable = VK_TRUE;
        color_blend_attachment.src_color_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment.dst_color_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment.src_alpha_blend_factor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

        ColorBlendState color_blend_state{};
        color_blend_state.attachments.resize(GetOutputAttachments().size());

        for (auto &it : color_blend_state.attachments)
        {
            it = color_blend_attachment;
        }

        command_buffer.GetPipelineState().SetColorBlendState(color_blend_state);

        command_buffer.GetPipelineState().SetDepthStencilState(GetDepthStencilState());

        // Draw transparent objects in back-to-front order
        for (auto node_it = transparent_nodes.rbegin(); node_it != transparent_nodes.rend(); node_it++)
        {
            auto &transform = *(node_it->second.second);
            UpdateUniform(render_context, command_buffer, transform, layer.GetCamera(), m_ThreadIndex);

            DrawSubmesh(command_buffer, *node_it->second.first);
        }
    }

    void GeometrySubpass::GetSortedNodes(
        std::multimap<float, std::pair<sg::Submesh *, sg::Transform *>> &opaque_nodes,
        std::multimap<float, std::pair<sg::Submesh *, sg::Transform *>> &transparent_nodes,
        Entity *camera)
    {
        auto camera_matrix = camera->GetComponent<sg::Transform>().GetWorldMatrix();

        auto view = m_Scene.GetRegistry().view<sg::Mesh, sg::Transform>();
        for (auto &entity : view)
        {
            auto [mesh, transform] = view.get<sg::Mesh, sg::Transform>(entity);

            auto world_matrix = transform.GetWorldMatrix();
            auto &mesh_bounds = mesh.GetBounds();

            sg::AABB world_bounds{mesh_bounds.GetMin(), mesh_bounds.GetMax()};
            world_bounds.Transform(world_matrix);

            float distance = glm::length(glm::vec3(camera_matrix[3]) -
                                         world_bounds.GetCenter());

            for (auto &submesh : mesh.GetSubmeshes())
            {
                auto pair = std::make_pair(submesh, &transform);
                if (submesh->GetMaterial()->m_AlphaMode == sg::AlphaMode::Blend)
                    transparent_nodes.emplace(distance, pair);

                else
                    opaque_nodes.emplace(distance, pair);
            }
        }
    }

    void GeometrySubpass::UpdateUniform(RenderContext &render_context, CommandBuffer &command_buffer, sg::Transform &submesh_transform, Entity *camera, size_t thread_index)
    {
        GlobalUniform global_uniform;
        auto &perspective_camera = camera->GetComponent<sg::PerspectiveCamera>();
        auto &camera_transform = camera->GetComponent<sg::Transform>();

        global_uniform.camera_view_proj = perspective_camera.m_PreRotation *
                                          VulkanStyleProjection(perspective_camera.GetProjection()) *
                                          glm::inverse(camera_transform.GetWorldMatrix());

        auto &render_frame = render_context.GetActiveFrame();

        //TODO: thread index
        auto allocation = render_frame.AllocateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                      sizeof(GlobalUniform), thread_index);

        global_uniform.model = submesh_transform.GetWorldMatrix();
        global_uniform.camera_position = glm::vec3(
            camera_transform.GetWorldMatrix()[3]);

        allocation.Update(global_uniform);
        command_buffer.BindBuffer(allocation.GetBuffer(), allocation.GetOffset(), allocation.GetSize(), 0, 1, 0);
    }

    void GeometrySubpass::DrawSubmesh(CommandBuffer &command_buffer, sg::Submesh &submesh, VkFrontFace front_face)
    {
        auto &device = command_buffer.GetDevice();
        PreparePipelineState(command_buffer, front_face, submesh.GetMaterial()->m_DoubleSided);

        MultisampleState multisample_state{};
        multisample_state.rasterization_samples = m_SampleCount;
        command_buffer.GetPipelineState().SetMultisampleState(multisample_state);

        auto &vert_shader_module = device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_VERTEX_BIT, m_VertexShader, submesh.GetShaderVariant());
        auto &frag_shader_module = device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT, m_FragmentShader, submesh.GetShaderVariant());

        std::vector<ShaderModule *> shader_modules{&vert_shader_module, &frag_shader_module};

        auto &pipeline_layout = PreparePipelineLayout(command_buffer, shader_modules);

        command_buffer.GetPipelineState().SetPipelineLayout(pipeline_layout);

        if (pipeline_layout.GetPushConstantRangeStage(sizeof(PBRMaterialUniform)) != 0)
            PreparePushConstants(command_buffer, submesh);

        DescriptorSetLayout &descriptor_set_layout = pipeline_layout.GetDescriptorSetLayout(0);

        for (auto &texture : submesh.GetMaterial()->m_Textures)
        {
            if (auto layout_binding = descriptor_set_layout.GetLayoutBinding(texture.first))
            {
                command_buffer.GetResourceBindingState().BindImage(
                    texture.second->GetImage()->GetVkImageView(),
                    texture.second->GetSampler()->m_VkSampler,
                    0, layout_binding->binding, 0);
            }
        }

        auto vertex_input_resources = pipeline_layout.GetResources(ShaderResourceType::Input, VK_SHADER_STAGE_VERTEX_BIT);

        VertexInputState vertex_input_state;

        for (auto &input_resource : vertex_input_resources)
        {
            sg::VertexAttribute attribute;

            if (!submesh.GetAttribute(input_resource.name, attribute))
            {
                continue;
            }

            VkVertexInputAttributeDescription vertex_attribute{};
            vertex_attribute.binding = input_resource.location;
            vertex_attribute.format = attribute.format;
            vertex_attribute.location = input_resource.location;
            vertex_attribute.offset = attribute.offset;

            vertex_input_state.attributes.push_back(vertex_attribute);

            VkVertexInputBindingDescription vertex_binding{};
            vertex_binding.binding = input_resource.location;
            vertex_binding.stride = attribute.stride;

            vertex_input_state.bindings.push_back(vertex_binding);
        }

        command_buffer.GetPipelineState().SetVertexInputState(vertex_input_state);

        // Find submesh vertex buffers matching the shader input attribute names
        for (auto &input_resource : vertex_input_resources)
        {
            const auto &buffer_iter = submesh.m_VertexBuffers.find(input_resource.name);

            if (buffer_iter != submesh.m_VertexBuffers.end())
            {
                std::vector<std::reference_wrapper<const core::Buffer>> buffers;
                buffers.emplace_back(std::ref(buffer_iter->second));

                // Bind vertex buffers only for the attribute locations defined
                command_buffer.BindVertexBuffers(input_resource.location, std::move(buffers), {0});
            }
        }

        DrawSubmeshCommand(command_buffer, submesh);
    }

    void GeometrySubpass::PreparePipelineState(CommandBuffer &command_buffer, VkFrontFace front_face, bool double_sided_material)
    {
        RasterizationState rasterization_state = m_BaseRasterizationState;
        rasterization_state.front_face = front_face;

        if (double_sided_material)
        {
            rasterization_state.cull_mode = VK_CULL_MODE_NONE;
        }

        command_buffer.GetPipelineState().SetRasterizationState(rasterization_state);

        MultisampleState multisample_state{};
        multisample_state.rasterization_samples = m_SampleCount;
        command_buffer.GetPipelineState().SetMultisampleState(multisample_state);
    }

    PipelineLayout &GeometrySubpass::PreparePipelineLayout(CommandBuffer &command_buffer, const std::vector<ShaderModule *> &shader_modules)
    {
        // Sets any specified resource modes
        for (auto &shader_module : shader_modules)
        {
            for (auto &resource_mode : m_ResourceModeMap)
            {
                shader_module->SetResourceMode(resource_mode.first, resource_mode.second);
            }
        }

        return command_buffer.GetDevice().GetResourceCache().RequestPipelineLayout(shader_modules);
    }

    void GeometrySubpass::PreparePushConstants(CommandBuffer &command_buffer, sg::Submesh &submesh)
    {
        auto pbr_material = dynamic_cast<const sg::PBRMaterial *>(submesh.GetMaterial());

        PBRMaterialUniform pbr_material_uniform{};
        pbr_material_uniform.base_color_factor = pbr_material->m_BaseColorFactor;
        pbr_material_uniform.metallic_factor = pbr_material->m_MetallicFactor;
        pbr_material_uniform.roughness_factor = pbr_material->m_RoughnessFactor;

        auto data = ToBytes(pbr_material_uniform);

        if (!data.empty())
        {
            command_buffer.PushConstants(data);
        }
    }

    void GeometrySubpass::DrawSubmeshCommand(CommandBuffer &command_buffer, sg::Submesh &submesh)
    {
        // Draw submesh indexed if indices exists
        if (submesh.m_VertexIndices != 0)
        {
            // Bind index buffer of submesh
            command_buffer.BindIndexBuffer(*submesh.m_IndexBuffer, submesh.m_IndexOffset, submesh.m_IndexType);

            // Draw submesh using indexed data
            command_buffer.DrawIndexed(submesh.m_VertexIndices, 1, 0, 0, 0);
        }
        else
        {
            // Draw submesh using vertices only
            command_buffer.Draw(submesh.m_VerticesCount, 1, 0, 0);
        }
    }
}
