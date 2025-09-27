project ("SimpleSynthImGUI")

set(SDL2_DIR "${REFERENCE_DIR}/SDL2-2.0.16")
set(IMGUI_DIR "${REFERENCE_DIR}/imgui-1.91.5")
set(IMGUI_FILEDLG_DIR "${REFERENCE_DIR}/ImGuiFileDialog-0.6.7")
set(IMPLOT_DIR "${REFERENCE_DIR}/implot-master")
set(IMGUI_KNOBS_DIR "${REFERENCE_DIR}/imgui-knobs")
set(JSON_DIR "${REFERENCE_DIR}/json")
set(IMGUI_BACKENDS_DIR "${IMGUI_DIR}/backends")


add_executable (${PROJECT_NAME} "main.cpp"
"${IMGUI_BACKENDS_DIR}/imgui_impl_sdl2.cpp"
"${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp"
"${IMGUI_DIR}/imgui.cpp"
"${IMGUI_DIR}/imgui_draw.cpp"
"${IMGUI_DIR}/imgui_tables.cpp"
"${IMGUI_DIR}/imgui_widgets.cpp"
"${IMGUI_DIR}/imgui_demo.cpp"
"${IMPLOT_DIR}/implot.cpp"
"${IMPLOT_DIR}/implot_items.cpp"
"${IMGUI_FILEDLG_DIR}/ImGuiFileDialog.cpp"
"${IMGUI_KNOBS_DIR}/imgui-knobs.cpp"
 "SimpleSynthWindow.cpp"
 "ProjectObject.cpp"
 "CurrentProjectContext.cpp"
 "ProjectWindowSelect.cpp"
 "NotificationManager.cpp"
 "RegisterAllModules.cpp"
 "Shortcuts.cpp"
 "Serialize.cpp"
 "RightContextMenu.cpp"
 "sakura/SakuraRender.h"
 "sakura/SakuraRender.cpp"
 "piano/PianoRender.h"
 "piano/PianoRender.cpp"
 "sample/SampleRender.h"
 "sample/SampleRender.cpp"
 "dsprender/DSPRender.h" 
 "dsprender/CompressorRender.cpp" 
 "interpolatorrender/InterpolatorRender.h" 
 "interpolatorrender/GraphInterpolatorRender.cpp"
 "dsprender/EnvelopDetectorRender.cpp"
 "ProjectObject.h"
 "sswindow/EQWindow.cpp"
 "LangToEnum.h"
 "sswindow/OscilloscopeWindow.cpp"
 "sswindow/ChannelSettingWindow.cpp"
 "sswindow/MixerSettingWindow.cpp"
"notesrcrender/FM6x6Render.cpp"
 "notesrcrender/NoteSrcRender.h" "notesrcrender/MultiStageEnvRender.cpp")

target_include_directories(${PROJECT_NAME} PRIVATE "${SDL2_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${IMGUI_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${IMGUI_FILEDLG_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${IMGUI_KNOBS_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${IMPLOT_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${JSON_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${IMGUI_BACKENDS_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/yzrutil/main")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/SimpleSynth/main")

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package(OpenGL REQUIRED)

target_link_directories(${PROJECT_NAME} PRIVATE "${SDL2_DIR}/lib/x64")
target_link_libraries(${PROJECT_NAME} PRIVATE yzrutil)
target_link_libraries(${PROJECT_NAME} PRIVATE SimpleSynth)
target_link_libraries(${PROJECT_NAME} PRIVATE "${SDL2_DIR}/lib/x64/SDL2.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)