module;
#include <entt/entt.hpp>
#include <magic_enum/magic_enum.hpp>
#include <print>
#include <variant>

export module systems.input;
import callback_interfaces;
import systems.pc_input;
import system;
import commands;

namespace wf::systems
{
export class input : public ikey_handler
{
  private:
    entt::entity settings_entity_ = entt::null;
    std::variant<pc_input> active_system_{
        std::in_place_type<pc_input>, pc_input::create_info{settings_entity_}};
    bool window_open_ = true;

    [[nodiscard]] bool is_pc_input_active_system_() const
    {
        return std::holds_alternative<pc_input>(active_system_);
    }

    void update_window_open_(entt::registry& ecs)
    {
        if (ecs.all_of<commands::exit>(settings_entity_))
        {
            window_open_ = false;
            ecs.erase<commands::exit>(settings_entity_);
        }
    }

  public:
    struct create_info
    {
        entt::entity settings_entity;
    };

    explicit input(const create_info& info)
        : settings_entity_(info.settings_entity)
    {
    }

    void update(entt::registry& registry)
    {
        std::visit(
            [&](systems::system auto& system) { system.update(registry); },
            active_system_);

        update_window_open_(registry);
    }

    void handle_key(key key, key_state state) override
    {
        if (not is_pc_input_active_system_())
        {
            active_system_.emplace<pc_input>(
                pc_input::create_info{settings_entity_});
        }
        auto& system = std::get<pc_input>(active_system_);
        system.consume_key(key, state);
    }

    [[nodiscard]] bool is_window_open() const
    {
        return window_open_;
    }
};
static_assert(system<input>);
} // namespace wf::systems