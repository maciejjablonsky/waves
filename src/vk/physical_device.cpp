#include <algorithm>
#include <fmt/color.h>
#include <fmt/format.h>
#include <print>
#include <vector>
#include <vk/physical_device.hpp>
#include <optional>

namespace wf::vk
{
struct queue_family_indices
{
    std::optional<uint32_t> graphics_family;

    bool is_complete() const
    {
        return graphics_family.has_value(); 
    }
};

queue_family_indices find_queue_families(VkPhysicalDevice device)
{
    queue_family_indices indices{};
    uint32_t queue_family_count{};
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, std::addressof(queue_family_count), nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, std::addressof(queue_family_count), queue_families.data());

    int i{};
    for (const auto& queue_family : queue_families)
    {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics_family = i; 
        }

        if (indices.is_complete())
        {
            break; 
        }
        ++i;
    }


    return indices;
}

bool is_physical_device_suitable(VkPhysicalDevice device)
{
    auto qf_indices = find_queue_families(device);
    return qf_indices.is_complete();
}

void present_device(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceProperties(device, std::addressof(device_properties));
    vkGetPhysicalDeviceFeatures(device, std::addressof(device_features));

    auto tag = fmt::format(fg(fmt::color::cyan), "vk physical device");
    fmt::println("[{}] {}",tag, device_properties.deviceName);
}

physical_device::physical_device(instance& instance) : instance_{instance}
{
    uint32_t device_count{};
    vkEnumeratePhysicalDevices(
        instance_.get(), std::addressof(device_count), nullptr);
    if (not device_count)
    {
        throw std::runtime_error{"failed to find GPUs with Vulkan support!"};
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(
        instance_.get(), std::addressof(device_count), devices.data());

    if (auto potential_device_it = std::find_if(std::begin(devices),
                                                std::end(devices),
                                                is_physical_device_suitable);
        std::end(devices) != potential_device_it)
    {
        physical_device_ = *potential_device_it;
    }
    else
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    present_device(physical_device_);
}
} // namespace wf::vk
