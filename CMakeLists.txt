cmake_minimum_required(VERSION 3.15)
project(waves_field CXX)

add_subdirectory(shaders)

add_executable(waves_field)
add_dependencies(waves_field wf_shaders)

target_sources(waves_field
    PUBLIC
        src/main.cpp
        src/window.cpp
        src/vk/instance.cpp
        src/utils.cpp
    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS src
    FILES
        src/window.hpp
        src/vk/instance.hpp
        src/utils.hpp
)

find_package(glfw3 REQUIRED CONFIG)
find_package(glm REQUIRED CONFIG)
find_package(VulkanLoader REQUIRED CONFIG)
find_package(magic_enum REQUIRED CONFIG)
find_package(fmt REQUIRED CONFIG)
target_link_libraries(waves_field
    PUBLIC
        glfw
        glm::glm
        Vulkan::Loader
        magic_enum::magic_enum
        fmt::fmt
)

install(TARGETS waves_field DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
)
