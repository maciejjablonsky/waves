module;

#include <filesystem>
#include <fmt/format.h>
#include <glm/glm.hpp>
#include <source_location>

export module utils;

namespace wf
{

export void log(
    const std::string& message,
    const std::source_location& loc = std::source_location::current());

export std::string load_text_from_file(const std::filesystem::path& path);
export std::vector<std::byte> load_binary_from_file(
    const std::filesystem::path& path);
export struct non_copyable
{
    non_copyable(const non_copyable&)            = delete;
    non_copyable& operator=(const non_copyable&) = delete;
    non_copyable()                               = default;
    non_copyable(non_copyable&&)                 = default;
    non_copyable& operator=(non_copyable&&)      = default;
};
static_assert(std::default_initializable<non_copyable>);
static_assert(not std::copy_constructible<non_copyable>);
static_assert(std::move_constructible<non_copyable>);

export struct scoped_file
{
  private:
    std::FILE* fp_ = nullptr;

  public:
    explicit scoped_file(auto&&... args)
    {
#if defined(_WIN32)
        fopen_s(&fp_, std::forward<decltype(args)>(args)...);
#else
        fp_ = std::fopen(std::forward<decltype(args)>(args)...);
#endif
    }
    ~scoped_file();
    explicit operator std::FILE*() const;
    explicit operator bool() const;
};

export class unimplemented_error final : public std::exception
{
  private:
    std::string message_;

  public:
    explicit unimplemented_error(const std::string& message);
    [[nodiscard]] const char* what() const override;
};

export template <class... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};

export template <typename T>
using optional_ref = std::optional<std::reference_wrapper<T>>;

export template <typename T> constexpr bool is_in(T&& value, auto&& container)
{
    return std::find(std::begin(container),
                     std::end(container),
                     std::forward<T>(value)) != std::end(container);
}

export template <typename> struct is_tuple : std::false_type
{
};

export template <typename... T>
struct is_tuple<std::tuple<T...>> : std::true_type
{
};

export template <typename T>
constexpr inline bool is_tuple_v = is_tuple<T>::value;

export template <typename> struct function_traits;

export template <typename Function>
struct function_traits : public function_traits<decltype(&Function::operator())>
{
};

export template <typename Ret, typename Class, typename... Args>
struct function_traits<Ret (Class::*)(Args...) const>
{
    using result_type = Ret;

    template <std::size_t Index>
    using argument = std::tuple_element_t<Index, std::tuple<Args...>>;

    static const std::size_t arity = sizeof...(Args);
};

export template <typename Ret, typename... Args>
struct function_traits<Ret (*)(Args...)>
{
    using result_type = Ret;
    template <std::size_t Index>
    using argument = std::tuple_element_t<Index, std::tuple<Args...>>;
    static const std::size_t arity = sizeof...(Args);
};

export template <typename Ret, typename Class, typename... Args>
struct function_traits<Ret (Class::*)(Args...)>
{
    using result_type = Ret;
    template <std::size_t Index>
    using argument = std::tuple_element_t<Index, std::tuple<Args...>>;
    static const std::size_t arity = sizeof...(Args);
};

export template <std::floating_point T> constexpr bool is_equal(T&& a, T&& b)
{
    return std::abs(a - b) < std::numeric_limits<T>::epsilon() * std::abs(a);
}

export template <std::integral To, std::integral From> To to(From from)
{
    if constexpr (std::signed_integral<From> != std::signed_integral<To>)
    {
        if constexpr (std::signed_integral<From>)
        {
            assert(from >= 0 && static_cast<std::make_unsigned_t<From>>(from) <=
                                    std::numeric_limits<To>::max());
        }
        else
        {
            assert(from <= static_cast<From>(std::numeric_limits<To>::max()));
        }
    }
    else
    {
        if constexpr (sizeof(From) > sizeof(To))
        {
            assert(from >= std::numeric_limits<To>::min() &&
                   from <= std::numeric_limits<To>::max());
        }
    }
    return static_cast<To>(from);
}

export template <typename To, typename T> To size(const T& container) noexcept
{
    return wf::to<To>(std::size(container));
}
} // namespace wf

export template <>
struct fmt::formatter<glm::vec3> : fmt::formatter<std::string>
{
    static auto format(const glm::vec3& v, format_context& ctx)
    {
        auto it = ctx.out();
        it      = std::format_to(it, "[x: {}, y: {}, z: {}]", v.x, v.y, v.z);
        return it;
    }
};
