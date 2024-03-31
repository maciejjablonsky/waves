#pragma once

#include <vulkan/vulkan.h>
#include <vk/instance.hpp>
#include <utils.hpp>

namespace wf::vk
{
class physical_device : non_copyable
{
    std::reference_wrapper<instance> instance_;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  public:
    physical_device(instance& instance);
};
}