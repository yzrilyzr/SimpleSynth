project("SimpleSynth")

file(GLOB_RECURSE SRC_DIR_LIST "*.h" "*.hpp" "*.cpp")

add_library(${PROJECT_NAME} SHARED ${SRC_DIR_LIST})

target_include_directories(${PROJECT_NAME} PRIVATE
"${CMAKE_SOURCE_DIR}/yzrutil/main"
"${CMAKE_SOURCE_DIR}/SimpleDSP/main"
${PROJECT_SOURCE_DIR}
)
target_precompile_headers(${PROJECT_NAME} PRIVATE SimpleSynth.h)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

target_link_libraries(${PROJECT_NAME} PRIVATE yzrutil SimpleDSP)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("USE O3")
    target_compile_options(${PROJECT_NAME} PRIVATE -O3)
endif()

