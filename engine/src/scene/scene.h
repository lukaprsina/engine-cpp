#pragma once

#include "entt/entt.hpp"

namespace engine
{
    class Scene
    {
    public:
        Scene();
        Scene(const std::string &name);
        ~Scene();
        Scene(const Scene &);

        const entt::registry &GetRegistry() const { return m_Registry; }

    private:
        std::string m_Name{"Unnamed scene"};
        entt::registry m_Registry{};
    };
}
