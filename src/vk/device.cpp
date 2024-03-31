#include <algorithm>
#include <fmt/color.h>
#include <fmt/format.h>
#include <optional>
#include <print>
#include <vector>
#include <vk/device.hpp>

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
    fmt::println("[{}] {}", tag, device_properties.deviceName);
}

void device::pick_physical_device_()
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

void device::create_logical_device_()
{
    queue_family_indices indices = find_queue_families(physical_device_);

    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = indices.graphics_family.value();
    queue_create_info.queueCount       = 1;
    float queue_priority               = 1.f;
    queue_create_info.pQueuePriorities = std::addressof(queue_priority);

    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo create_info{};
    create_info.sType                 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos     = std::addressof(queue_create_info);
    create_info.queueCreateInfoCount  = 1;
    create_info.pEnabledFeatures      = std::addressof(device_features);
    create_info.enabledExtensionCount = 0;

    if (validation_layers_enabled)
    {
        create_info.enabledLayerCount =
            static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }
    if (vkCreateDevice(physical_device_,
                       std::addressof(create_info),
                       nullptr,
                       std::addressof(logical_device_)) != VK_SUCCESS)
    {
        throw std::runtime_error{"failed to create logical device!"};
    }

    vkGetDeviceQueue(logical_device_,
                     indices.graphics_family.value(),
                     0,
                     std::addressof(graphics_queue_));
}

device::device(instance& instance) : instance_{instance}
{
    pick_physical_device_();
    create_logical_device_();
}

device::~device() noexcept
{
    vkDestroyDevice(logical_device_, nullptr);
}
} // namespace wf::vk
