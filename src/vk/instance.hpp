#pragma once
#include <array>
#include <string_view>
#include <vulkan/vulkan.h>
#include <vector>
#include <utils.hpp>

namespace wf::vk
{
using namespace std::string_view_literals;
constexpr std::array validation_layers = {"VK_LAYER_KHRONOS_validation"};
#ifdef NDEBUG
constexpr bool validation_layers_enabled = false;
#else
constexpr bool validation_layers_enabled = true;
#endif

class instance : wf::non_copyable
{
  private:
    VkInstance instance_;
    VkDebugUtilsMessengerEXT debug_messenger_;

    bool check_validation_layer_support_();
    void set_debug_messenger_();

  public:
    instance();
    operator VkInstance();
    ~instance();
};
} // namespace wf::vk