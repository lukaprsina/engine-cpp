#pragma once

#include "platform/filesystem.h"
#include "core/layer.h"
#include "events/mouse_event.h"
#include "events/key_event.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace engine
{
    class Window;
    class Application;
    class PipelineLayout;
    class CommandBuffer;
    class RenderTarget;
    class RenderFrame;

    namespace core
    {
        class Image;
        class ImageView;
        class Buffer;
        class Sampler;
    }

    struct Font
    {
        Font(const std::string &name, float size)
            : name{name},
              data{fs::ReadBinaryFile(fs::path::Get(fs::path::Type::Fonts, name + ".ttf"))},
              size{size}
        {
            // Keep ownership of the font data to avoid a double delete
            ImFontConfig font_config{};
            font_config.FontDataOwnedByAtlas = false;

            ImGuiIO &io = ImGui::GetIO();
            handle = io.Fonts->AddFontFromMemoryTTF(data.data(), static_cast<int>(data.size()), size, &font_config);
        }

        ImFont *handle{nullptr};
        std::string name;
        std::vector<uint8_t> data;
        float size{};
    };

    class Gui : public Layer
    {
    public:
        Gui(Application &application,
            const Window &window,
            const float font_size = 21.0f,
            bool explicit_update = false);
        ~Gui();

        void Prepare(const VkPipelineCache pipeline_cache, const VkRenderPass render_pass,
                     const std::vector<VkPipelineShaderStageCreateInfo> &shader_stages);

        void NewFrame();

        void OnUpdate(float delta_time) override;
        void Resize(const uint32_t width, const uint32_t height) const;
        void Draw(CommandBuffer &command_buffer);

        void OnEvent(Event &event) override;
        bool OnKeyPressed(KeyPressedEvent &event);
        bool OnKeyReleased(KeyReleasedEvent &event);
        bool OnMouseButtonPressed(MouseButtonPressedEvent &event);
        bool OnMouseButtonReleased(MouseButtonReleasedEvent &event);
        bool OnMouseMoved(MouseMovedEvent &event);

        static const std::string default_font;

    private:
        Application &m_Application;

        std::vector<Font> m_Fonts;
        std::unique_ptr<core::Image> m_FontImage;
        std::unique_ptr<core::ImageView> m_FontImageView;

        std::unique_ptr<core::Buffer> m_VertexBuffer;
        std::unique_ptr<core::Buffer> m_IndexBuffer;

        std::unique_ptr<core::Sampler> m_Sampler{nullptr};
        PipelineLayout *m_PipelineLayout{nullptr};

        VkDescriptorPool m_DescriptorPool{VK_NULL_HANDLE};
        VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};
        VkDescriptorSet m_DescriptorSet{VK_NULL_HANDLE};
        VkPipeline m_Pipeline{VK_NULL_HANDLE};

        bool m_ExplicitUpdate{false};

        void UpdateBuffers(CommandBuffer &command_buffer, RenderFrame &render_frame);
    };
}
