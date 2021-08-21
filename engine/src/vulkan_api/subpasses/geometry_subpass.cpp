#include "vulkan_api/subpasses/geometry_subpass.h"

#include "scene/scene.h"
#include "scene/components/mesh.h"
#include "scene/components/material.h"
#include "scene/components/submesh.h"
#include "scene/components/camera.h"
#include "vulkan_api/device.h"

namespace engine
{
    GeometrySubpass::GeometrySubpass(RenderContext &render_context, ShaderSource &&vertex_shader, ShaderSource &&fragment_shader, Scene &scene, sg::Camera &camera)
        : Subpass(render_context,
                  std::move(vertex_shader),
                  std::move(fragment_shader)),
          m_Scene(scene) // , m_Camera(camera)
    {
    }

    GeometrySubpass::~GeometrySubpass()
    {
    }

    void GeometrySubpass::Prepare()
    {
        auto &device = m_RenderContext.GetDevice();
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

    void GeometrySubpass::Draw(CommandBuffer &command_buffer)
    {
        std::multimap<float, std::pair<Entity *, sg::Submesh *>> opaque_nodes;
        std::multimap<float, std::pair<Entity *, sg::Submesh *>> transparent_nodes;

        GetSortedNodes(opaque_nodes, transparent_nodes);

        for (auto node_it = opaque_nodes.begin(); node_it != opaque_nodes.end(); node_it++)
        {
            auto transform = node_it->second.first->GetComponent<sg::Transform>();
            UpdateUniform(command_buffer, transform, m_ThreadIndex);

            // Invert the front face if the mesh was flipped
            const auto &scale = transform.GetScale();
            bool flipped = scale.x * scale.y * scale.z < 0;
            VkFrontFace front_face = flipped ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;

            DrawSubmesh(command_buffer, *node_it->second.second, front_face);
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
            auto transform = node_it->second.first->GetComponent<sg::Transform>();
            UpdateUniform(command_buffer, transform, m_ThreadIndex);

            DrawSubmesh(command_buffer, *node_it->second.second);
        }
    }

    void GeometrySubpass::GetSortedNodes(
        std::multimap<float, std::pair<Entity *, sg::Submesh *>> opaque_nodes,
        std::multimap<float, std::pair<Entity *, sg::Submesh *>> transparent_nodes)
    {
        // TODO:
        auto camera_transform = m_Scene.GetCameras().front()->GetComponent<sg::Transform>().GetWorldMatrix();

        auto view = m_Scene.GetRegistry().view<sg::Mesh, sg::Transform>();
        for (auto &entity : view)
        {
            auto [mesh, transform] = view.get<sg::Mesh, sg::Transform>(entity);

            auto world_matrix = transform.GetWorldMatrix();
            auto &mesh_bounds = mesh.GetBounds();

            sg::AABB world_bounds{mesh_bounds.GetMin(), mesh_bounds.GetMax()};
            world_bounds.Transform(world_matrix);

            float distance = glm::length(glm::vec3(camera_transform[3]) -
                                         world_bounds.GetCenter());

            for (auto &submesh : mesh.GetSubmeshes())
            {
                if (submesh->GetMaterial()->m_AlphaMode == sg::AlphaMode::Blend)
                    transparent_nodes.emplace(distance,
                                              std::make_pair(entity, submesh));

                else
                    opaque_nodes.emplace(distance,
                                         std::make_pair(entity, submesh));
            }
        }
    }

    void GeometrySubpass::UpdateUniform(CommandBuffer &command_buffer, sg::Transform &submesh_transform, size_t thread_index)
    {
        GlobalUniform global_uniform;
        auto camera = m_Scene.GetCameras().front()->GetComponent<sg::Camera>();
        auto camera_transform = m_Scene.GetCameras().front()->GetComponent<sg::Transform>();

        global_uniform.camera_view_proj = camera.m_PreRotation *
                                          VulkanStyleProjection(
                                              camera.GetProjection()) *
                                          camera_transform.GetWorldMatrix();

        auto &render_frame = m_RenderContext.GetActiveFrame();

        //TODO: thread index
        auto allocation = render_frame.AllocateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                      sizeof(GlobalUniform), thread_index);

        global_uniform.model = submesh_transform.GetWorldMatrix();
        global_uniform.camera_position = glm::vec3(glm::inverse(
            camera_transform.GetWorldMatrix())[3]);

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

        auto &vert_shader_module = device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_VERTEX_BIT, get_vertex_shader(), sub_mesh.get_shader_variant());
        auto &frag_shader_module = device.GetResourceCache().RequestShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT, get_fragment_shader(), sub_mesh.get_shader_variant());

        std::vector<ShaderModule *> shader_modules{&vert_shader_module, &frag_shader_module};

        auto &pipeline_layout = PreparePipelineLayout(command_buffer, shader_modules);

        command_buffer.GetPipelineState().SetPipelineLayout(pipeline_layout);

        if (pipeline_layout.GetPushConstantRangeStage(sizeof(PBRMaterialUniform)) != 0)
        {
            PreparePushConstants(command_buffer, sub_mesh);
        }

        DescriptorSetLayout &descriptor_set_layout = pipeline_layout.get_descriptor_set_layout(0);

        for (auto &texture : sub_mesh.get_material()->textures)
        {
            if (auto layout_binding = descriptor_set_layout.GetLayoutBinding(texture.first))
            {
                command_buffer.BindImage(texture.second->get_image()->get_vk_image_view(),
                                         texture.second->get_sampler()->vk_sampler,
                                         0, layout_binding->binding, 0);
            }
        }

        auto vertex_input_resources = pipeline_layout.GetResources(ShaderResourceType::Input, VK_SHADER_STAGE_VERTEX_BIT);

        VertexInputState vertex_input_state;

        for (auto &input_resource : vertex_input_resources)
        {
            sg::VertexAttribute attribute;

            if (!sub_mesh.get_attribute(input_resource.name, attribute))
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

        command_buffer.SetVertexInputState(vertex_input_state);

        // Find submesh vertex buffers matching the shader input attribute names
        for (auto &input_resource : vertex_input_resources)
        {
            const auto &buffer_iter = sub_mesh.vertex_buffers.find(input_resource.name);

            if (buffer_iter != sub_mesh.vertex_buffers.end())
            {
                std::vector<std::reference_wrapper<const core::Buffer>> buffers;
                buffers.emplace_back(std::ref(buffer_iter->second));

                // Bind vertex buffers only for the attribute locations defined
                command_buffer.BindVertexBuffers(input_resource.location, std::move(buffers), {0});
            }
        }

        DrawSubmeshCommand(command_buffer, sub_mesh);
    }
}
