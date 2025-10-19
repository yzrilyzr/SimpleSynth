project ("SimpleSynthJNI")

file(GLOB_RECURSE SRC_DIR_LIST "*.h" "*.hpp" "*.cpp" "*.c")

add_library(${PROJECT_NAME} SHARED ${SRC_DIR_LIST})

target_include_directories(${PROJECT_NAME} PRIVATE
${JNI_INCLUDE_DIRS}
"${CMAKE_SOURCE_DIR}/yzrutil/main"
"${CMAKE_SOURCE_DIR}/SimpleDSP/main"
"${CMAKE_SOURCE_DIR}/SimpleSynth/main"
)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

target_link_libraries(${PROJECT_NAME} PRIVATE yzrutil SimpleDSP SimpleSynth)

