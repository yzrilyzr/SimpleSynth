project(LuaSynth)

set(LUA_DIR "${REFERENCE_DIR}/lua-5.4.8")

file(GLOB_RECURSE SRC_DIR_LIST "*.h" "*.hpp" "*.cpp")
set(LUA_SOURCES
    "${LUA_DIR}/src/lapi.c"
    "${LUA_DIR}/src/lauxlib.c"
    "${LUA_DIR}/src/lbaselib.c"
    "${LUA_DIR}/src/lcode.c"
    "${LUA_DIR}/src/lcorolib.c"
    "${LUA_DIR}/src/lctype.c"
    "${LUA_DIR}/src/ldblib.c"
    "${LUA_DIR}/src/ldebug.c"
    "${LUA_DIR}/src/ldo.c"
    "${LUA_DIR}/src/ldump.c"
    "${LUA_DIR}/src/lfunc.c"
    "${LUA_DIR}/src/lgc.c"
    "${LUA_DIR}/src/linit.c"
    "${LUA_DIR}/src/liolib.c"
    "${LUA_DIR}/src/llex.c"
    "${LUA_DIR}/src/lmathlib.c"
    "${LUA_DIR}/src/lmem.c"
    "${LUA_DIR}/src/loadlib.c"
    "${LUA_DIR}/src/lobject.c"
    "${LUA_DIR}/src/lopcodes.c"
    "${LUA_DIR}/src/loslib.c"
    "${LUA_DIR}/src/lparser.c"
    "${LUA_DIR}/src/lstate.c"
    "${LUA_DIR}/src/lstring.c"
    "${LUA_DIR}/src/lstrlib.c"
    "${LUA_DIR}/src/ltable.c"
    "${LUA_DIR}/src/ltablib.c"
    "${LUA_DIR}/src/ltm.c"
    "${LUA_DIR}/src/lundump.c"
    "${LUA_DIR}/src/lutf8lib.c"
    "${LUA_DIR}/src/lvm.c"
    "${LUA_DIR}/src/lzio.c"
)

add_library(${PROJECT_NAME} SHARED ${SRC_DIR_LIST} ${LUA_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
"${CMAKE_SOURCE_DIR}/yzrutil/main"
"${CMAKE_SOURCE_DIR}/SimpleDSP/main"
"${CMAKE_SOURCE_DIR}/SimpleSynth/main"
"${LUA_DIR}/src"
${PROJECT_SOURCE_DIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

target_link_libraries(${PROJECT_NAME} PRIVATE yzrutil SimpleDSP SimpleSynth)