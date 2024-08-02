cmake_minimum_required(VERSION 3.29.6 FATAL_ERROR)
project(waves_field CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(shaders)

add_executable(waves_field)
add_dependencies(waves_field wf_shaders)
target_compile_features(waves_field PUBLIC cxx_std_23)

target_sources(waves_field
    PUBLIC
        src/main.cpp
        src/window.cpp
        src/vk/instance.cpp 
        src/utils.cpp
    PUBLIC FILE_SET CXX_MODULES FILES
        src/utils.ixx
        src/window.ixx
        src/vk.ixx
)

find_package(glfw3 REQUIRED CONFIG)
find_package(glm REQUIRED CONFIG)
find_package(VulkanLoader REQUIRED CONFIG)
find_package(magic_enum REQUIRED CONFIG)
find_package(fmt REQUIRED CONFIG)
find_package(stb REQUIRED CONFIG)
find_package(tinyobjloader REQUIRED CONFIG)
target_link_libraries(waves_field
    PUBLIC
        glfw
        glm::glm
        Vulkan::Loader
        magic_enum::magic_enum
        fmt::fmt
        stb::stb
        tinyobjloader::tinyobjloader
)

install(TARGETS waves_field DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
)
