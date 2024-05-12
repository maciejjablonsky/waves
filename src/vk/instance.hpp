#pragma once
#include <array>
#include <set>
#include <string_view>
#include <utils.hpp>
#include <vector>
#include <vulkan/vulkan.h>
#include <window.hpp>

namespace wf::vk
{
using namespace std::string_view_literals;
constexpr std::array validation_layers = {"VK_LAYER_KHRONOS_validation"};
#ifdef NDEBUG
constexpr bool validation_layers_enabled = false;
#else
constexpr bool validation_layers_enabled = true;
#endif

constexpr std::array device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct queue_family_indices
{
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool is_complete() const
    {
        return graphics_family.has_value() and present_family.has_value();
    }
};

struct swap_chain_support_details
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

class instance : wf::non_copyable
{
  private:
    std::reference_wrapper<window> window_;
    VkInstance instance_                      = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_                     = VK_NULL_HANDLE;

    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice logical_device_          = VK_NULL_HANDLE;

    VkQueue graphics_queue_    = VK_NULL_HANDLE;
    VkQueue present_queue_     = VK_NULL_HANDLE;
    VkSwapchainKHR swap_chain_ = VK_NULL_HANDLE;
    std::vector<VkImage> swap_chain_images_;
    VkFormat swap_chain_image_format_;
    VkExtent2D swap_chain_extent_;
    std::vector<VkImageView> swap_chain_image_views_;

    void create_instance_();
    swap_chain_support_details query_swap_chain_support_(
        VkPhysicalDevice device);
    bool check_validation_layer_support_();
    void set_debug_messenger_();
    void create_surface_();
    void pick_physical_device_();
    void create_logical_device_();
    queue_family_indices find_queue_families_(VkPhysicalDevice device);

    bool is_physical_device_suitable_(VkPhysicalDevice device);
    VkExtent2D choose_swap_extent_(
        const VkSurfaceCapabilitiesKHR& capabilities);
    bool check_device_extension_support_(VkPhysicalDevice device);
    void create_swap_chain_();
    void create_image_views_();
    VkShaderModule create_shader_module_(const std::vector<std::byte>& code);
    void create_grahpics_pipeline_();

  public:
    instance(window& window);
    operator VkInstance();
    ~instance();
};
} // namespace wf::vk
