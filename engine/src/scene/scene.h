#pragma once

#include <entt/entt.hpp>

namespace engine
{
    class Entity;

    class Scene
    {
    public:
        Scene();
        Scene(const std::string &name);
        ~Scene();
        Scene(const Scene &);

        template <typename T>
        void OnComponentAdded(Entity entity, T &component);

        entt::registry &GetRegistry() { return m_Registry; }

    private:
        std::string m_Name{"Unnamed scene"};
        entt::registry m_Registry{};
    };
}
