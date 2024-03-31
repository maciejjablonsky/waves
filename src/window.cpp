#include <window.hpp>

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
window::operator GLFWwindow*()
{
    return window_;
}
} // namespace wf