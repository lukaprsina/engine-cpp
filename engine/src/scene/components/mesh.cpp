#include "scene/components/mesh.h"

#include "scene/components/submesh.h"

namespace engine
{
    namespace sg
    {
        Mesh::Mesh(const std::string &name)
            : m_Name(name)
        {
        }

        Mesh::~Mesh()
        {
        }

        void Mesh::UpdateBounds(const std::vector<glm::vec3> &vertex_data, const std::vector<uint16_t> &index_data)
        {
            m_Bounds.Update(vertex_data, index_data);
        }

        void Mesh::AddSubmesh(Submesh &submesh)
        {
            m_Submeshes.push_back(&submesh);
        }
    }
}
