#include <GLFW/glfw3.h>
#include <algorithm>
#include <fmt/color.h>
#include <fmt/format.h>
#include <format>
#include <magic_enum/magic_enum.hpp>
#include <print>
#include <ranges>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>
#include <vk/instance.hpp>

namespace wf::vk
{
static VKAPI_ATTR VkBool32
vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                  void* pUserData)
{
    auto severity = [=](VkDebugUtilsMessageSeverityFlagBitsEXT s) {
        switch (s)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            return fmt::format(fg(fmt::color::green), "{:^10}", "vk verbose");
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            return fmt::format(fg(fmt::color::cyan), "{:^10}", "vk info");
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            return fmt::format(fg(fmt::color::orange), "{:^10}", "vk warning");
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            return fmt::format(fg(fmt::color::red), "{:^10}", "vk error");
        default:
            std::unreachable();
        };
    };
    std::println(
        stderr, "[{}] {}", severity(messageSeverity), pCallbackData->pMessage);
    return VK_FALSE;
}

bool instance::check_validation_layer_support_()
{
    uint32_t layer_count{};
    vkEnumerateInstanceLayerProperties(std::addressof(layer_count), nullptr);
    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(std::addressof(layer_count),
                                       available_layers.data());

    for (const auto& layer_name : validation_layers)
    {
        bool layer_found = false;
        for (const auto& layer_properties : available_layers)
        {
            if (std::strcmp(layer_name, layer_properties.layerName) == 0)
            {
                layer_found = true;
                break;
            }
        }
        if (not layer_found)
        {
            return false;
        }
    }

    return true;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

void populate_debug_messenger_create_info(
    VkDebugUtilsMessengerCreateInfoEXT& create_info)
{
    create_info       = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = vk_debug_callback;
}

void instance::set_debug_messenger_()
{
    if (not validation_layers_enabled)
    {
        return;
    }
    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    populate_debug_messenger_create_info(create_info);

    if (CreateDebugUtilsMessengerEXT(*this,
                                     std::addressof(create_info),
                                     nullptr,
                                     std::addressof(debug_messenger_)) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

std::vector<const char*> get_required_extensions()
{
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(
        nullptr, std::addressof(extension_count), nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(
        nullptr, std::addressof(extension_count), available_extensions.data());
    auto available_ext_range =
        available_extensions |
        std::views::transform([](const VkExtensionProperties& ep) {
            return std::string_view{ep.extensionName};
        });

    std::set<std::string_view> available_extension_set{std::from_range_t{},
                                                       available_ext_range};
    std::ranges::for_each(available_ext_range,
                          [](std::string_view v) { std::println("{}", v); });

    uint32_t glfw_extension_count{};
    const char** glfw_extensions =
        glfwGetRequiredInstanceExtensions(std::addressof(glfw_extension_count));
    std::vector<const char*> required_extensions(
        glfw_extensions, glfw_extensions + glfw_extension_count);
    if (validation_layers_enabled)
    {
        required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    std::set<std::string_view> required_extension_set{std::from_range_t{},
                                                      required_extensions};
    assert(std::all_of(std::begin(required_extension_set),
                       std::end(required_extension_set),
                       [&](const auto& elem) {
                           return available_extension_set.find(elem) !=
                                  std::end(available_extension_set);
                       }));

    return required_extensions;
}
instance::instance()
{
    VkApplicationInfo app_info{
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName   = "Vulkan app",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName        = "No engine",
        .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion         = VK_API_VERSION_1_3,
    };

    if (validation_layers_enabled and not check_validation_layer_support_())
    {
        throw std::runtime_error{
            "validation layers requested, but not available!"};
    }

    auto required_extensions = get_required_extensions();

    VkInstanceCreateInfo create_info{
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo        = std::addressof(app_info),
        .enabledLayerCount       = 0,
        .enabledExtensionCount   = wf::to<uint32_t>(required_extensions.size()),
        .ppEnabledExtensionNames = required_extensions.data(),
    };
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
    if (validation_layers_enabled)
    {
        create_info.enabledLayerCount =
            wf::to<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
        populate_debug_messenger_create_info(debug_create_info);
        create_info.pNext = std::addressof(debug_create_info);
    }

    if (VkResult result = vkCreateInstance(
            std::addressof(create_info), nullptr, std::addressof(instance_));
        VK_SUCCESS != result)
    {
        throw std::runtime_error{
            std::format("failed to create instance! error: {}",
                        magic_enum::enum_name(result))};
    }
    set_debug_messenger_();
}
instance::operator VkInstance()
{
    return instance_;
}
instance::~instance()
{
    if (validation_layers_enabled)
    {
        DestroyDebugUtilsMessengerEXT(*this, debug_messenger_, nullptr);
    }
    vkDestroyInstance(*this, nullptr);
}
} // namespace wf::vk