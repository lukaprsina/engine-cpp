#include "vulkan_api/subpasses/geometry_subpass.h"

#include "scene/scene.h"
#include "renderer/camera.h"

namespace engine
{
    GeometrySubpass::GeometrySubpass(RenderContext &render_context, ShaderSource &&vertex_shader, ShaderSource &&fragment_shader, Scene &scene, Camera &camera)
        : Subpass(render_context,
                  std::move(vertex_shader),
                  std::move(fragment_shader)),
          m_Scene(scene)
    {
    }

    GeometrySubpass::~GeometrySubpass()
    {
    }

    void GeometrySubpass::Prepare()
    {
    }

    void GeometrySubpass::Draw(CommandBuffer &command_buffer)
    {
    }
}
