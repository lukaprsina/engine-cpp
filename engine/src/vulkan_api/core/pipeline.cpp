#include "vulkan_api/core/pipeline.h"

#include "vulkan_api/device.h"

namespace engine
{
    Pipeline::Pipeline(Device &device)
    {
    }

    Pipeline::~Pipeline()
    {
    }

    GraphicsPipeline::GraphicsPipeline(Device &device,
                                       VkPipelineCache pipeline_cache,
                                       PipelineState &pipeline_state)
    {
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
    }

    ComputePipeline::ComputePipeline(Device &device,
                                     VkPipelineCache pipeline_cache,
                                     PipelineState &pipeline_state)
    {
    }

    ComputePipeline::~ComputePipeline()
    {
    }
}
