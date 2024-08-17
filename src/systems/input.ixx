module;
#include <entt/entt.hpp>
#include <print>
export module systems.input;
import callback_interfaces;

namespace wf::systems
{
export class input : public ikey_handler
{
  public:
    struct create_info
    {
    };

    void update(entt::registry& registry)
    {
    }

    void handle_key(int key, int scancode, int action, int mods) override
    {
        std::println("pressed {}", key);
    }
};
} // namespace wf::systems