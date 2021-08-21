#pragma once

#include "vulkan_api/subpasses/subpass.h"

namespace engine
{
    class RenderContext;
    class ShaderSource;
    class Scene;

    namespace sg
    {
        class Camera;
        class Submesh;
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
        GeometrySubpass(RenderContext &render_context,
                        ShaderSource &&vertex_shader,
                        ShaderSource &&fragment_shader,
                        Scene &scene, sg::Camera &camera);
        virtual ~GeometrySubpass();

        virtual void Prepare() override;
        virtual void Draw(CommandBuffer &command_buffer) override;
        void GetSortedNodes(std::multimap<float, std::pair<Entity *, sg::Submesh *>> opaque_nodes,
                            std::multimap<float, std::pair<Entity *, sg::Submesh *>> transparent_nodes);

        void UpdateUniform(CommandBuffer &command_buffer, sg::Transform &transform, size_t thread_index = 0);

        void DrawSubmesh(CommandBuffer &command_buffer,
                         sg::Submesh &sub_mesh,
                         VkFrontFace front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE);

        void PreparePipelineState(CommandBuffer &command_buffer, VkFrontFace front_face, bool double_sided_material);
        PipelineLayout &PreparePipelineLayout(CommandBuffer &command_buffer, const std::vector<ShaderModule *> &shader_modules);
        void PreparePushConstants(CommandBuffer &command_buffer, sg::Submesh &submesh);
        void DrawSubmeshCommand(CommandBuffer &command_buffer, sg::Submesh &submesh);

    protected:
        Scene &m_Scene;
        // sg::Camera &m_Camera;
        uint32_t m_ThreadIndex{0};
        RasterizationState m_BaseRasterizationState{};
    };
}
