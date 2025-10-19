project ("SimpleSynthConsole")

set(SDL2_DIR "${REFERENCE_DIR}/SDL2-2.0.16")
add_executable (${PROJECT_NAME} "main.cpp")

target_include_directories(${PROJECT_NAME} PRIVATE
"${SDL2_DIR}/include"
"${CMAKE_SOURCE_DIR}/yzrutil/main"
"${CMAKE_SOURCE_DIR}/SimpleDSP/main"
"${CMAKE_SOURCE_DIR}/SimpleSynth/main"
)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

target_link_directories(${PROJECT_NAME} PRIVATE "${SDL2_DIR}/lib/x64")
target_link_libraries(${PROJECT_NAME} PRIVATE yzrutil SimpleDSP SimpleSynth "${SDL2_DIR}/lib/x64/SDL2.lib")

if(WIN32)
  target_sources(${PROJECT_NAME} PRIVATE win32res.rc)
endif(WIN32)