env = Environment(
    CPPPATH=["/opt/homebrew/include", "third_party/vk-bootstrap/src"],
    LIBPATH=["/opt/homebrew/lib"],
    CXXFLAGS=["-std=c++17"],
)
    

env.Tool("compilation_db")
env.CompilationDatabase()

env.Program(
    "bin/vulkun",
    [Glob("src/*.cpp"), Glob("src/**/*.cpp")],
    LIBS=["SDL2"],
)
