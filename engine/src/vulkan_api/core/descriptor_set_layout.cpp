#include "vulkan_api/core/descriptor_set_layout.h"

#include "vulkan_api/device.h"
#include "vulkan_api/physical_device.h"
#include "renderer/shader.h"

namespace engine
{
    namespace
    {
        inline VkDescriptorType FindDescriptorType(ShaderResourceType resource_type, bool dynamic)
        {
            switch (resource_type)
            {
            case ShaderResourceType::InputAttachment:
                return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
                break;
            case ShaderResourceType::Image:
                return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                break;
            case ShaderResourceType::ImageSampler:
                return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                break;
            case ShaderResourceType::ImageStorage:
                return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                break;
            case ShaderResourceType::Sampler:
                return VK_DESCRIPTOR_TYPE_SAMPLER;
                break;
            case ShaderResourceType::BufferUniform:
                if (dynamic)
                {
                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                }
                else
                {
                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                }
                break;
            case ShaderResourceType::BufferStorage:
                if (dynamic)
                {
                    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                }
                else
                {
                    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                }
                break;
            default:
                throw std::runtime_error("No conversion possible for the shader resource type.");
                break;
            }
        }

        inline bool ValidateBinding(const VkDescriptorSetLayoutBinding &binding, const std::vector<VkDescriptorType> &blacklist)
        {
            return !(std::find_if(blacklist.begin(), blacklist.end(), [binding](const VkDescriptorType &type)
                                  { return type == binding.descriptorType; }) != blacklist.end());
        }

        inline bool ValidateFlags(const PhysicalDevice &gpu, const std::vector<VkDescriptorSetLayoutBinding> &bindings, const std::vector<VkDescriptorBindingFlagsEXT> &flags)
        {
            // Assume bindings are valid if there are no flags
            if (flags.empty())
            {
                return true;
            }

            // Binding count has to equal flag count as its a 1:1 mapping
            if (bindings.size() != flags.size())
            {
                ENG_CORE_ERROR("Binding count has to be equal to flag count.");
                return false;
            }

            return true;
        }
    }

    DescriptorSetLayout::DescriptorSetLayout(Device &device,
                                             const uint32_t set_index,
                                             const std::vector<ShaderModule *> &shader_modules,
                                             const std::vector<ShaderResource> &resource_set)
        : m_Device(device), m_SetIndex(set_index), m_ShaderModules(shader_modules)
    {
        for (auto &resource : resource_set)
        {
            // Skip shader resources whitout a binding point
            if (resource.type == ShaderResourceType::Input ||
                resource.type == ShaderResourceType::Output ||
                resource.type == ShaderResourceType::PushConstant ||
                resource.type == ShaderResourceType::SpecializationConstant)
            {
                continue;
            }

            // Convert from ShaderResourceType to VkDescriptorType.
            auto descriptor_type = FindDescriptorType(resource.type, resource.mode == ShaderResourceMode::Dynamic);

            if (resource.mode == ShaderResourceMode::UpdateAfterBind)
            {
                m_BindingFlags.push_back(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT);
            }
            else
            {
                // When creating a descriptor set layout, if we give a structure to create_info.pNext, each binding needs to have a binding flag
                // (pBindings[i] uses the flags in pBindingFlags[i])
                // Adding 0 ensures the bindings that dont use any flags are mapped correctly.
                m_BindingFlags.push_back(0);
            }

            // Convert ShaderResource to VkDescriptorSetLayoutBinding
            VkDescriptorSetLayoutBinding layout_binding{};

            layout_binding.binding = resource.binding;
            layout_binding.descriptorCount = resource.array_size;
            layout_binding.descriptorType = descriptor_type;
            layout_binding.stageFlags = static_cast<VkShaderStageFlags>(resource.stages);

            m_Bindings.push_back(layout_binding);

            // Store mapping between binding and the binding point
            m_BindingsLookup.emplace(resource.binding, layout_binding);

            m_BindingFlagsLookup.emplace(resource.binding, m_BindingFlags.back());

            m_ResourcesLookup.emplace(resource.name, resource.binding);
        }

        VkDescriptorSetLayoutCreateInfo create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        create_info.flags = 0;
        create_info.bindingCount = ToUint32_t(m_Bindings.size());
        create_info.pBindings = m_Bindings.data();

        // Handle update-after-bind extensions
        if (std::find_if(resource_set.begin(), resource_set.end(),
                         [](const ShaderResource &shader_resource)
                         { return shader_resource.mode == ShaderResourceMode::UpdateAfterBind; }) != resource_set.end())
        {
            // Spec states you can't have ANY dynamic resources if you have one of the bindings set to update-after-bind
            if (std::find_if(resource_set.begin(), resource_set.end(),
                             [](const ShaderResource &shader_resource)
                             { return shader_resource.mode == ShaderResourceMode::Dynamic; }) != resource_set.end())
            {
                throw std::runtime_error("Cannot create descriptor set layout, dynamic resources are not allowed if at least one resource is update-after-bind.");
            }

            if (!ValidateFlags(device.GetGPU(), m_Bindings, m_BindingFlags))
            {
                throw std::runtime_error("Invalid binding, couldn't create descriptor set layout.");
            }

            VkDescriptorSetLayoutBindingFlagsCreateInfoEXT binding_flags_create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT};
            binding_flags_create_info.bindingCount = ToUint32_t(m_BindingFlags.size());
            binding_flags_create_info.pBindingFlags = m_BindingFlags.data();

            create_info.pNext = &binding_flags_create_info;
            create_info.flags |= std::find(m_BindingFlags.begin(), m_BindingFlags.end(), VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT) != m_BindingFlags.end() ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT : 0;
        }

        // Create the Vulkan descriptor set layout handle
        VkResult result = vkCreateDescriptorSetLayout(device.GetHandle(), &create_info, nullptr, &m_Handle);

        if (result != VK_SUCCESS)
        {
            throw VulkanException{result, "Cannot create DescriptorSetLayout"};
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        if (m_Handle != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(m_Device.GetHandle(), m_Handle, nullptr);
    }

    DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout &&other)
        : m_Device{other.m_Device},
          m_Handle{other.m_Handle},
          m_SetIndex{other.m_SetIndex},
          m_Bindings{std::move(other.m_Bindings)},
          m_BindingFlags{std::move(other.m_BindingFlags)},
          m_BindingsLookup{std::move(other.m_BindingsLookup)},
          m_BindingFlagsLookup{std::move(other.m_BindingFlagsLookup)},
          m_ResourcesLookup{std::move(other.m_ResourcesLookup)},
          m_ShaderModules{other.m_ShaderModules}
    {
        other.m_Handle = VK_NULL_HANDLE;
    }

    std::unique_ptr<VkDescriptorSetLayoutBinding> DescriptorSetLayout::GetLayoutBinding(uint32_t binding_index) const
    {
        auto it = m_BindingsLookup.find(binding_index);

        if (it == m_BindingsLookup.end())
        {
            return nullptr;
        }

        return std::make_unique<VkDescriptorSetLayoutBinding>(it->second);
    }

}
