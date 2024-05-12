#pragma once
#include <concepts>
#include <cstdio>
#include <filesystem>
#include <fmt/format.h>
#include <glm/glm.hpp>
#include <source_location>
#include <string>
#include <type_traits>
#include <utility>

namespace wf
{

void log(const std::string& message,
         const std::source_location& loc = std::source_location::current());

std::string load_text_from_file(const std::filesystem::path& path);
std::vector<std::byte> load_binary_from_file(const std::filesystem::path& path);

struct non_copyable
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

struct scoped_file
{
  private:
    std::FILE* fp_;

  public:
    scoped_file(auto&&... args)
    {
        fp_ = std::fopen(std::forward<decltype(args)>(args)...);
    }
    ~scoped_file();
    operator std::FILE*();
    operator bool();
};

class unimplemented_error : public std::exception
{
  private:
    std::string message_;

  public:
    unimplemented_error(const std::string& message);
    const char* what() const;
};

template <class... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};

template <typename T>
using optional_ref = std::optional<std::reference_wrapper<T>>;

template <typename T> constexpr bool is_in(T&& value, auto&& container)
{
    return std::find(std::begin(container),
                     std::end(container),
                     std::forward<T>(value)) != std::end(container);
}

template <typename> struct is_tuple : std::false_type
{
};

template <typename... T> struct is_tuple<std::tuple<T...>> : std::true_type
{
};

template <typename T> constexpr inline bool is_tuple_v = is_tuple<T>::value;

template <typename> struct function_traits;

template <typename Function>
struct function_traits : public function_traits<decltype(&Function::operator())>
{
};

template <typename Ret, typename Class, typename... Args>
struct function_traits<Ret (Class::*)(Args...) const>
{
    using result_type = Ret;

    template <std::size_t Index>
    using argument =
        typename std::tuple_element<Index, std::tuple<Args...>>::type;

    static const std::size_t arity = sizeof...(Args);
};

template <typename Ret, typename... Args>
struct function_traits<Ret (*)(Args...)>
{
    using result_type = Ret;
    template <std::size_t Index>
    using argument =
        typename std::tuple_element<Index, std::tuple<Args...>>::type;
    static const std::size_t arity = sizeof...(Args);
};

template <typename Ret, typename Class, typename... Args>
struct function_traits<Ret (Class::*)(Args...)>
{
    using result_type = Ret;
    template <std::size_t Index>
    using argument =
        typename std::tuple_element<Index, std::tuple<Args...>>::type;
    static const std::size_t arity = sizeof...(Args);
};

template <std::floating_point T> constexpr bool is_equal(T&& a, T&& b)
{
    return std::abs(a - b) < std::numeric_limits<T>::epsilon() * std::abs(a);
}

template <std::integral To, std::integral From> To to(From&& from)
{
    // TODO: add runtime checks in case signed/unsigned conversions
	return static_cast<To>(from);
}
} // namespace wf

template <> struct fmt::formatter<glm::vec3> : fmt::formatter<std::string>
{
    auto format(const glm::vec3& v, format_context& ctx) const
    {
        auto it = ctx.out();
        it      = std::format_to(it, "[x: {}, y: {}, z: {}]", v.x, v.y, v.z);
        return it;
    }
};
