env = Environment(parse_flags='-I/opt/homebrew/include')

env.Tool("compilation_db")
env.CompilationDatabase()

env.Program(
    "bin/vulkun",
    [Glob("src/*.cpp"), Glob("src/**/*.cpp")],
    LIBS=["SDL2"],
    LIBPATH=["/opt/homebrew/lib"],
)
