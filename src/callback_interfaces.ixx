export module callback_interfaces;
export import systems.keys;

namespace wf::systems
{
export class ikey_handler
{
  public:
    virtual void handle_key(key key, key_state state) = 0;
    virtual ~ikey_handler() noexcept                  = default;
};
} // namespace wf::systems