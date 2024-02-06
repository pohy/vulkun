# TODO: Building fmt requires a manual build, ugh. It uses C++ modules which is highly incompatible. W/e, ugh
# Library(
#     'fmt',
#     Glob('third_party/fmt/src/*.cc'),
#     CPPPATH=['third_party/fmt/include'],
#     CXXFLAGS=["-std=c++20", "-fmodules-ts"],
#     CPPDEFINES=["FMT_ATTACH_TO_GLOBAL_MODULE"],
# )

env = Environment(
    CPPPATH=[
        "/opt/homebrew/include",
        "third_party/vk-bootstrap/src",
        "third_party/fmt/include",
    ],
    LIBPATH=[
        "/opt/homebrew/lib",
        "./third_party/fmt/build",
    ],
    CXXFLAGS=["-std=c++17"],
    LIBS=["SDL2", "vulkan", "fmt"],
)
    

env.Tool("compilation_db")
env.CompilationDatabase()

env.Program(
    "bin/vulkun",
    [
        Glob("src/*.cpp"),
        Glob("src/**/*.cpp"),
        # Glob("third_party/*.cpp"),
        Glob("third_party/vk-bootstrap/src/*.cpp"),
        # Glob("third_party/fmt/src/*.cc"),
    ],
)
