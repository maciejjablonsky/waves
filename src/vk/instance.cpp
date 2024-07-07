module;
#include <fmt/color.h>
#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

module vk;

import std;

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
            return fmt::format(
                fg(fmt::color::gray), "{:^10}", "vk unknown error level");
        };
    };
    std::println(
        stderr, "[{}] {}", severity(messageSeverity), pCallbackData->pMessage);
    return VK_FALSE;
}

swap_chain_support_details instance::query_swap_chain_support_(
    VkPhysicalDevice device)
{
    swap_chain_support_details details{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device, surface_, std::addressof(details.capabilities));

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        device, surface_, std::addressof(format_count), nullptr);
    if (format_count)
    {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device,
                                             surface_,
                                             std::addressof(format_count),
                                             details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface_, std::addressof(present_mode_count), nullptr);
    if (present_mode_count)
    {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            surface_,
            std::addressof(present_mode_count),
            details.present_modes.data());
    }

    return details;
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

void instance::create_surface_()
{
    if (glfwCreateWindowSurface(
            instance_, window_.get(), nullptr, std::addressof(surface_)) !=
        VK_SUCCESS)
    {
        throw std::runtime_error{"failed to create window surface!"};
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

static void framebuffer_resize_callback(GLFWwindow* window,
                                        int width,
                                        int height)
{
    auto app = reinterpret_cast<instance*>(glfwGetWindowUserPointer(window));
    app->framebuffer_resized = true;
}

instance::instance(window& window) : window_{window}
{
    glfwSetWindowUserPointer(window_.get(), this);
    glfwSetFramebufferSizeCallback(window_.get(), framebuffer_resize_callback);
    create_instance_();
    set_debug_messenger_();
    create_surface_();
    pick_physical_device_();
    create_logical_device_();
    create_swap_chain_();
    create_image_views_();
    create_render_pass_();
    create_descriptor_set_layout_();
    create_grahpics_pipeline_();
    create_framebuffers_();
    create_command_pool_();
    create_texture_image_();
    create_texture_image_view_();
    create_texture_sampler_();
    create_vertex_buffer_();
    create_index_buffer_();
    create_uniform_buffers_();
    create_descriptor_pool_();
    create_descriptor_sets_();
    create_command_buffers_();
    create_sync_objects_();
}

void instance::create_instance_()
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
}

instance::operator VkInstance()
{
    return instance_;
}

void instance::draw_frame()
{
    vkWaitForFences(logical_device_,
                    1,
                    std::addressof(in_flight_fences_[current_frame_]),
                    VK_TRUE,
                    UINT64_MAX);

    uint32_t image_index;
    VkResult result =
        vkAcquireNextImageKHR(logical_device_,
                              swap_chain_,
                              UINT64_MAX,
                              image_available_semaphores_[current_frame_],
                              VK_NULL_HANDLE,
                              std::addressof(image_index));
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreate_swap_chain_();
        return;
    }
    else if (result != VK_SUCCESS and result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error{"failed to acquire swap chain image!"};
    }
    vkResetFences(
        logical_device_, 1, std::addressof(in_flight_fences_[current_frame_]));

    vkResetCommandBuffer(command_buffers_[current_frame_], 0);
    record_command_buffer_(command_buffers_[current_frame_], image_index);

    update_uniform_buffer_(current_frame_);

    VkSubmitInfo submit_info{};
    submit_info.sType          = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    std::array wait_semaphores = {image_available_semaphores_[current_frame_]};
    std::array<VkPipelineStageFlags, 1> wait_stages = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores    = wait_semaphores.data();
    submit_info.pWaitDstStageMask  = wait_stages.data();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers =
        std::addressof(command_buffers_[current_frame_]);

    std::array signal_semaphores = {
        render_finished_semaphores_[current_frame_]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = signal_semaphores.data();

    if (vkQueueSubmit(graphics_queue_,
                      1,
                      std::addressof(submit_info),
                      in_flight_fences_[current_frame_]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR present_info{};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores.data();

    std::array swap_chains      = {swap_chain_};
    present_info.swapchainCount = 1;
    present_info.pSwapchains    = swap_chains.data();
    present_info.pImageIndices  = std::addressof(image_index);
    result = vkQueuePresentKHR(present_queue_, std::addressof(present_info));

    if (result == VK_ERROR_OUT_OF_DATE_KHR or result == VK_SUBOPTIMAL_KHR or
        framebuffer_resized)
    {
        framebuffer_resized = false;
        recreate_swap_chain_();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error{"failed to present swap chain image!"};
    }

    current_frame_ = (current_frame_ + 1) % max_frames_in_flight;
}

void instance::wait_device_idle()
{
    vkDeviceWaitIdle(logical_device_);
}

instance::~instance()
{
    cleanup_swap_chain_();

    vkDestroySampler(logical_device_, texture_sampler_, nullptr);
    vkDestroyImageView(logical_device_, texture_image_view_, nullptr);
    vkDestroyImage(logical_device_, texture_image_, nullptr);
    vkFreeMemory(logical_device_, texture_image_memory_, nullptr);
    std::ranges::for_each(
        std::views::zip(uniform_buffers_, uniform_buffers_memory_),
        [this](auto&& uniform) {
            const auto& [buffer, memory] = uniform;
            vkDestroyBuffer(logical_device_, buffer, nullptr);
            vkFreeMemory(logical_device_, memory, nullptr);
        });
    vkDestroyDescriptorPool(logical_device_, descriptor_pool_, nullptr);
    vkDestroyDescriptorSetLayout(
        logical_device_, descriptor_set_layout_, nullptr);
    vkDestroyBuffer(logical_device_, index_buffer_, nullptr);
    vkFreeMemory(logical_device_, index_buffer_memory_, nullptr);
    vkDestroyBuffer(logical_device_, vertex_buffer_, nullptr);
    vkFreeMemory(logical_device_, vertex_buffer_memory_, nullptr);

    vkDestroyPipeline(logical_device_, graphics_pipeline_, nullptr);
    vkDestroyPipelineLayout(logical_device_, pipeline_layout_, nullptr);
    vkDestroyRenderPass(logical_device_, render_pass_, nullptr);

    std::ranges::for_each(render_finished_semaphores_, [this](auto semaphore) {
        vkDestroySemaphore(logical_device_, semaphore, nullptr);
    });
    std::ranges::for_each(image_available_semaphores_, [this](auto semaphore) {
        vkDestroySemaphore(logical_device_, semaphore, nullptr);
    });
    std::ranges::for_each(in_flight_fences_, [this](auto fence) {
        vkDestroyFence(logical_device_, fence, nullptr);
    });

    vkDestroyCommandPool(logical_device_, command_pool_, nullptr);

    vkDestroyDevice(logical_device_, nullptr);

    if (validation_layers_enabled)
    {
        DestroyDebugUtilsMessengerEXT(*this, debug_messenger_, nullptr);
    }

    vkDestroySurfaceKHR(instance_, surface_, nullptr);

    vkDestroyInstance(*this, nullptr);
}

queue_family_indices instance::find_queue_families_(VkPhysicalDevice device)
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

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            device, i, surface_, std::addressof(present_support));
        if (present_support)
        {
            indices.present_family = i;
        }

        if (indices.is_complete())
        {
            break;
        }
        ++i;
    }

    return indices;
}

