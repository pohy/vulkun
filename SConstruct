env = Environment(
    CPPPATH=["/opt/homebrew/include", "third_party/vk-bootstrap/src"],
    LIBPATH=["/opt/homebrew/lib"],
    CXXFLAGS=["-std=c++17"],
    LIBS=["SDL2", "vulkan"],
)
    

env.Tool("compilation_db")
env.CompilationDatabase()

env.Program(
    "bin/vulkun",
    [
        Glob("src/*.cpp"),
        Glob("src/**/*.cpp"),
        Glob("third_party/vk-bootstrap/src/*.cpp")
    ],
)
