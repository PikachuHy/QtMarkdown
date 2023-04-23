from conans import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake
from conan.tools.layout import cmake_layout


class MarkdownParserConan(ConanFile):
    name = "QtMarkdown"
    version = "0.1"

    # Optional metadata
    license = "<Put the package license here>"
    author = "PikachuHy <pikachuhy@163.com>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of Hello here>"
    topics = ("markdown", "parser", "<and here>")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False], "USE_CONAN_QT": [True, False]}
    default_options = {"shared": False, "fPIC": True, "USE_CONAN_QT": False}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*", "example/*", "assets/*", "MarkdownInstall.cmake"

    def requirements(self):
        # self.requires("MarkdownParser/0.1@demo/testing")
        self.requires("microtex/1.0.0@demo/testing")
        self.options['microtex'].QT = True
        self.options['microtex'].USE_CONAN_QT = self.options.USE_CONAN_QT
        self.options['microtex'].HAVE_LOG = False
        self.requires("magic_enum/0.7.3")
        if self.options.USE_CONAN_QT:
            self.requires("qt/6.2.3")
            self.options['qt'].shared = True

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        # This writes the "conan_toolchain.cmake"
        tc.generate()

        deps = CMakeDeps(self)
        # This writes all the config files (xxx-config.cmake)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.includedirs = [
            "include"
        ]
        self.cpp_info.libs = [
            "QtMarkdownParser",
            "QtMarkdownRender",
            "QtMarkdownEditorCore",
            "QtWidgetMarkdownEditor",
            "QtQuickMarkdownEditor"
        ]