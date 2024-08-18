#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <cstdlib>
#include <entt/entt.hpp>
#include <filesystem>
#include <format>
#include <print>

import glfw;
import systems;

namespace wf
{
using namespace std::string_literals;
class app
{
  public:
    app()
    {
        glfw glfw_instance({.window_width  = 800,
                            .window_height = 600,
                            .window_title  = "worlds simulator"s,
                            .fullscreen    = false});

        entt::registry ecs;
        entt::entity settings = ecs.create();

        systems::input input_system({settings});
        glfw_instance.register_key_handler(input_system);

        while (input_system.is_window_open())
        {
            glfw_instance.poll_events();
            input_system.update(ecs);
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
