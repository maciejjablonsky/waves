#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <cstdlib>
#include <format>
#include <print>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <filesystem>

import glfw;

namespace wf
{
using namespace std::string_literals;
class app
{
  private:
    glfw glfw_instance_;

  public:
    app()
        : glfw_instance_({.window_width  = 800,
                          .window_height = 600,
                          .window_title  = "world simulator"s,
                          .fullscreen    = true})
    {
        while (not glfw_instance_.should_window_close())
        {
            glfwPollEvents();
        }
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
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
