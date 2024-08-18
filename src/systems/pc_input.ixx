module;
#include <array>
#include <entt/entt.hpp>
#include <magic_enum/magic_enum.hpp>

export module systems.pc_input;
import system;
import systems.keys;
import commands;

namespace wf::systems
{
export class pc_input
{
    const entt::entity settings_entity_ = entt::null;
    std::array<systems::key_state, magic_enum::enum_count<systems::key>()>
        keys_states_;

    [[nodiscard]] key_state get_key_state_(key key) const
    {
        return keys_states_[*magic_enum::enum_index(key)];
    }

    [[nodiscard]] bool is_key_pressed_(key key) const
    {
        constexpr std::array options = {key_state::pressed, key_state::active};
        return std::ranges::contains(options, get_key_state_(key));
    }

    [[nodiscard]] bool is_key_just_pressed_(key key) const
    {
        constexpr std::array options = {key_state::pressed};
        return std::ranges::contains(options, get_key_state_(key));
    }

    void update_keys_states_()
    {
        std::ranges::for_each(keys_states_, [](key_state& state) {
            switch (state)
            {
            case key_state::pressed:
                state = key_state::active;
                break;
            case key_state::released:
                state = key_state::idle;
                break;
            default:
                break;
            }
        });
    }

    void update_app_exit_(entt::registry& ecs) const
    {
        if (is_key_just_pressed_(key::escape))
        {
            ecs.emplace_or_replace<commands::exit>(settings_entity_);
        }
    }

  public:
    struct create_info
    {
        entt::entity settings_entity;
    };

    explicit pc_input(const create_info& info)
        : settings_entity_(info.settings_entity)
    {
        std::ranges::fill(keys_states_, key_state::idle);
    }

    void consume_key(key key, key_state action)
    {
        keys_states_[*magic_enum::enum_index(key)] = action;
    }

    void update(entt::registry& registry)
    {
        update_app_exit_(registry);

        // at the end
        update_keys_states_();
    }
};
static_assert(system<pc_input>);
} // namespace wf::systems