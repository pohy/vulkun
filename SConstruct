import platform

AddOption(
    '--release',
    dest='release',
    action='store_true',
    help='Build in release mode',
    default=False,
)

is_release = GetOption('release')
os = platform.system().lower()

env = Environment(
    CPPPATH=[
        "third_party/vk-bootstrap/src",
        "third_party/fmt/include",
        "third_party/VulkanMemoryAllocator/include",
        "third_party/glm",
        "third_party/tinyobjloader",
        "third_party/imgui",
        "third_party/imgui/backends",
        "third_party/imgui/misc/cpp",
    ],
    LIBPATH="bin",
    LIBS=["SDL2", "imgui"],
)

if os == "darwin":
    default_flags = ["-std=c++20", "-Wall"]
    release_flags = ["-O3", "-DNDEBUG"]
    debug_flags = ["-g", "-O0"]

    flags = default_flags
    if is_release:
        flags = flags + release_flags
    else:
        flags = flags + debug_flags

    env.Append(CXXFLAGS=flags)
    env.Append(
        CPPPATH=[
            "/opt/homebrew/include",
            "/opt/homebrew/include/SDL2",  # For imgui which includes SDL as: #include <SDL.h>
        ]
    )
    env.Append(
        LIBPATH=[
            "third_party/fmt/build",
            "/opt/homebrew/lib",
        ]
    )
    env.Append(LIBS=["vulkan", "fmt"])
if os == "windows":
    env.Append(CXXFLAGS=["/std:c++20", "/EHsc", "/permissive"])
    env.Append(
        CPPPATH=[
            "e:/David/Software/VulkanSDK/1.3.275.0/Include/",
            "e:/David/Software/VulkanSDK/1.3.275.0/Include/SDL2/",
        ]
    )
    env.Append(
        LIBPATH=[
            "third_party/fmt/build/Debug/",
            "e:/David/Software/VulkanSDK/1.3.275.0/Lib/",
        ]
    )
    env.Append(LIBS=["vulkan-1", "fmtd"])

imgui_os_impl = "third_party/imgui/backends/imgui_impl_"
if os == "darwin":
    imgui_os_impl += "osx.mm"
if os == "windows":
    imgui_os_impl += "win32.cpp"

env.Library(
    "bin/imgui",
    [
        Glob("third_party/imgui/*.cpp"),
        "third_party/imgui/backends/imgui_impl_sdl2.cpp",
        "third_party/imgui/backends/imgui_impl_vulkan.cpp",
        "third_party/imgui/misc/cpp/imgui_stdlib.cpp",
        imgui_os_impl,
    ],
)

env.Tool("compilation_db")
env.CompilationDatabase()

bin_name = "vulkun"
if is_release:
    bin_name += "-release"

env.Program(
    "bin/" + bin_name,
    [
        Glob("src/*.cpp"),
        Glob("third_party/vk-bootstrap/src/*.cpp"),
        # Glob("third_party/imgui/*.cpp"),
        # "third_party/imgui/backends/imgui_impl_sdl2.cpp",
        # "third_party/imgui/backends/imgui_impl_vulkan.cpp",
        # imgui_os_impl,
    ],
)