bool instance::is_physical_device_suitable_(VkPhysicalDevice device)
{
    auto qf_indices           = find_queue_families_(device);
    auto extensions_supported = check_device_extension_support_(device);

    bool swap_chain_adequate = false;
    if (extensions_supported)
    {
        auto swap_chain_support = query_swap_chain_support_(device);
        swap_chain_adequate     = not swap_chain_support.formats.empty() and
                              not swap_chain_support.present_modes.empty();
    }
    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(device, std::addressof(supported_features));
    return qf_indices.is_complete() and extensions_supported and
           swap_chain_adequate and supported_features.samplerAnisotropy;
}

VkSurfaceFormatKHR choose_swap_surface_format(
    const std::vector<VkSurfaceFormatKHR>& available_formats)
{
    for (const auto& available_format : available_formats)
    {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB and
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return available_format;
        }
    }
    return available_formats[0];
}

VkPresentModeKHR choose_swap_present_mode(
    const std::vector<VkPresentModeKHR>& available_present_modes)
{
    for (const auto& mode : available_present_modes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D instance::choose_swap_extent_(
    const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(
            window_.get(), std::addressof(width), std::addressof(height));
        VkExtent2D actual_extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height),
        };
        actual_extent.width  = std::clamp(actual_extent.width,
                                         capabilities.minImageExtent.width,
                                         capabilities.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height,
                                          capabilities.minImageExtent.height,
                                          capabilities.maxImageExtent.height);
        return actual_extent;
    }
}

auto get_available_device_extensions(VkPhysicalDevice device)
{
    uint32_t extensions_count{};
    vkEnumerateDeviceExtensionProperties(
        device, nullptr, std::addressof(extensions_count), nullptr);
    std::vector<VkExtensionProperties> available_extensions(extensions_count);
    vkEnumerateDeviceExtensionProperties(device,
                                         nullptr,
                                         std::addressof(extensions_count),
                                         available_extensions.data());
    return available_extensions;
}

