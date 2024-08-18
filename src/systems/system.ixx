module;
#include <entt/entt.hpp>
export module system;

namespace wf::systems
{
export template <typename T>
concept system = requires(T& s, entt::registry& registry) {
    { s.update(registry) } -> std::same_as<void>;
};
} // namespace wf::systems