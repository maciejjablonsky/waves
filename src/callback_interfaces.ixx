export module callback_interfaces;

namespace wf::systems
{
export class ikey_handler
{
  public:
    virtual void handle_key(int key, int scancode, int action, int mods) = 0;
    virtual ~ikey_handler() noexcept = default;
};
} // namespace wf::systems