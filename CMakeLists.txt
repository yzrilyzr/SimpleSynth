add_subdirectory ("main")
option(SIMPLESYNTH_WIN_CONSOLE "Build SimpleSynth Windows Console" ON)
option(SIMPLESYNTH_WIN_IMGUI "Build SimpleSynth Windows ImGui" ON)
option(SIMPLESYNTH_VST "Build SimpleSynth VST Plugin" ON)

if (SIMPLESYNTH_WIN_CONSOLE)
add_subdirectory ("test/Console")
endif ()

if (SIMPLESYNTH_WIN_IMGUI)
add_subdirectory ("test/SimpleSynthImGUI")
endif ()

if (SIMPLESYNTH_VST)
add_subdirectory ("vst")
endif ()

if (CMAKE_USE_JNI)
add_subdirectory ("jni")
endif ()

