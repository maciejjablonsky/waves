#include <print>
#include <window.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <vk/instance.hpp>
#include <vk/device.hpp>

namespace wf
{
class app
{
  private:
    window window_;
    vk::instance vk_instance_;
    vk::device vk_physical_device_{vk_instance_};

  public:
    app()
    {
        //glm::mat4 matrix;
        //glm::vec4 vec;
        //auto test = matrix * vec;

        //while (!glfwWindowShouldClose(window_))
        //{
        //    glfwPollEvents();
        //}
    }
};
} // namespace wf

int main()
{
    try
    {
        wf::app app;
    }
    catch (const std::exception& e)
    {
        std::print("{}", e.what());
        return 1;
    }
    return 0;
}
