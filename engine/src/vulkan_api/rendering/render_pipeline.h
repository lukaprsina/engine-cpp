#pragma once

namespace engine
{
    class CommandBuffer;
    class RenderTarget;
    class Subpass;
    class Layer;
    class Device;
    class RenderContext;

    class RenderPipeline
    {
    public:
        RenderPipeline(Device &device, std::vector<std::unique_ptr<Subpass>> &&subpasses = {});
        ~RenderPipeline();

        void AddSubpass(std::unique_ptr<Subpass> &&subpass);

        void Draw(RenderContext &render_context, Layer &layer, CommandBuffer &command_buffer, RenderTarget &render_target,
                  VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

        std::vector<VkClearValue> &GetClearColor() { return m_ClearValue; }
        void SetClearColor(std::vector<VkClearValue> &color) { m_ClearValue = color; }

        std::vector<LoadStoreInfo> &GetLoadStore() { return m_LoadStore; }
        void SetLoadStore(std::vector<LoadStoreInfo> &load_store) { m_LoadStore = load_store; }

    private:
        Device &m_Device;
        std::vector<std::unique_ptr<Subpass>> m_Subpasses{};
        std::vector<LoadStoreInfo> m_LoadStore = std::vector<LoadStoreInfo>(2);
        std::vector<VkClearValue> m_ClearValue = std::vector<VkClearValue>(2);
        size_t m_ActiveSubpassIndex{0};
    };
}
