#pragma once

#include "renderer/shader.h"
#include "vulkan_api/rendering/pipeline_state.h"
#include "vulkan_api/core/buffer_pool.h"
#include "renderer/shader.h"
#include "common/glm.h"

namespace engine
{
    class CommandBuffer;
    class ShaderSource;
    class RenderContext;
    class RenderTarget;

    struct alignas(16) LightInfo
    {
        glm::vec4 position;  // position.w represents type of light
        glm::vec4 color;     // color.w represents light intensity
        glm::vec4 direction; // direction.w represents range
        glm::vec2 info;      // (only used for spot lights) info.x represents light inner cone angle, info.y represents light outer cone angle
    };

    struct LightingState
    {
        std::vector<LightInfo> directional_lights;
        std::vector<LightInfo> point_lights;
        std::vector<LightInfo> spot_lights;
        // TODO
        BufferAllocation light_buffer;
    };

    class Subpass
    {
    public:
        Subpass(RenderContext &render_context,
                ShaderSource &&vertex_shader,
                ShaderSource &&fragment_shader);
        virtual ~Subpass();

        virtual void Prepare() = 0;
        virtual void Draw(CommandBuffer &command_buffer) = 0;
        void UpdateRenderTargetAttachments(RenderTarget &render_target);

        RenderContext &get_render_context();

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

    protected:
        RenderContext &m_RenderContext;
        VkSampleCountFlagBits m_SampleCount{VK_SAMPLE_COUNT_1_BIT};
        std::unordered_map<std::string, ShaderResourceMode> m_ResourceModeMap;
        LightingState m_LightingState{};

    private:
        ShaderSource m_VertexShader;
        ShaderSource m_FragmentShader;
        DepthStencilState m_DepthStencilState{};
        bool m_DisableDepthStencilAttachment{false};
        VkResolveModeFlagBits m_DepthStencilResolveMode{VK_RESOLVE_MODE_NONE};
        std::vector<uint32_t> m_InputAttachments = {};
        std::vector<uint32_t> m_OutputAttachments = {0};
        std::vector<uint32_t> m_ColorResolveAttachments = {};
        uint32_t m_DepthStencilResolveAttachment{VK_ATTACHMENT_UNUSED};
    };
}
