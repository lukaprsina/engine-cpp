#pragma once

#include "renderer/shader.h"
#include "vulkan_api/rendering/render_pipeline.h"
#include "vulkan_api/rendering/pipeline_state.h"
#include "scene/components/light.h"
#include "scene/components/transform.h"
#include "vulkan_api/core/buffer_pool.h"
#include "vulkan_api/render_context.h"
#include "renderer/shader.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "common/glm.h"

namespace engine
{
    class CommandBuffer;
    class ShaderSource;
    class RenderContext;
    class RenderTarget;
    class Entity;
    class Device;
    class Layer;

    struct alignas(16) LightInfo
    {
        glm::vec4 position;
        glm::vec4 color;
        glm::vec4 direction;
        glm::vec2 info;
    };

    struct LightingState
    {
        std::vector<LightInfo> directional_lights;
        std::vector<LightInfo> point_lights;
        std::vector<LightInfo> spot_lights;
        BufferAllocation light_buffer;
    };

    extern const std::vector<std::string> light_type_definitions;

    glm::mat4 VulkanStyleProjection(const glm::mat4 &proj);

    class Subpass
    {
    public:
        Subpass(ShaderSource &&vertex_shader,
                ShaderSource &&fragment_shader);
        virtual ~Subpass();

        virtual void Prepare(Device &device) = 0;
        virtual void Draw(RenderContext &render_context, Layer &layer, CommandBuffer &command_buffer) = 0;
        void UpdateRenderTargetAttachments(RenderTarget &render_target);

        const ShaderSource &GetVertexShader() const { return m_VertexShader; }
        const ShaderSource &GetFragmentShader() const { return m_FragmentShader; }
        DepthStencilState &GetDepthStencilState() { return m_DepthStencilState; }
        const std::vector<uint32_t> &GetInputAttachments() const { return m_InputAttachments; }
        void SetInputAttachments(std::vector<uint32_t> input) { m_InputAttachments = input; }
        const std::vector<uint32_t> &GetOutputAttachments() const { return m_OutputAttachments; }
        void SetOutputAttachments(std::vector<uint32_t> output) { m_OutputAttachments = output; }
        const std::vector<uint32_t> &GetColorResolveAttachments() const { return m_ColorResolveAttachments; }
        void SetColorResolveAttachments(std::vector<uint32_t> color_resolve) { m_ColorResolveAttachments = color_resolve; }
        const bool &GetDisableDepthStencilAttachment() const { return m_DisableDepthStencilAttachment; }
        void SetDisableDepthStencilAttachment(bool disable_depth_stencil) { m_DisableDepthStencilAttachment = disable_depth_stencil; }
        const uint32_t &GetDepthStencilResolveAttachment() const { return m_DepthStencilResolveAttachment; }
        void SetDepthStencilResolveAttachment(uint32_t depth_stencil_resolve) { m_DepthStencilResolveAttachment = depth_stencil_resolve; }
        const VkResolveModeFlagBits GetDepthStencilResolveMode() const { return m_DepthStencilResolveMode; }
        void SetDepthStencilResolveMode(VkResolveModeFlagBits mode) { m_DepthStencilResolveMode = mode; }
        void SetSampleCount(VkSampleCountFlagBits sample_count) { m_SampleCount = sample_count; }
        LightingState &GetLightingState() { return m_LightingState; }

        template <typename T>
        void AllocateLights(RenderContext &render_context, Scene &scene, size_t light_count)
        {
            auto view = scene.GetRegistry().view<sg::Light, sg::Transform>();

            // TODO: size hint
            // ENG_ASSERT(view.s() <= (light_count * sg::LightType::Max), "Exceeding Max Light Capacity");

            m_LightingState.directional_lights.clear();
            m_LightingState.point_lights.clear();
            m_LightingState.spot_lights.clear();

            for (auto &entity : view)
            {
                auto [scene_light, transform] = view.get<sg::Light, sg::Transform>(entity);

                const auto &properties = scene_light.GetLightProperties();

                LightInfo light{{transform.GetTranslation(), static_cast<float>(scene_light.GetLightType())},
                                {properties.color, properties.intensity},
                                {transform.GetRotation() * properties.direction, properties.range},
                                {properties.inner_cone_angle, properties.outer_cone_angle}};

                // TODO: light direction wrong
                switch (scene_light.GetLightType())
                {
                case sg::LightType::Directional:
                {
                    if (m_LightingState.directional_lights.size() < light_count)
                        m_LightingState.directional_lights.push_back(light);

                    break;
                }
                case sg::LightType::Point:
                {
                    if (m_LightingState.point_lights.size() < light_count)
                        m_LightingState.point_lights.push_back(light);

                    break;
                }
                case sg::LightType::Spot:
                {
                    if (m_LightingState.spot_lights.size() < light_count)
                        m_LightingState.spot_lights.push_back(light);

                    break;
                }
                default:
                    break;
                }
            }

            T light_info;

            std::copy(m_LightingState.directional_lights.begin(), m_LightingState.directional_lights.end(), light_info.directional_lights);
            std::copy(m_LightingState.point_lights.begin(), m_LightingState.point_lights.end(), light_info.point_lights);
            std::copy(m_LightingState.spot_lights.begin(), m_LightingState.spot_lights.end(), light_info.spot_lights);

            auto &render_frame = render_context.GetActiveFrame();
            m_LightingState.light_buffer = render_frame.AllocateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(T));
            m_LightingState.light_buffer.Update(light_info);
        }

    protected:
        VkSampleCountFlagBits m_SampleCount{VK_SAMPLE_COUNT_1_BIT};
        std::unordered_map<std::string, ShaderResourceMode> m_ResourceModeMap;
        LightingState m_LightingState{};
        ShaderSource m_VertexShader;
        ShaderSource m_FragmentShader;

    private:
        DepthStencilState m_DepthStencilState{};
        bool m_DisableDepthStencilAttachment{false};
        VkResolveModeFlagBits m_DepthStencilResolveMode{VK_RESOLVE_MODE_NONE};
        std::vector<uint32_t> m_InputAttachments = {};
        std::vector<uint32_t> m_OutputAttachments = {0};
        std::vector<uint32_t> m_ColorResolveAttachments = {};
        uint32_t m_DepthStencilResolveAttachment{VK_ATTACHMENT_UNUSED};
    };
}
