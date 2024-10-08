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
struct uniform_buffer_object
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

export struct vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 tex_coord;

    static VkVertexInputBindingDescription get_binding_description();
    static std::array<VkVertexInputAttributeDescription, 3>
    get_attribute_descriptions();
    bool operator==(const vertex& other) const;
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

    VkPhysicalDevice physical_device_   = VK_NULL_HANDLE;
    VkSampleCountFlagBits msaa_samples_ = VK_SAMPLE_COUNT_1_BIT;
    VkDevice logical_device_            = VK_NULL_HANDLE;

    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    VkQueue present_queue_  = VK_NULL_HANDLE;

    VkSwapchainKHR swap_chain_ = VK_NULL_HANDLE;
    std::vector<VkImage> swap_chain_images_;
    VkFormat swap_chain_image_format_;
    VkExtent2D swap_chain_extent_;
    std::vector<VkImageView> swap_chain_image_views_;
    std::vector<VkFramebuffer> swap_chain_framebuffers_;

    VkRenderPass render_pass_;
    VkDescriptorSetLayout descriptor_set_layout_;
    VkPipelineLayout pipeline_layout_;
    VkPipeline graphics_pipeline_;

    VkCommandPool command_pool_;

    VkImage color_image_;
    VkDeviceMemory color_image_memory_;
    VkImageView color_image_view_;

    VkImage depth_image_;
    VkDeviceMemory depth_image_memory_;
    VkImageView depth_image_view_;

    uint32_t mip_levels_{};
    VkImage texture_image_{};
    VkDeviceMemory texture_image_memory_{};
    VkImageView texture_image_view_{};
    VkSampler texture_sampler_{};

    std::vector<vertex> vertices_;
    std::vector<uint32_t> indices_;
    VkBuffer vertex_buffer_;
    VkDeviceMemory vertex_buffer_memory_;
    VkBuffer index_buffer_;
    VkDeviceMemory index_buffer_memory_;

    std::vector<VkBuffer> uniform_buffers_;
    std::vector<VkDeviceMemory> uniform_buffers_memory_;
    std::vector<void*> uniform_buffers_mapped_;

    VkDescriptorPool descriptor_pool_;
    std::vector<VkDescriptorSet> descriptor_sets_;

    std::vector<VkCommandBuffer> command_buffers_;

    std::vector<VkSemaphore> image_available_semaphores_;
    std::vector<VkSemaphore> render_finished_semaphores_;
    std::vector<VkFence> in_flight_fences_;
    uint32_t current_frame_ = 0;

    void create_instance_();
    swap_chain_support_details query_swap_chain_support_(
        VkPhysicalDevice device) const;
    bool check_validation_layer_support_();
    void set_debug_messenger_();
    void create_surface_();
    void pick_physical_device_();
    void create_logical_device_();
    queue_family_indices find_queue_families_(VkPhysicalDevice device);

    bool is_device_suitable_(VkPhysicalDevice device);
    VkExtent2D choose_swap_extent_(
        const VkSurfaceCapabilitiesKHR& capabilities);
    bool check_device_extension_support_(VkPhysicalDevice device);
    void create_swap_chain_();
    void create_image_views_();
    void create_graphics_pipeline_();
    void create_render_pass_();
    void create_framebuffers_();
    void create_command_pool_();
    void create_command_buffers_();
    void record_command_buffer_(VkCommandBuffer command_buffer,
                                uint32_t image_index);
    void create_sync_objects_();
    void recreate_swap_chain_();
    void cleanup_swap_chain_();
    void create_vertex_buffer_();
    uint32_t find_memory_type_(uint32_t type_filter,
                               VkMemoryPropertyFlags properties);
    void create_buffer_(VkDeviceSize size,
                        VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties,
                        VkBuffer& buffer,
                        VkDeviceMemory& buffer_memory);

    void copy_buffer_(VkBuffer src_buffer,
                      VkBuffer dst_buffer,
                      VkDeviceSize size);
    void create_index_buffer_();
    void create_descriptor_set_layout_();
    void create_uniform_buffers_();
    void update_uniform_buffer_(uint32_t current_image);
    void create_descriptor_pool_();
    void create_descriptor_sets_();
    void create_texture_image_();
    void create_image_(uint32_t width,
                       uint32_t heigth,
                       uint32_t mip_levels,
                       VkSampleCountFlagBits num_samples,
                       VkFormat format,
                       VkImageTiling tiling,
                       VkImageUsageFlags usage,
                       VkMemoryPropertyFlags properties,
                       VkImage& image,
                       VkDeviceMemory& image_memory);
    VkCommandBuffer begin_single_time_commands_();
    void end_single_time_commands_(VkCommandBuffer command_buffer);
    void transition_image_layout_(VkImage image,
                                  VkFormat format,
                                  VkImageLayout old_layout,
                                  VkImageLayout new_layout,
                                  uint32_t mip_levels);
    void copy_buffer_to_image_(VkBuffer buffer,
                               VkImage image,
                               uint32_t width,
                               uint32_t height);
    void create_texture_image_view_();
    VkImageView create_image_view_(VkImage image,
                                   VkFormat format,
                                   VkImageAspectFlags aspect_flags,
                                   uint32_t mip_levels);
    void create_texture_sampler_();
    void create_depth_resources_();
    VkFormat find_depth_format_();
    VkFormat find_supported_format_(const std::vector<VkFormat>& candidates,
                                    VkImageTiling tiling,
                                    VkFormatFeatureFlags features);
    void load_model_();
    void generate_mip_maps_(VkImage image,
                            VkFormat image_format,
                            int32_t tex_widht,
                            int32_t tex_height,
                            uint32_t mip_levels);
    void create_color_resources_();
    VkSampleCountFlagBits get_max_usable_sample_count_();

  public:
    bool framebuffer_resized = false;
    instance(window& window);
    operator VkInstance();
    void draw_frame();
    void wait_device_idle();
    ~instance();
};
} // namespace wf::vk
