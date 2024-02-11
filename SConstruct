import platform

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
        "third_party/vk-bootstrap/src",
        "third_party/fmt/include",
        "third_party/VulkanMemoryAllocator/include",
        "third_party/glm",
        "third_party/tinyobjloader",
    ],
    LIBS=["SDL2"],
)

os = platform.system().lower()
if os in ["linux", "darwin"]:
    env.Append(CXXFLAGS=["-std=c++17"])
    env.Append(CPPPATH=["/opt/homebrew/include"])
    env.Append(LIBPATH=[
        "third_party/fmt/build",
        "/opt/homebrew/lib",
    ])
    env.Append(LIBS=["vulkan", "fmt"])
if os == "windows":
    env.Append(CXXFLAGS=["/std:c++17", "/EHsc"])
    env.Append(CPPPATH=["e:/David/Software/VulkanSDK/1.3.275.0/Include/"])
    env.Append(LIBPATH=[
        "third_party/fmt/build/Debug/",
        "e:/David/Software/VulkanSDK/1.3.275.0/Lib/",
    ])
    env.Append(LIBS=["vulkan-1", "fmtd"])
    

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
