#pragma once

#include "vulkan_api/rendering/pipeline_state.h"

namespace engine
{
    class Device;

    class Pipeline
    {
    public:
        Pipeline(Device &device);
        virtual ~Pipeline();
    };

    class GraphicsPipeline : public Pipeline
    {
    public:
        GraphicsPipeline(Device &device,
                         VkPipelineCache pipeline_cache,
                         PipelineState &pipeline_state);
        ~GraphicsPipeline();
    };

    class ComputePipeline : public Pipeline
    {
    public:
        ComputePipeline(Device &device,
                        VkPipelineCache pipeline_cache,
                        PipelineState &pipeline_state);
        ~ComputePipeline();
    };
}
