module;
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <string>

export module glfw;

import utils;

namespace wf
{
export struct glfw_create_info
{
    size_t window_width{};
    size_t window_height{};
    std::string window_title{};
    bool fullscreen{};
};

export class glfw
{
    GLFWwindow* window_{};

  public:
    explicit glfw(const glfw_create_info& info)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window_ = glfwCreateWindow(wf::to<int>(info.window_width),
                                   wf::to<int>(info.window_height),
                                   info.window_title.c_str(),
                                   info.fullscreen ? glfwGetPrimaryMonitor()
                                                   : nullptr,
                                   nullptr);
        if (not window_)
        {
            throw std::invalid_argument("glfwCreateWindow failed");
        }
    }

    [[nodiscard]] bool should_window_close() const noexcept
    {
        return glfwWindowShouldClose(window_);
    }

    ~glfw() noexcept
    {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }
};
} // namespace wf