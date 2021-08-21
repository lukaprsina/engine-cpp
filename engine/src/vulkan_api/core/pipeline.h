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
        Pipeline(Pipeline &&other);
        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;
        Pipeline &operator=(Pipeline &&) = delete;

        VkPipeline GetHandle() const { return m_Handle; }

    protected:
        PipelineState m_State;
        VkPipeline m_Handle = VK_NULL_HANDLE;

    private:
        Device &m_Device;
    };

    class GraphicsPipeline : public Pipeline
    {
    public:
        GraphicsPipeline(Device &device,
                         VkPipelineCache pipeline_cache,
                         PipelineState &pipeline_state);
        virtual ~GraphicsPipeline();
        GraphicsPipeline(GraphicsPipeline &&) = default;
    };

    class ComputePipeline : public Pipeline
    {
    public:
        ComputePipeline(Device &device,
                        VkPipelineCache pipeline_cache,
                        PipelineState &pipeline_state);
        virtual ~ComputePipeline();
        ComputePipeline(ComputePipeline &&) = default;
    };
}
