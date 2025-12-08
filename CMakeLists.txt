add_subdirectory ("main")
option(SIMPLESYNTH_WIN_CONSOLE "Build SimpleSynth Windows Console" ON)
option(SIMPLESYNTH_WIN_SIMPLE_TEST "Build SimpleSynth Windows SimpleTest" ON)
option(SIMPLESYNTH_WIN_IMGUI "Build SimpleSynth Lua Script" ON)
option(SIMPLESYNTH_WIN_IMGUI "Build SimpleSynth Windows ImGui" ON)
option(SIMPLESYNTH_VST "Build SimpleSynth VST Plugin" ON)
option(SIMPLESYNTH_FB2K_COMPONENT "Build SimpleSynth foobar2000 Component" ON)

if (SIMPLESYNTH_WIN_CONSOLE)
add_subdirectory ("test/Console")
endif ()

if (SIMPLESYNTH_WIN_SIMPLE_TEST)
add_subdirectory ("test/SimpleTest")
endif ()

if (SIMPLESYNTH_WIN_IMGUI)
add_subdirectory ("luasynth")
endif ()

if (SIMPLESYNTH_WIN_IMGUI)
add_subdirectory ("test/SimpleSynthImGUI")
endif ()

if (SIMPLESYNTH_VST)
add_subdirectory ("vst")
endif ()

if (SIMPLESYNTH_FB2K_COMPONENT)
add_subdirectory ("fb2kcomponent")
endif ()

if (CMAKE_USE_JNI)
add_subdirectory ("jni")
endif ()

