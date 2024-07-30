module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module window;
import utils;

namespace wf
{
export class window : non_copyable
{
  private:
    GLFWwindow* window_ = nullptr;

  public:
    window();
    ~window();
    operator GLFWwindow*() const;
};
} // namespace wf
