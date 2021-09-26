#pragma once

ENG_DISABLE_WARNINGS()
#include <entt/entt.hpp>
ENG_ENABLE_WARNINGS()

namespace engine
{
    class Entity;
    class Window;
    class RenderPipeline;
    class Device;
    class RenderContext;
    class CommandBuffer;
    class RenderTarget;

    namespace sg
    {
        class Light;
        class Sampler;
        class Image;
        class Texture;
        class PBRMaterial;
        class Submesh;
        class PerspectiveCamera;
    }

    class Scene
    {
    public:
        Scene();
        Scene(const std::string &name);
        ~Scene();

        void Update(float delta_time);
        void Draw(RenderContext &render_context, CommandBuffer &command_buffer, RenderTarget &render_target,
                  VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

        Entity CreateEntity();
        void AddFreeCamera(VkExtent2D extent, Window *window);
        void CreateRenderPipeline(Device &device);

        entt::registry &GetRegistry() { return m_Registry; }
        Entity &GetDefaultCamera() { return *m_DefaultCamera; }

        std::vector<std::unique_ptr<Entity>> &GetLights() { return m_Lights; }
        std::vector<std::unique_ptr<sg::Sampler>> &GetSamplers() { return m_Samplers; }
        std::vector<std::unique_ptr<sg::Image>> &GetImages() { return m_Images; }
        std::vector<std::unique_ptr<sg::Texture>> &GetTextures() { return m_Textures; }
        std::vector<std::unique_ptr<sg::PBRMaterial>> &GetMaterials() { return m_Materials; }
        std::vector<std::unique_ptr<Entity>> &GetMeshes() { return m_Meshes; }
        std::vector<std::unique_ptr<Entity>> &GetCameras() { return m_Cameras; }
        std::vector<std::unique_ptr<sg::Submesh>> &GetSubmeshes() { return m_Submeshes; }

    private:
        std::string m_Name{"Unnamed scene"};
        entt::registry m_Registry{};

        std::vector<std::unique_ptr<Entity>> m_Lights;
        std::vector<std::unique_ptr<sg::Sampler>> m_Samplers;
        std::vector<std::unique_ptr<sg::Image>> m_Images;
        std::vector<std::unique_ptr<sg::Texture>> m_Textures;
        std::vector<std::unique_ptr<sg::PBRMaterial>> m_Materials;
        std::vector<std::unique_ptr<Entity>> m_Meshes;
        std::vector<std::unique_ptr<Entity>> m_Cameras;
        std::vector<std::unique_ptr<sg::Submesh>> m_Submeshes;

        Entity *m_DefaultCamera;
        std::unique_ptr<RenderPipeline> m_RenderPipeline{};
    };
}
