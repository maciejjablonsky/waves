from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
import os

required_conan_version = ">=2.0.0"

class waves_field(ConanFile):
    name = "waves_field"
    package_type = "application"

    license = "MIT"
    author = "Maciek Jablonski maciekjablonsky@gmail.com"
    topics = ("opengl", "sea", "simulation")

    settings = "os", "compiler", "build_type", "arch"

    exports_sources = "CMakeLists.txt", "src/*", "shaders/*"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("fmt/11.0.1")
        self.requires("glfw/3.3.8")
        self.requires("glm/cci.20230113")
        self.requires("rapidjson/cci.20230929")
        self.requires("entt/3.12.2")
        self.requires("tinyobjloader/2.0.0-rc10")
        self.requires("ms-gsl/4.0.0")
        # self.requires("mp-units/2.1.0")
        self.requires("magic_enum/0.9.5")
        self.requires("freetype/2.13.2")
        self.requires("boost/1.84.0")
        self.requires("range-v3/0.12.0")
        self.requires("vulkan-loader/1.3.268.0")
        self.requires("stb/cci.20240213")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)

        if self.settings.compiler == "msvc" and self.settings.build_type == 'Release':
            tc.extra_cxxflags.extend(['/O2', '/Oi', '/Ot', '/Oy', '/Ob2'])
            tc.extra_exelinkflags.extend(['/LTCG'])
        if self.settings.compiler == "clang":
            tc.extra_cxxflags.extend(['-Wno-deprecated-declarations'])
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
