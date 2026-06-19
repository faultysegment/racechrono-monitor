Import("env")
import platform

system = platform.system()

if system == "Windows":
    print("Configuring SDL2 for Windows (MSYS2 UCRT64 assumed)")
    env.Append(CPPPATH=["C:/msys64/ucrt64/include"])
    env.Append(LIBPATH=["C:/msys64/ucrt64/lib"])
    env.Append(LIBS=["mingw32", "SDL2main", "SDL2", "SDL2_ttf"])
else:
    print(f"Configuring SDL2 for {system} via pkg-config")
    env.ParseConfig("pkg-config --cflags --libs sdl2 SDL2_ttf")

env.AddCustomTarget(
    name="run",
    dependencies="$BUILD_DIR/${PROGNAME}",
    actions=[
        '"$BUILD_DIR/${PROGNAME}"'
    ],
    title="Run Simulator",
    description="Run the native SDL2 simulator"
)
