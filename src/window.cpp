module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

module window;

namespace wf
{
window::window()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
}

window::~window()
{
    glfwDestroyWindow(window_);
    glfwTerminate();
}

window::operator GLFWwindow*() const
{
    return window_;
}
} // namespace wf