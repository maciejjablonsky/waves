cmake_minimum_required(VERSION 3.15)
project(waves_field CXX)

find_package(glfw3 REQUIRED CONFIG)
find_package(fmt REQUIRED CONFIG)
find_package(glew REQUIRED CONFIG)
find_package(glm REQUIRED CONFIG)
find_package(RapidJSON REQUIRED CONFIG)
find_package(EnTT REQUIRED CONFIG)
find_package(tinyobjloader REQUIRED CONFIG)
find_package(Microsoft.GSL REQUIRED CONFIG)
find_package(magic_enum REQUIRED CONFIG)
find_package(boost REQUIRED CONFIG)
find_package(freetype REQUIRED CONFIG)
find_package(range-v3 REQUIRED CONFIG)

add_executable(waves_field)
target_sources(waves_field PUBLIC

    PUBLIC
    FILE_SET HEADERS 
    BASE_DIRS src
    FILES

)

target_link_libraries(waves_field PRIVATE fmt::fmt glfw glm::glm rapidjson EnTT::EnTT tinyobjloader::tinyobjloader Microsoft.GSL::GSL magic_enum::magic_enum Boost::headers Freetype::Freetype range-v3::range-v3 GLEW::GLEW)


install(TARGETS waves_field DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        )