bool instance::check_device_extension_support_(VkPhysicalDevice device)
{
    auto available_extensions = get_available_device_extensions(device);

    std::set<std::string> required_extensions(std::begin(device_extensions),
                                              std::end(device_extensions));

    for (const auto& extension : available_extensions)
    {
        required_extensions.erase(extension.extensionName);
    }
    return required_extensions.empty();
}

void instance::create_swap_chain_()
{
    auto swap_chain_support = query_swap_chain_support_(physical_device_);

    auto surface_format =
        choose_swap_surface_format(swap_chain_support.formats);
    auto present_mode =
        choose_swap_present_mode(swap_chain_support.present_modes);
    auto extent = choose_swap_extent_(swap_chain_support.capabilities);

    uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
    if (swap_chain_support.capabilities.maxImageCount > 0 and
        image_count > swap_chain_support.capabilities.maxImageCount)
    {
        image_count = swap_chain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{
        .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface          = surface_,
        .minImageCount    = image_count,
        .imageFormat      = surface_format.format,
        .imageExtent      = extent,
        .imageArrayLayers = 1,
        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    };
    queue_family_indices indices    = find_queue_families_(physical_device_);
    std::array queue_family_indices = {indices.graphics_family.value(),
                                       indices.present_family.value()};
    if (indices.graphics_family != indices.present_family)
    {
        create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices   = queue_family_indices.data();
    }
    else
    {
        create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices   = nullptr;
    }

    create_info.preTransform = swap_chain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode    = present_mode;
    create_info.clipped        = VK_TRUE;

    if (vkCreateSwapchainKHR(logical_device_,
                             std::addressof(create_info),
                             nullptr,
                             std::addressof(swap_chain_)) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(
        logical_device_, swap_chain_, std::addressof(image_count), nullptr);
    swap_chain_images_.resize(image_count);
    vkGetSwapchainImagesKHR(logical_device_,
                            swap_chain_,
                            std::addressof(image_count),
                            swap_chain_images_.data());

    swap_chain_image_format_ = surface_format.format;
    swap_chain_extent_       = extent;
}

void instance::create_image_views_()
{
    swap_chain_image_views_.resize(swap_chain_images_.size());
    for (size_t i = 0; i < swap_chain_images_.size(); ++i)
    {
        swap_chain_image_views_[i] =
            create_image_view_(swap_chain_images_[i], swap_chain_image_format_);
    }
}

struct vk_shader_module
{
    VkShaderModule module;
    VkDevice device;
    vk_shader_module(VkDevice device, const std::vector<std::byte>& code)
        : device{device}
    {
        VkShaderModuleCreateInfo create_info{};
        create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        create_info.pCode    = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(device,
                                 std::addressof(create_info),
                                 nullptr,
                                 std::addressof(module)) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }
    }

    ~vk_shader_module()
    {
        vkDestroyShaderModule(device, module, nullptr);
    }
};

void instance::create_grahpics_pipeline_()
{
    vk_shader_module vert_shader_module(
        logical_device_, load_binary_from_file("shaders/shader.vert.spv"));
    vk_shader_module frag_shader_module(
        logical_device_, load_binary_from_file("shaders/shader.frag.spv"));

    VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
    vert_shader_stage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = vert_shader_module.module;
    vert_shader_stage_info.pName  = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
    frag_shader_stage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = frag_shader_module.module;
    frag_shader_stage_info.pName  = "main";

    std::array shader_stages = {vert_shader_stage_info, frag_shader_stage_info};

    std::array dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,
                                 VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = to<uint32_t>(dynamic_states.size());
    dynamic_state.pDynamicStates    = dynamic_states.data();

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto binding_description    = vertex::get_binding_description();
    auto attribute_descriptions = vertex::get_attribute_descriptions();
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.vertexAttributeDescriptionCount =
        wf::to<uint32_t>(attribute_descriptions.size());
    vertex_input_info.pVertexBindingDescriptions =
        std::addressof(binding_description);
    vertex_input_info.pVertexAttributeDescriptions =
        attribute_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x        = 0.f;
    viewport.y        = 0.f;
    viewport.width    = static_cast<float>(swap_chain_extent_.width);
    viewport.height   = static_cast<float>(swap_chain_extent_.height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent_;

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports    = std::addressof(viewport);
    viewport_state.scissorCount  = 1;
    viewport_state.pScissors     = std::addressof(scissor);

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.f;
    rasterizer.depthBiasClamp          = 0.f;
    rasterizer.depthBiasSlopeFactor    = 0.f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0;
    multisampling.pSampleMask           = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    color_blend_attachment.blendEnable         = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable     = VK_FALSE;
    color_blending.logicOp           = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount   = 1;
    color_blending.pAttachments      = std::addressof(color_blend_attachment);
    color_blending.blendConstants[0] = 0.f;
    color_blending.blendConstants[1] = 0.f;
    color_blending.blendConstants[2] = 0.f;
    color_blending.blendConstants[3] = 0.f;

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = std::addressof(descriptor_set_layout_);
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges    = nullptr;

    if (vkCreatePipelineLayout(logical_device_,
                               std::addressof(pipeline_layout_info),
                               nullptr,
                               std::addressof(pipeline_layout_)) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages    = shader_stages.data();
    pipeline_info.pVertexInputState   = std::addressof(vertex_input_info);
    pipeline_info.pInputAssemblyState = std::addressof(input_assembly);
    pipeline_info.pViewportState      = std::addressof(viewport_state);
    pipeline_info.pRasterizationState = std::addressof(rasterizer);
    pipeline_info.pMultisampleState   = std::addressof(multisampling);
    pipeline_info.pDepthStencilState  = nullptr;
    pipeline_info.pColorBlendState    = std::addressof(color_blending);
    pipeline_info.pDynamicState       = std::addressof(dynamic_state);
    pipeline_info.layout              = pipeline_layout_;
    pipeline_info.renderPass          = render_pass_;
    pipeline_info.subpass             = 0;
    pipeline_info.basePipelineHandle  = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex   = -1;

    if (vkCreateGraphicsPipelines(logical_device_,
                                  VK_NULL_HANDLE,
                                  1,
                                  std::addressof(pipeline_info),
                                  nullptr,
                                  std::addressof(graphics_pipeline_)) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

void instance::create_render_pass_()
{
    VkAttachmentDescription color_attachment{};
    color_attachment.format         = swap_chain_image_format_;
    color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = std::addressof(color_attachment_ref);

    VkSubpassDependency dependency{};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments    = std::addressof(color_attachment);
    render_pass_info.subpassCount    = 1;
    render_pass_info.pSubpasses      = std::addressof(subpass);
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies   = std::addressof(dependency);

    if (vkCreateRenderPass(logical_device_,
                           std::addressof(render_pass_info),
                           nullptr,
                           std::addressof(render_pass_)) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void instance::create_framebuffers_()
{
    swap_chain_framebuffers_.resize(swap_chain_image_views_.size());
    for (size_t i = 0; i < swap_chain_image_views_.size(); i++)
    {
        std::array attachments = {swap_chain_image_views_[i]};

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType      = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass_;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments    = attachments.data();
        framebuffer_info.width           = swap_chain_extent_.width;
        framebuffer_info.height          = swap_chain_extent_.height;
        framebuffer_info.layers          = 1;

        if (vkCreateFramebuffer(logical_device_,
                                std::addressof(framebuffer_info),
                                nullptr,
                                std::addressof(swap_chain_framebuffers_[i])) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void instance::create_command_pool_()
{
    queue_family_indices queue_family_indices =
        find_queue_families_(physical_device_);

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

    if (vkCreateCommandPool(logical_device_,
                            std::addressof(pool_info),
                            nullptr,
                            std::addressof(command_pool_)) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void instance::create_command_buffers_()
{
    command_buffers_.resize(max_frames_in_flight);
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool_;
    alloc_info.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = wf::to<uint32_t>(command_buffers_.size());

    if (vkAllocateCommandBuffers(logical_device_,
                                 std::addressof(alloc_info),
                                 command_buffers_.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void instance::record_command_buffer_(VkCommandBuffer command_buffer,
                                      uint32_t image_index)
{
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags            = 0;
    begin_info.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(command_buffer, std::addressof(begin_info)) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass  = render_pass_;
    render_pass_info.framebuffer = swap_chain_framebuffers_[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = swap_chain_extent_;
    VkClearValue clear_color           = {0.f, 0.f, 0.f, 1.f};

    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues    = std::addressof(clear_color);
    vkCmdBeginRenderPass(command_buffer,
                         std::addressof(render_pass_info),
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(
        command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);

    std::array vertex_buffers           = {vertex_buffer_};
    std::array<VkDeviceSize, 1> offsets = {0};
    vkCmdBindVertexBuffers(
        command_buffer, 0, 1, vertex_buffers.data(), offsets.data());
    vkCmdBindIndexBuffer(
        command_buffer, index_buffer_, 0, VK_INDEX_TYPE_UINT16);

    VkViewport viewport{};
    viewport.x        = 0.f;
    viewport.y        = 0.f;
    viewport.width    = static_cast<float>(swap_chain_extent_.width);
    viewport.height   = static_cast<float>(swap_chain_extent_.height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    vkCmdSetViewport(command_buffer, 0, 1, std::addressof(viewport));

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent_;
    vkCmdSetScissor(command_buffer, 0, 1, std::addressof(scissor));

    vkCmdBindDescriptorSets(command_buffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout_,
                            0,
                            1,
                            std::addressof(descriptor_sets_[current_frame_]),
                            0,
                            nullptr);
    vkCmdDrawIndexed(
        command_buffer, wf::to<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffer);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void instance::create_sync_objects_()
{
    image_available_semaphores_.resize(max_frames_in_flight);
    render_finished_semaphores_.resize(max_frames_in_flight);
    in_flight_fences_.resize(max_frames_in_flight);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (auto&& [image_available_semaphore,
                 render_finished_semaphore,
                 in_flight_fence] : std::views::zip(image_available_semaphores_,
                                                    render_finished_semaphores_,
                                                    in_flight_fences_))
    {
        if (vkCreateSemaphore(logical_device_,
                              std::addressof(semaphore_info),
                              nullptr,
                              std::addressof(image_available_semaphore)) !=
                VK_SUCCESS or
            vkCreateSemaphore(logical_device_,
                              std::addressof(semaphore_info),
                              nullptr,
                              std::addressof(render_finished_semaphore)) !=
                VK_SUCCESS or
            vkCreateFence(logical_device_,
                          std::addressof(fence_info),
                          nullptr,
                          std::addressof(in_flight_fence)) != VK_SUCCESS)
        {
            throw std::runtime_error(
                "failed to create synchronization objects for a frame!");
        }
    }
}

void instance::recreate_swap_chain_()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(
        window_.get(), std::addressof(width), std::addressof(height));
    while (width = 0 || height == 0)
    {
        glfwGetFramebufferSize(
            window_.get(), std::addressof(width), std::addressof(height));
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(logical_device_);

    cleanup_swap_chain_();

    create_swap_chain_();
    create_image_views_();
    create_framebuffers_();
}

void instance::cleanup_swap_chain_()
{
    std::ranges::for_each(swap_chain_framebuffers_, [this](auto framebuffer) {
        vkDestroyFramebuffer(logical_device_, framebuffer, nullptr);
    });
    std::ranges::for_each(swap_chain_image_views_, [this](auto image_view) {
        vkDestroyImageView(logical_device_, image_view, nullptr);
    });
    vkDestroySwapchainKHR(logical_device_, swap_chain_, nullptr);
}

void instance::copy_buffer_(VkBuffer src_buffer,
                            VkBuffer dst_buffer,
                            VkDeviceSize size)
{
    VkCommandBuffer command_buffer = begin_single_time_commands_();

    VkBufferCopy copy_region{};
    copy_region.size = size;
    vkCmdCopyBuffer(
        command_buffer, src_buffer, dst_buffer, 1, std::addressof(copy_region));
    end_single_time_commands_(command_buffer);
}

void instance::create_index_buffer_()
{
    VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    create_buffer_(buffer_size,
                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                   staging_buffer,
                   staging_buffer_memory);

    void* data = nullptr;
    vkMapMemory(logical_device_,
                staging_buffer_memory,
                0,
                buffer_size,
                0,
                std::addressof(data));
    std::copy(
        std::begin(indices), std::end(indices), static_cast<uint16_t*>(data));
    vkUnmapMemory(logical_device_, staging_buffer_memory);

    create_buffer_(buffer_size,
                   VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                   index_buffer_,
                   index_buffer_memory_);

    copy_buffer_(staging_buffer, index_buffer_, buffer_size);

    vkDestroyBuffer(logical_device_, staging_buffer, nullptr);
    vkFreeMemory(logical_device_, staging_buffer_memory, nullptr);
}

void instance::create_descriptor_set_layout_()
{
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding            = 0;
    ubo_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount    = 1;
    ubo_layout_binding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding         = 1;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array bindings = {ubo_layout_binding, sampler_layout_binding};
    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = wf::to<uint32_t>(bindings.size());
    layout_info.pBindings    = bindings.data();

    if (vkCreateDescriptorSetLayout(logical_device_,
                                    std::addressof(layout_info),
                                    nullptr,
                                    std::addressof(descriptor_set_layout_)) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void instance::create_uniform_buffers_()
{
    [](auto&... vectors) {
        (vectors.resize(max_frames_in_flight), ...);
    }(uniform_buffers_, uniform_buffers_memory_, uniform_buffers_mapped_);

    auto uniforms = std::views::zip(
        uniform_buffers_, uniform_buffers_memory_, uniform_buffers_mapped_);

    std::ranges::for_each(uniforms, [this](auto&& uniform) {
        auto& [buffer, memory, map] = uniform;
        size_t buffer_size          = sizeof(uniform_buffer_object);
        create_buffer_(buffer_size,
                       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       buffer,
                       memory);
        vkMapMemory(
            logical_device_, memory, 0, buffer_size, 0, std::addressof(map));
    });
}

void instance::update_uniform_buffer_(uint32_t current_image)
{
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto current_time      = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     current_time - start_time)
                     .count();

    uniform_buffer_object ubo{};
    ubo.model = glm::rotate(
        glm::mat4(1.f), time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
    ubo.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f),
                           glm::vec3(0.f, 0.f, 0.f),
                           glm::vec3(0.f, 0.f, 1.f));
    ubo.proj =
        glm::perspective(glm::radians(45.f),
                         swap_chain_extent_.width /
                             static_cast<float>(swap_chain_extent_.height),
                         0.1f,
                         10.f);
    ubo.proj[1][1] *= -1;
    std::memcpy(uniform_buffers_mapped_[current_image],
                std::addressof(ubo),
                sizeof(ubo));
}

void instance::create_descriptor_pool_()
{
    std::array<VkDescriptorPoolSize, 2> pool_sizes{};
    pool_sizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = wf::to<uint32_t>(max_frames_in_flight);
    pool_sizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = wf::to<uint32_t>(max_frames_in_flight);

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = wf::to<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes    = pool_sizes.data();
    pool_info.maxSets       = wf::to<uint32_t>(max_frames_in_flight);
    if (vkCreateDescriptorPool(logical_device_,
                               std::addressof(pool_info),
                               nullptr,
                               std::addressof(descriptor_pool_)) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void instance::create_descriptor_sets_()
{
    std::vector<VkDescriptorSetLayout> layouts(max_frames_in_flight,
                                               descriptor_set_layout_);
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool_;
    alloc_info.descriptorSetCount = wf::to<uint32_t>(max_frames_in_flight);
    alloc_info.pSetLayouts        = layouts.data();

    descriptor_sets_.resize(max_frames_in_flight);
    if (vkAllocateDescriptorSets(logical_device_,
                                 std::addressof(alloc_info),
                                 descriptor_sets_.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < max_frames_in_flight; ++i)
    {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniform_buffers_[i];
        buffer_info.offset = 0;
        buffer_info.range  = sizeof(uniform_buffer_object);

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView   = texture_image_view_;
        image_info.sampler     = texture_sampler_;

        std::array<VkWriteDescriptorSet, 2> descriptor_writes{};
        descriptor_writes[0].sType  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = descriptor_sets_[i];
        descriptor_writes[0].dstBinding      = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo     = std::addressof(buffer_info);

        descriptor_writes[1].sType  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = descriptor_sets_[i];
        descriptor_writes[1].dstBinding      = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo      = std::addressof(image_info);

        vkUpdateDescriptorSets(logical_device_,
                               wf::to<uint32_t>(descriptor_writes.size()),
                               descriptor_writes.data(),
                               0,
                               nullptr);
    }
}

void instance::create_texture_image_()
{
    int tex_width, tex_height, tex_channels;
    auto pixels             = stbi_load("textures/statue-1275469.jpg",
                            std::addressof(tex_width),
                            std::addressof(tex_height),
                            std::addressof(tex_channels),
                            STBI_rgb_alpha);
    VkDeviceSize image_size = tex_width * tex_height * 4;

    if (not pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    create_buffer_(image_size,
                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                   staging_buffer,
                   staging_buffer_memory);
    void* data{};
    vkMapMemory(logical_device_,
                staging_buffer_memory,
                0,
                image_size,
                0,
                std::addressof(data));
    std::memcpy(data, pixels, wf::to<size_t>(image_size));
    vkUnmapMemory(logical_device_, staging_buffer_memory);
    stbi_image_free(pixels);

    create_image_(tex_width,
                  tex_height,
                  VK_FORMAT_R8G8B8A8_SRGB,
                  VK_IMAGE_TILING_OPTIMAL,
                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                  texture_image_,
                  texture_image_memory_);

    transition_image_layout_(texture_image_,
                             VK_FORMAT_R8G8B8A8_SRGB,
                             VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copy_buffer_to_image_(staging_buffer,
                          texture_image_,
                          wf::to<uint32_t>(tex_width),
                          wf::to<uint32_t>(tex_height));

    transition_image_layout_(texture_image_,
                             VK_FORMAT_R8G8B8A8_SRGB,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(logical_device_, staging_buffer, nullptr);
    vkFreeMemory(logical_device_, staging_buffer_memory, nullptr);
}

void instance::create_image_(uint32_t width,
                             uint32_t height,
                             VkFormat format,
                             VkImageTiling tiling,
                             VkImageUsageFlags usage,
                             VkMemoryPropertyFlags properties,
                             VkImage& image,
                             VkDeviceMemory& image_memory)
{
    VkImageCreateInfo image_info{};
    image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType     = VK_IMAGE_TYPE_2D;
    image_info.extent.width  = width;
    image_info.extent.height = height;
    image_info.extent.depth  = 1;
    image_info.mipLevels     = 1;
    image_info.arrayLayers   = 1;
    image_info.format        = VK_FORMAT_R8G8B8A8_SRGB;
    image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples     = VK_SAMPLE_COUNT_1_BIT;

    if (vkCreateImage(logical_device_,
                      std::addressof(image_info),
                      nullptr,
                      std::addressof(image)) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements mem_requirements{};
    vkGetImageMemoryRequirements(
        logical_device_, image, std::addressof(mem_requirements));

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize  = mem_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type_(
        mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (vkAllocateMemory(logical_device_,
                         std::addressof(alloc_info),
                         nullptr,
                         std::addressof(image_memory)) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate iamge memory!");
    }
    vkBindImageMemory(logical_device_, image, image_memory, 0);
}

VkCommandBuffer instance::begin_single_time_commands_()
{
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool_;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer{};
    vkAllocateCommandBuffers(logical_device_,
                             std::addressof(alloc_info),
                             std::addressof(command_buffer));
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, std::addressof(begin_info));

    return command_buffer;
}

void instance::end_single_time_commands_(VkCommandBuffer command_buffer)
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = std::addressof(command_buffer);

    vkQueueSubmit(
        graphics_queue_, 1, std::addressof(submit_info), VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue_);
    vkFreeCommandBuffers(
        logical_device_, command_pool_, 1, std::addressof(command_buffer));
}

void instance::transition_image_layout_(VkImage image,
                                        VkFormat format,
                                        VkImageLayout old_layout,
                                        VkImageLayout new_layout)
{
    VkCommandBuffer command_buffer = begin_single_time_commands_();

    VkImageMemoryBarrier barrier{};
    barrier.sType     = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;

    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = image;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED and
        new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage          = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage     = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL and
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source_stage          = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage     = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::invalid_argument{"unsupported layout transition!"};
    }

    vkCmdPipelineBarrier(command_buffer,
                         source_stage,
                         destination_stage,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         std::addressof(barrier));

    end_single_time_commands_(command_buffer);
}

void instance::copy_buffer_to_image_(VkBuffer buffer,
                                     VkImage image,
                                     uint32_t width,
                                     uint32_t height)
{
    VkCommandBuffer command_buffer = begin_single_time_commands_();

    VkBufferImageCopy region{};
    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset                     = {0, 0, 0};
    region.imageExtent                     = {width, height, 1};
    vkCmdCopyBufferToImage(command_buffer,
                           buffer,
                           image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           std::addressof(region));

    end_single_time_commands_(command_buffer);
}

void instance::create_texture_image_view_()
{
    texture_image_view_ =
        create_image_view_(texture_image_, VK_FORMAT_R8G8B8A8_SRGB);
}

VkImageView instance::create_image_view_(VkImage image, VkFormat format)
{
    VkImageViewCreateInfo view_info{};
    view_info.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image    = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format   = format;
    view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel   = 0;
    view_info.subresourceRange.levelCount     = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount     = 1;

    VkImageView image_view;
    if (vkCreateImageView(logical_device_,
                          std::addressof(view_info),
                          nullptr,
                          std::addressof(image_view)) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }

    return image_view;
}

void instance::create_texture_sampler_()
{
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter        = VK_FILTER_LINEAR;
    sampler_info.minFilter        = VK_FILTER_LINEAR;
    sampler_info.addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device_, std::addressof(properties));
    sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    sampler_info.borderColor   = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable           = VK_FALSE;
    sampler_info.compareOp               = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias              = 0.0f;
    sampler_info.minLod                  = 0.0f;
    sampler_info.maxLod                  = 0.f;

    if (vkCreateSampler(logical_device_,
                        std::addressof(sampler_info),
                        nullptr,
                        std::addressof(texture_sampler_)) != VK_SUCCESS)
    {
        throw std::runtime_error{"failed to create texture sampler!"};
    }
}

void instance::create_vertex_buffer_()
{
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    create_buffer_(buffer_size,
                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                   staging_buffer,
                   staging_buffer_memory);

    void* data = nullptr;
    vkMapMemory(logical_device_,
                staging_buffer_memory,
                0,
                buffer_size,
                0,
                std::addressof(data));
    std::copy(
        std::begin(vertices), std::end(vertices), static_cast<vertex*>(data));
    vkUnmapMemory(logical_device_, staging_buffer_memory);
    create_buffer_(buffer_size,
                   VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                   vertex_buffer_,
                   vertex_buffer_memory_);
    copy_buffer_(staging_buffer, vertex_buffer_, buffer_size);
    vkDestroyBuffer(logical_device_, staging_buffer, nullptr);
    vkFreeMemory(logical_device_, staging_buffer_memory, nullptr);
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

void instance::pick_physical_device_()
{
    uint32_t device_count{};
    vkEnumeratePhysicalDevices(
        instance_, std::addressof(device_count), nullptr);
    if (not device_count)
    {
        throw std::runtime_error{"failed to find GPUs with Vulkan support!"};
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(
        instance_, std::addressof(device_count), devices.data());

    if (auto potential_device_it =
            std::find_if(std::begin(devices),
                         std::end(devices),
                         [this](VkPhysicalDevice device) {
                             return is_physical_device_suitable_(device);
                         });
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

void instance::create_logical_device_()
{
    queue_family_indices indices = find_queue_families_(physical_device_);
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(),
                                                indices.present_family.value()};

    float queue_priority = 1.f;
    for (uint32_t queue_family : unique_queue_families)
    {
        VkDeviceQueueCreateInfo& queue_create_info =
            queue_create_infos.emplace_back();
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = indices.graphics_family.value();
        queue_create_info.queueCount       = 1;
        queue_create_info.pQueuePriorities = std::addressof(queue_priority);
    }

    VkPhysicalDeviceFeatures device_features{};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo create_info{};
    create_info.sType             = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.queueCreateInfoCount =
        static_cast<uint32_t>(queue_create_infos.size());
    create_info.pEnabledFeatures = std::addressof(device_features);
    create_info.enabledExtensionCount =
        static_cast<uint32_t>(device_extensions.size());
    create_info.ppEnabledExtensionNames = device_extensions.data();

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
    vkGetDeviceQueue(logical_device_,
                     indices.present_family.value(),
                     0,
                     std::addressof(present_queue_));
}

VkVertexInputBindingDescription vertex::get_binding_description()
{
    VkVertexInputBindingDescription binding_description{};
    binding_description.binding   = 0;
    binding_description.stride    = sizeof(vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding_description;
}

std::array<VkVertexInputAttributeDescription, 3> vertex::
    get_attribute_descriptions()
{
    std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions{};

    attribute_descriptions[0].binding  = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format   = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[0].offset   = offsetof(vertex, pos);

    attribute_descriptions[1].binding  = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset   = offsetof(vertex, color);

    attribute_descriptions[2].binding  = 0;
    attribute_descriptions[2].location = 2;
    attribute_descriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[2].offset   = offsetof(vertex, tex_coord);

    return attribute_descriptions;
}

uint32_t instance::find_memory_type_(uint32_t type_filter,
                                     VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device_,
                                        std::addressof(mem_properties));
    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; ++i)
    {
        std::bitset<32> bitset{type_filter};
        bool matching_properties =
            (mem_properties.memoryTypes[i].propertyFlags & properties) ==
            properties;
        if (bitset.test(i) and matching_properties)
        {
            return i;
        }
    }
    throw std::runtime_error{"failed to find suitable memory type!"};
}
void instance::create_buffer_(VkDeviceSize size,
                              VkBufferUsageFlags usage,
                              VkMemoryPropertyFlags properties,
                              VkBuffer& buffer,
                              VkDeviceMemory& buffer_memory)
{
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size        = size;
    buffer_info.usage       = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(logical_device_,
                       std::addressof(buffer_info),
                       nullptr,
                       std::addressof(buffer)) != VK_SUCCESS)
    {
        throw std::runtime_error{"failed to create buffer!"};
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(
        logical_device_, buffer, std::addressof(mem_requirements));
    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex =
        find_memory_type_(mem_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(logical_device_,
                         std::addressof(alloc_info),
                         nullptr,
                         std::addressof(buffer_memory)) != VK_SUCCESS)
    {
        throw std::runtime_error{"failed to allocate buffer memory!"};
    }
    vkBindBufferMemory(logical_device_, buffer, buffer_memory, 0);
}
} // namespace wf::vk
