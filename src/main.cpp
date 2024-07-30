#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <cstdlib>
#include <format>
#include <print>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <filesystem>

import vk;
import window;

namespace wf
{
class app
{
  private:
    window window_;
    vk::instance vk_instance_{window_};

  public:
    app()
    {
        while (!glfwWindowShouldClose(window_))
        {
            glfwPollEvents();
            draw_frame();
        }
        vk_instance_.wait_device_idle();
    }

    void draw_frame()
    {
        vk_instance_.draw_frame();
    }
};
} // namespace wf

int main()
{
    try
    {
        if (const char* cwd = std::getenv("WAVES_FIELD_WORKING_DIR"))
        {
            std::println("Switching current working directory to {}", cwd);
            std::filesystem::current_path(cwd);
        }
        std::println("Current working directory: {}",
                     std::filesystem::current_path().string());

        wf::app app;
    }
    catch (const std::exception& e)
    {
        std::print("{}", e.what());
        return 1;
    }
    return 0;
}
