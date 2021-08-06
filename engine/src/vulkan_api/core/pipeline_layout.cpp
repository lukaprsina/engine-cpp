#include "vulkan_api/core/pipeline_layout.h"

namespace engine
{
    PipelineLayout::PipelineLayout(Device &device,
                                   const std::vector<ShaderModule *> &shader_modules)
        : m_Device(device), m_ShaderModules(shader_modules)
    {
    }

    PipelineLayout::~PipelineLayout()
    {
    }
}
