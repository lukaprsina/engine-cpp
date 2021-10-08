#pragma once

#include "vulkan_api/subpasses/subpass.h"

namespace engine
{
    class RenderContext;
    class ShaderSource;
    class Scene;
    class Entity;

    namespace sg
    {
        class Camera;
        class Submesh;
        class Transform;
    }

    struct alignas(16) GlobalUniform
    {
        glm::mat4 model;
        glm::mat4 camera_view_proj;
        glm::vec3 camera_position;
    };

    struct PBRMaterialUniform
    {
        glm::vec4 base_color_factor;
        float metallic_factor;
        float roughness_factor;
    };

    class GeometrySubpass : public Subpass
    {
    public:
        GeometrySubpass(ShaderSource &&vertex_shader,
                        ShaderSource &&fragment_shader,
                        Scene &scene);
        virtual ~GeometrySubpass();

        virtual void Prepare(Device &device) override;
        virtual void Draw(RenderContext &render_context, Layer &layer, CommandBuffer &command_buffer) override;
        void GetSortedNodes(std::multimap<float, std::pair<sg::Submesh *, sg::Transform *>> &opaque_nodes,
                            std::multimap<float, std::pair<sg::Submesh *, sg::Transform *>> &transparent_nodes,
                            Entity *camera);

        void UpdateUniform(RenderContext &render_context, CommandBuffer &command_buffer, sg::Transform &transform, Entity *camera, size_t thread_index = 0);

        void DrawSubmesh(CommandBuffer &command_buffer,
                         sg::Submesh &sub_mesh,
                         VkFrontFace front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE);

        void PreparePipelineState(CommandBuffer &command_buffer, VkFrontFace front_face, bool double_sided_material);
        PipelineLayout &PreparePipelineLayout(CommandBuffer &command_buffer, const std::vector<ShaderModule *> &shader_modules);
        void PreparePushConstants(CommandBuffer &command_buffer, sg::Submesh &submesh);
        void DrawSubmeshCommand(CommandBuffer &command_buffer, sg::Submesh &submesh);

    protected:
        Scene &m_Scene;
        uint32_t m_ThreadIndex{0};
        RasterizationState m_BaseRasterizationState{};
    };
}
