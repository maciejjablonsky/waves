#pragma once
#include <utils.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wf
{
class window : non_copyable
{
  private:
    GLFWwindow* window_ = nullptr;
  public:
    window();
    ~window();
    operator GLFWwindow*();
};
} // namespace wf