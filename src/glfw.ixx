module;
#include <boost/container/flat_map.hpp>
#include <cassert>
#include <entt/entity/registry.hpp>
#include <format>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <string>

export module glfw;

import commands;
import callback_interfaces;
import utils;
import systems.keys;

namespace wf
{
auto make_glfw_keys_mapping()
{
    boost::container::flat_map<int, wf::systems::key> keys;
    keys[GLFW_KEY_ESCAPE] = systems::key::escape;

    return keys;
}

auto make_glfw_keys_action_mapping()
{
    boost::container::flat_map<int, wf::systems::key_state> states;
    states[GLFW_PRESS]   = systems::key_state::pressed;
    states[GLFW_RELEASE] = systems::key_state::released;
    return states;
}

const auto keys_mapping       = make_glfw_keys_mapping();
const auto keys_state_mapping = make_glfw_keys_action_mapping();

export struct glfw_create_info
{
    size_t window_width{};
    size_t window_height{};
    std::string window_title{};
    bool fullscreen = false;
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

        if (keys_mapping.contains(key))
        {
            self.key_handler_->handle_key(keys_mapping.at(key),
                                          keys_state_mapping.at(action));
        }
        else
        {
            throw std::invalid_argument(
                std::format("Unknown key pressed: {}", key));
        }
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