#pragma once

#include <vulkan/vulkan.h>
#include <vk/instance.hpp>
#include <utils.hpp>

namespace wf::vk
{
class device : non_copyable
{
    std::reference_wrapper<instance> instance_;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice logical_device_          = VK_NULL_HANDLE;
    VkQueue graphics_queue_           = VK_NULL_HANDLE;

    void pick_physical_device_();
    void create_logical_device_();
  public:
    device(instance& instance);
    ~device()noexcept;
};
}