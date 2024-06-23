module;
#include <array>
#include <glm/glm.hpp>
#include <optional>
#include <string_view>
#include <vector>
#include <vulkan/vulkan.h>

export module vk;

import window;
import utils;

namespace wf::vk
{
export struct vertex
{
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription get_binding_description();
    static std::array<VkVertexInputAttributeDescription, 2>
    get_attribute_descriptions();
};
static_assert(std::is_standard_layout_v<vertex>,
              "vertex must be standard layout");

using namespace std::string_view_literals;
constexpr std::array validation_layers = {"VK_LAYER_KHRONOS_validation"};
#ifdef NDEBUG
constexpr bool validation_layers_enabled = false;
#else
constexpr bool validation_layers_enabled = true;
#endif

constexpr std::array device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
constexpr int max_frames_in_flight     = 2;

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

export class instance : wf::non_copyable
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

    VkRenderPass render_pass_;
    VkPipelineLayout pipeline_layout_;
    VkPipeline graphics_pipeline_;
    std::vector<VkFramebuffer> swap_chain_framebuffers_;
    VkCommandPool command_pool_;
    std::vector<VkCommandBuffer> command_buffers_;

    std::vector<VkSemaphore> image_available_semaphores_;
    std::vector<VkSemaphore> render_finished_semaphores_;
    std::vector<VkFence> in_flight_fences_;
    uint32_t current_frame_ = 0;

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
    void create_grahpics_pipeline_();
    void create_render_pass_();
    void create_framebuffers_();
    void create_command_pool_();
    void create_command_buffers_();
    void record_command_buffer_(VkCommandBuffer command_buffer,
                                uint32_t image_index);
    void create_sync_objects_();
    void recreate_swap_chain_();
    void cleanup_swap_chain_();

  public:
    bool framebuffer_resized = false;
    instance(window& window);
    operator VkInstance();
    void draw_frame();
    void wait_device_idle();
    ~instance();
};
} // namespace wf::vk
