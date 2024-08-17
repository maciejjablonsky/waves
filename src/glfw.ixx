module;
#include <GLFW/glfw3.h>
#include <cassert>
#include <stdexcept>
#include <string>

export module glfw;

import callback_interfaces;
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

export class glfw : wf::non_copyable
{
    GLFWwindow* window_{};
    systems::ikey_handler* key_handler_;

    static glfw& get_self_from_glfw_(GLFWwindow* some_window)
    {
        auto self = static_cast<glfw*>(glfwGetWindowUserPointer(some_window));
        assert(self and "glfw window user pointer wasn't set");
        return *self;
    }

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
        glfwSetWindowUserPointer(window_, this);
    }

    static void key_callback(
        GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        auto& self = get_self_from_glfw_(window);
        self.key_handler_->handle_key(key, scancode, action, mods);
    }

    [[nodiscard]] bool should_window_close() const noexcept
    {
        return glfwWindowShouldClose(window_);
    }

    void register_key_handler(systems::ikey_handler& handler)
    {
        key_handler_ = std::addressof(handler);
        glfwSetKeyCallback(window_, key_callback);
    }

    void poll_events() const
    {
        glfwPollEvents();
    }

    ~glfw() noexcept
    {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }
};
} // namespace wf