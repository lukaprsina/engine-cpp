/* Copyright (c) 2018-2021, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "vulkan_api/core/descriptor_pool.h"
#include "vulkan_api/core/descriptor_set.h"
#include "vulkan_api/core/descriptor_set_layout.h"
#include "vulkan_api/core/framebuffer.h"
#include "vulkan_api/core/pipeline.h"
#include "vulkan_api/rendering/pipeline_state.h"
#include "vulkan_api/render_target.h"

#include "common/helpers.h"

namespace std
{
	template <>
	struct hash<engine::ShaderSource>
	{
		std::size_t operator()(const engine::ShaderSource &shader_source) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, shader_source.GetID());

			return result;
		}
	};

	template <>
	struct hash<engine::ShaderVariant>
	{
		std::size_t operator()(const engine::ShaderVariant &shader_variant) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, shader_variant.GetID());

			return result;
		}
	};

	template <>
	struct hash<engine::ShaderModule>
	{
		std::size_t operator()(const engine::ShaderModule &shader_module) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, shader_module.GetID());

			return result;
		}
	};

	template <>
	struct hash<engine::DescriptorSetLayout>
	{
		std::size_t operator()(const engine::DescriptorSetLayout &descriptor_set_layout) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, descriptor_set_layout.GetHandle());

			return result;
		}
	};

	template <>
	struct hash<engine::DescriptorPool>
	{
		std::size_t operator()(const engine::DescriptorPool &descriptor_pool) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, descriptor_pool.GetDescriptorSetLayout());

			return result;
		}
	};

	template <>
	struct hash<engine::PipelineLayout>
	{
		std::size_t operator()(const engine::PipelineLayout &pipeline_layout) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, pipeline_layout.GetHandle());

			return result;
		}
	};

	template <>
	struct hash<engine::RenderPass>
	{
		std::size_t operator()(const engine::RenderPass &render_pass) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, render_pass.GetHandle());

			return result;
		}
	};

	template <>
	struct hash<engine::Attachment>
	{
		std::size_t operator()(const engine::Attachment &attachment) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, static_cast<std::underlying_type<VkFormat>::type>(attachment.format));
			engine::HashCombine(result, static_cast<std::underlying_type<VkSampleCountFlagBits>::type>(attachment.samples));
			engine::HashCombine(result, attachment.usage);
			engine::HashCombine(result, static_cast<std::underlying_type<VkImageLayout>::type>(attachment.initial_layout));

			return result;
		}
	};

	template <>
	struct hash<engine::LoadStoreInfo>
	{
		std::size_t operator()(const engine::LoadStoreInfo &load_store_info) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, static_cast<std::underlying_type<VkAttachmentLoadOp>::type>(load_store_info.load_op));
			engine::HashCombine(result, static_cast<std::underlying_type<VkAttachmentStoreOp>::type>(load_store_info.store_op));

			return result;
		}
	};

	template <>
	struct hash<engine::SubpassInfo>
	{
		std::size_t operator()(const engine::SubpassInfo &subpass_info) const
		{
			std::size_t result = 0;

			for (uint32_t output_attachment : subpass_info.output_attachments)
			{
				engine::HashCombine(result, output_attachment);
			}

			for (uint32_t input_attachment : subpass_info.input_attachments)
			{
				engine::HashCombine(result, input_attachment);
			}

			for (uint32_t resolve_attachment : subpass_info.color_resolve_attachments)
			{
				engine::HashCombine(result, resolve_attachment);
			}

			engine::HashCombine(result, subpass_info.disable_depth_stencil_attachment);
			engine::HashCombine(result, subpass_info.depth_stencil_resolve_attachment);
			engine::HashCombine(result, subpass_info.depth_stencil_resolve_mode);

			return result;
		}
	};

	template <>
	struct hash<engine::SpecializationConstantState>
	{
		std::size_t operator()(const engine::SpecializationConstantState &specialization_constant_state) const
		{
			std::size_t result = 0;

			for (auto constants : specialization_constant_state.GetSpecializationConstantState())
			{
				engine::HashCombine(result, constants.first);
				for (const auto data : constants.second)
				{
					engine::HashCombine(result, data);
				}
			}

			return result;
		}
	};

	template <>
	struct hash<engine::ShaderResource>
	{
		std::size_t operator()(const engine::ShaderResource &shader_resource) const
		{
			std::size_t result = 0;

			if (shader_resource.type == engine::ShaderResourceType::Input ||
				shader_resource.type == engine::ShaderResourceType::Output ||
				shader_resource.type == engine::ShaderResourceType::PushConstant ||
				shader_resource.type == engine::ShaderResourceType::SpecializationConstant)
			{
				return result;
			}

			engine::HashCombine(result, shader_resource.set);
			engine::HashCombine(result, shader_resource.binding);
			engine::HashCombine(result, static_cast<std::underlying_type<engine::ShaderResourceType>::type>(shader_resource.type));
			engine::HashCombine(result, shader_resource.mode);

			return result;
		}
	};

	template <>
	struct hash<VkDescriptorBufferInfo>
	{
		std::size_t operator()(const VkDescriptorBufferInfo &descriptor_buffer_info) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, descriptor_buffer_info.buffer);
			engine::HashCombine(result, descriptor_buffer_info.range);
			engine::HashCombine(result, descriptor_buffer_info.offset);

			return result;
		}
	};

	template <>
	struct hash<VkDescriptorImageInfo>
	{
		std::size_t operator()(const VkDescriptorImageInfo &descriptor_image_info) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, descriptor_image_info.imageView);
			engine::HashCombine(result, static_cast<std::underlying_type<VkImageLayout>::type>(descriptor_image_info.imageLayout));
			engine::HashCombine(result, descriptor_image_info.sampler);

			return result;
		}
	};

	template <>
	struct hash<VkWriteDescriptorSet>
	{
		std::size_t operator()(const VkWriteDescriptorSet &write_descriptor_set) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, write_descriptor_set.dstSet);
			engine::HashCombine(result, write_descriptor_set.dstBinding);
			engine::HashCombine(result, write_descriptor_set.dstArrayElement);
			engine::HashCombine(result, write_descriptor_set.descriptorCount);
			engine::HashCombine(result, write_descriptor_set.descriptorType);

			switch (write_descriptor_set.descriptorType)
			{
			case VK_DESCRIPTOR_TYPE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
				for (uint32_t i = 0; i < write_descriptor_set.descriptorCount; i++)
				{
					engine::HashCombine(result, write_descriptor_set.pImageInfo[i]);
				}
				break;

			case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
				for (uint32_t i = 0; i < write_descriptor_set.descriptorCount; i++)
				{
					engine::HashCombine(result, write_descriptor_set.pTexelBufferView[i]);
				}
				break;

			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
				for (uint32_t i = 0; i < write_descriptor_set.descriptorCount; i++)
				{
					engine::HashCombine(result, write_descriptor_set.pBufferInfo[i]);
				}
				break;

			default:
				// Not implemented
				break;
			};

			return result;
		}
	};

	template <>
	struct hash<VkVertexInputAttributeDescription>
	{
		std::size_t operator()(const VkVertexInputAttributeDescription &vertex_attrib) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, vertex_attrib.binding);
			engine::HashCombine(result, static_cast<std::underlying_type<VkFormat>::type>(vertex_attrib.format));
			engine::HashCombine(result, vertex_attrib.location);
			engine::HashCombine(result, vertex_attrib.offset);

			return result;
		}
	};

	template <>
	struct hash<VkVertexInputBindingDescription>
	{
		std::size_t operator()(const VkVertexInputBindingDescription &vertex_binding) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, vertex_binding.binding);
			engine::HashCombine(result, static_cast<std::underlying_type<VkVertexInputRate>::type>(vertex_binding.inputRate));
			engine::HashCombine(result, vertex_binding.stride);

			return result;
		}
	};

	template <>
	struct hash<engine::StencilOpState>
	{
		std::size_t operator()(const engine::StencilOpState &stencil) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, static_cast<std::underlying_type<VkCompareOp>::type>(stencil.compare_op));
			engine::HashCombine(result, static_cast<std::underlying_type<VkStencilOp>::type>(stencil.depth_fail_op));
			engine::HashCombine(result, static_cast<std::underlying_type<VkStencilOp>::type>(stencil.fail_op));
			engine::HashCombine(result, static_cast<std::underlying_type<VkStencilOp>::type>(stencil.pass_op));

			return result;
		}
	};

	template <>
	struct hash<VkExtent2D>
	{
		size_t operator()(const VkExtent2D &extent) const
		{
			size_t result = 0;

			engine::HashCombine(result, extent.width);
			engine::HashCombine(result, extent.height);

			return result;
		}
	};

	template <>
	struct hash<VkOffset2D>
	{
		size_t operator()(const VkOffset2D &offset) const
		{
			size_t result = 0;

			engine::HashCombine(result, offset.x);
			engine::HashCombine(result, offset.y);

			return result;
		}
	};

	template <>
	struct hash<VkRect2D>
	{
		size_t operator()(const VkRect2D &rect) const
		{
			size_t result = 0;

			engine::HashCombine(result, rect.extent);
			engine::HashCombine(result, rect.offset);

			return result;
		}
	};

	template <>
	struct hash<VkViewport>
	{
		size_t operator()(const VkViewport &viewport) const
		{
			size_t result = 0;

			engine::HashCombine(result, viewport.width);
			engine::HashCombine(result, viewport.height);
			engine::HashCombine(result, viewport.maxDepth);
			engine::HashCombine(result, viewport.minDepth);
			engine::HashCombine(result, viewport.x);
			engine::HashCombine(result, viewport.y);

			return result;
		}
	};

	template <>
	struct hash<engine::ColorBlendAttachmentState>
	{
		std::size_t operator()(const engine::ColorBlendAttachmentState &color_blend_attachment) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, static_cast<std::underlying_type<VkBlendOp>::type>(color_blend_attachment.alpha_blend_op));
			engine::HashCombine(result, color_blend_attachment.blend_enable);
			engine::HashCombine(result, static_cast<std::underlying_type<VkBlendOp>::type>(color_blend_attachment.color_blend_op));
			engine::HashCombine(result, color_blend_attachment.color_write_mask);
			engine::HashCombine(result, static_cast<std::underlying_type<VkBlendFactor>::type>(color_blend_attachment.dst_alpha_blend_factor));
			engine::HashCombine(result, static_cast<std::underlying_type<VkBlendFactor>::type>(color_blend_attachment.dst_color_blend_factor));
			engine::HashCombine(result, static_cast<std::underlying_type<VkBlendFactor>::type>(color_blend_attachment.src_alpha_blend_factor));
			engine::HashCombine(result, static_cast<std::underlying_type<VkBlendFactor>::type>(color_blend_attachment.src_color_blend_factor));

			return result;
		}
	};

	template <>
	struct hash<engine::RenderTarget>
	{
		std::size_t operator()(const engine::RenderTarget &render_target) const
		{
			std::size_t result = 0;

			for (auto &view : render_target.GetViews())
			{
				engine::HashCombine(result, view.GetHandle());
				engine::HashCombine(result, view.GetImage().GetHandle());
			}

			return result;
		}
	};

	template <>
	struct hash<engine::PipelineState>
	{
		std::size_t operator()(const engine::PipelineState &pipeline_state) const
		{
			std::size_t result = 0;

			engine::HashCombine(result, pipeline_state.GetPipelineLayout().GetHandle());

			// For graphics only
			if (auto render_pass = pipeline_state.GetRenderPass())
			{
				engine::HashCombine(result, render_pass->GetHandle());
			}

			engine::HashCombine(result, pipeline_state.GetSpecializationConstantState());

			engine::HashCombine(result, pipeline_state.GetSubpassIndex());

			for (auto shader_module : pipeline_state.GetPipelineLayout().GetShaderModules())
			{
				engine::HashCombine(result, shader_module->GetID());
			}

			// VkPipelineVertexInputStateCreateInfo
			for (auto &attribute : pipeline_state.GetVertexInputState().attributes)
			{
				engine::HashCombine(result, attribute);
			}

			for (auto &binding : pipeline_state.GetVertexInputState().bindings)
			{
				engine::HashCombine(result, binding);
			}

			// VkPipelineInputAssemblyStateCreateInfo
			engine::HashCombine(result, pipeline_state.GetInputAssemblyState().primitive_restart_enable);
			engine::HashCombine(result, static_cast<std::underlying_type<VkPrimitiveTopology>::type>(pipeline_state.GetInputAssemblyState().topology));

			//VkPipelineViewportStateCreateInfo
			engine::HashCombine(result, pipeline_state.GetViewportState().viewport_count);
			engine::HashCombine(result, pipeline_state.GetViewportState().scissor_count);

			// VkPipelineRasterizationStateCreateInfo
			engine::HashCombine(result, pipeline_state.GetRasterizationState().cull_mode);
			engine::HashCombine(result, pipeline_state.GetRasterizationState().depth_bias_enable);
			engine::HashCombine(result, pipeline_state.GetRasterizationState().depth_clamp_enable);
			engine::HashCombine(result, static_cast<std::underlying_type<VkFrontFace>::type>(pipeline_state.GetRasterizationState().front_face));
			engine::HashCombine(result, static_cast<std::underlying_type<VkPolygonMode>::type>(pipeline_state.GetRasterizationState().polygon_mode));
			engine::HashCombine(result, pipeline_state.GetRasterizationState().rasterizer_discard_enable);

			// VkPipelineMultisampleStateCreateInfo
			engine::HashCombine(result, pipeline_state.GetMultisampleState().alpha_to_coverage_enable);
			engine::HashCombine(result, pipeline_state.GetMultisampleState().alpha_to_one_enable);
			engine::HashCombine(result, pipeline_state.GetMultisampleState().min_sample_shading);
			engine::HashCombine(result, static_cast<std::underlying_type<VkSampleCountFlagBits>::type>(pipeline_state.GetMultisampleState().rasterization_samples));
			engine::HashCombine(result, pipeline_state.GetMultisampleState().sample_shading_enable);
			engine::HashCombine(result, pipeline_state.GetMultisampleState().sample_mask);

			// VkPipelineDepthStencilStateCreateInfo
			engine::HashCombine(result, pipeline_state.GetDepthStencilState().back);
			engine::HashCombine(result, pipeline_state.GetDepthStencilState().depth_bounds_test_enable);
			engine::HashCombine(result, static_cast<std::underlying_type<VkCompareOp>::type>(pipeline_state.GetDepthStencilState().depth_compare_op));
			engine::HashCombine(result, pipeline_state.GetDepthStencilState().depth_test_enable);
			engine::HashCombine(result, pipeline_state.GetDepthStencilState().depth_write_enable);
			engine::HashCombine(result, pipeline_state.GetDepthStencilState().front);
			engine::HashCombine(result, pipeline_state.GetDepthStencilState().stencil_test_enable);

			// VkPipelineColorBlendStateCreateInfo
			engine::HashCombine(result, static_cast<std::underlying_type<VkLogicOp>::type>(pipeline_state.GetColorBlendState().logic_op));
			engine::HashCombine(result, pipeline_state.GetColorBlendState().logic_op_enable);

			for (auto &attachment : pipeline_state.GetColorBlendState().attachments)
			{
				engine::HashCombine(result, attachment);
			}

			return result;
		}
	};
}

namespace engine
{
	namespace
	{
		template <typename T>
		inline void HashParam(size_t &seed, const T &value)
		{
			engine::HashCombine(seed, value);
		}

		template <>
		inline void HashParam(size_t & /*seed*/, const VkPipelineCache & /*value*/)
		{
		}

		template <>
		inline void HashParam<std::vector<uint8_t>>(
			size_t &seed,
			const std::vector<uint8_t> &value)
		{
			engine::HashCombine(seed, std::string{value.begin(), value.end()});
		}

		template <>
		inline void HashParam<std::vector<Attachment>>(
			size_t &seed,
			const std::vector<Attachment> &value)
		{
			for (auto &attachment : value)
			{
				engine::HashCombine(seed, attachment);
			}
		}

		template <>
		inline void HashParam<std::vector<LoadStoreInfo>>(
			size_t &seed,
			const std::vector<LoadStoreInfo> &value)
		{
			for (auto &load_store_info : value)
			{
				engine::HashCombine(seed, load_store_info);
			}
		}

		template <>
		inline void HashParam<std::vector<SubpassInfo>>(
			size_t &seed,
			const std::vector<SubpassInfo> &value)
		{
			for (auto &subpass_info : value)
			{
				engine::HashCombine(seed, subpass_info);
			}
		}

		template <>
		inline void HashParam<std::vector<ShaderModule *>>(
			size_t &seed,
			const std::vector<ShaderModule *> &value)
		{
			for (auto &shader_module : value)
			{
				engine::HashCombine(seed, shader_module->GetID());
			}
		}

		template <>
		inline void HashParam<std::vector<ShaderResource>>(
			size_t &seed,
			const std::vector<ShaderResource> &value)
		{
			for (auto &resource : value)
			{
				engine::HashCombine(seed, resource);
			}
		}

		template <>
		inline void HashParam<std::map<uint32_t, std::map<uint32_t, VkDescriptorBufferInfo>>>(
			size_t &seed,
			const std::map<uint32_t, std::map<uint32_t, VkDescriptorBufferInfo>> &value)
		{
			for (auto &binding_set : value)
			{
				engine::HashCombine(seed, binding_set.first);

				for (auto &binding_element : binding_set.second)
				{
					engine::HashCombine(seed, binding_element.first);
					engine::HashCombine(seed, binding_element.second);
				}
			}
		}

		template <>
		inline void HashParam<std::map<uint32_t, std::map<uint32_t, VkDescriptorImageInfo>>>(
			size_t &seed,
			const std::map<uint32_t, std::map<uint32_t, VkDescriptorImageInfo>> &value)
		{
			for (auto &binding_set : value)
			{
				engine::HashCombine(seed, binding_set.first);

				for (auto &binding_element : binding_set.second)
				{
					engine::HashCombine(seed, binding_element.first);
					engine::HashCombine(seed, binding_element.second);
				}
			}
		}

		template <typename T, typename... Args>
		inline void HashParam(size_t &seed, const T &first_arg, const Args &...args)
		{
			HashParam(seed, first_arg);

			HashParam(seed, args...);
		}

		template <class T, class... A>
		struct RecordHelper
		{
			size_t record(ResourceRecord & /*recorder*/, A &.../*args*/)
			{
				return 0;
			}

			void index(ResourceRecord & /*recorder*/, size_t /*index*/, T & /*resource*/)
			{
			}
		};

		template <class... A>
		struct RecordHelper<ShaderModule, A...>
		{
			size_t record(ResourceRecord &recorder, A &...args)
			{
				return recorder.RegisterShaderModule(args...);
			}

			void index(ResourceRecord &recorder, size_t index, ShaderModule &shader_module)
			{
				recorder.SetShaderModule(index, shader_module);
			}
		};

		template <class... A>
		struct RecordHelper<PipelineLayout, A...>
		{
			size_t record(ResourceRecord &recorder, A &...args)
			{
				return recorder.RegisterPipelineLayout(args...);
			}

			void index(ResourceRecord &recorder, size_t index, PipelineLayout &pipeline_layout)
			{
				recorder.SetPipelineLayout(index, pipeline_layout);
			}
		};

		template <class... A>
		struct RecordHelper<RenderPass, A...>
		{
			size_t record(ResourceRecord &recorder, A &...args)
			{
				return recorder.RegisterRenderPass(args...);
			}

			void index(ResourceRecord &recorder, size_t index, RenderPass &render_pass)
			{
				recorder.SetRenderPass(index, render_pass);
			}
		};

		template <class... A>
		struct RecordHelper<GraphicsPipeline, A...>
		{
			size_t record(ResourceRecord &recorder, A &...args)
			{
				return recorder.RegisterGraphicsPipeline(args...);
			}

			void index(ResourceRecord &recorder, size_t index, GraphicsPipeline &graphics_pipeline)
			{
				recorder.SetGraphicsPipeline(index, graphics_pipeline);
			}
		};
	}

	template <class T, class... A>
	T &request_resource(Device &device, ResourceRecord *recorder, std::unordered_map<std::size_t, T> &resources, A &...args)
	{
		RecordHelper<T, A...> record_helper;

		std::size_t hash{0U};
		HashParam(hash, args...);

		auto res_it = resources.find(hash);

		if (res_it != resources.end())
		{
			return res_it->second;
		}

		// If we do not have it already, create and cache it
		const char *res_type = typeid(T).name();
		size_t res_id = resources.size();

		ENG_CORE_TRACE("Building #{} cache object ({})", res_id, res_type);

// Only error handle in release
#ifndef DEBUG
		try
		{
#endif
			T resource(device, args...);

			auto res_ins_it = resources.emplace(hash, std::move(resource));

			if (!res_ins_it.second)
			{
				throw std::runtime_error{std::string{"Insertion error for #"} + std::to_string(res_id) + "cache object (" + res_type + ")"};
			}

			res_it = res_ins_it.first;

			if (recorder)
			{
				size_t index = record_helper.record(*recorder, args...);
				record_helper.index(*recorder, index, res_it->second);
			}
#ifndef DEBUG
		}
		catch (const std::exception &e)
		{
			ENG_CORE_ERROR("Creation error for #{} cache object ({})", res_id, res_type);
			throw e;
		}
#endif

		return res_it->second;
	}
}
