#pragma once

#include "scene/components/aabb.h"

namespace engine
{
    namespace sg
    {
        class Submesh;

        class Mesh
        {
        public:
            Mesh() = default;
            Mesh(const std::string &name);
            ~Mesh();
            Mesh(const Mesh &) = default;

            void UpdateBounds(const std::vector<glm::vec3> &vertex_data, const std::vector<uint16_t> &index_data = {});
            void AddSubmesh(Submesh &submesh);

            const std::vector<Submesh *> &GetSubmeshes() const { return m_Submeshes; }
            const AABB &GetBounds() const { return m_Bounds; };

        private:
            std::string m_Name;
            AABB m_Bounds;
            std::vector<Submesh *> m_Submeshes;
        };
    }
}