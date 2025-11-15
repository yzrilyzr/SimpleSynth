project(SimpleSynthFB2kCPNT VERSION 1.0.0.0 DESCRIPTION "SimpleSynth foobar2000 Component")

# FB2K SDK 路径
set(FB2K_SDK_PATH "${REFERENCE_DIR}/fb2k_SDK-2025-03-07")
set(WTL_PATH "${REFERENCE_DIR}/WTL10_10320_Release")
set(ATL_PATH "E:/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/atlmfc")

if(NOT EXISTS ${FB2K_SDK_PATH})
    message(FATAL_ERROR "FB2K SDK not found at ${FB2K_SDK_PATH}")
endif()


# 平台特定设置
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_WINDOWS -D_USRDLL)
    
    # 设置编译器选项
    if(MSVC)
        # 重要：统一使用动态运行时库（与 foobar2000 保持一致）
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        
        # 强制所有配置都使用 /MD（因为SDK是Release版本）
        add_compile_options(/MD)
        
        #add_link_options(/DEBUG)
        #add_compile_options(/Zi)
        add_compile_options(/MP)
        add_compile_definitions(_CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS)
    endif()
endif()


# 源文件
file(GLOB_RECURSE COMPONENT_SRC "*.h" "*.cpp")

file(GLOB_RECURSE YZRUTIL_SRC
    "${CMAKE_SOURCE_DIR}/yzrutil/main/*.h"
    "${CMAKE_SOURCE_DIR}/yzrutil/main/*.hpp"
    "${CMAKE_SOURCE_DIR}/yzrutil/main/*.cpp"
)

file(GLOB_RECURSE SIMPLEDSP_SRC
    "${CMAKE_SOURCE_DIR}/SimpleDSP/main/*.h"
    "${CMAKE_SOURCE_DIR}/SimpleDSP/main/*.hpp"
    "${CMAKE_SOURCE_DIR}/SimpleDSP/main/*.cpp"
)

file(GLOB_RECURSE SIMPLESYNTH_SRC
    "${CMAKE_SOURCE_DIR}/SimpleSynth/main/*.h"
    "${CMAKE_SOURCE_DIR}/SimpleSynth/main/*.hpp"
    "${CMAKE_SOURCE_DIR}/SimpleSynth/main/*.cpp"
)

add_library(${PROJECT_NAME} SHARED 
    ${YZRUTIL_SRC}
    ${SIMPLEDSP_SRC}
    ${SIMPLESYNTH_SRC}
    ${COMPONENT_SRC}
)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "foo_simplesynth"
)


target_include_directories(${PROJECT_NAME} PRIVATE
"${ATL_PATH}/include"
"${WTL_PATH}/Include"
"${FB2K_SDK_PATH}/foobar2000"
"${FB2K_SDK_PATH}"
"${CMAKE_SOURCE_DIR}/yzrutil/main"
"${CMAKE_SOURCE_DIR}/SimpleDSP/main"
"${CMAKE_SOURCE_DIR}/SimpleSynth/main"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    _UNICODE
    UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX  
    STRICT
)

target_link_libraries(${PROJECT_NAME} PRIVATE
"${FB2K_SDK_PATH}/foobar2000/foo_sample/x64/Release/pfc.lib"
"${FB2K_SDK_PATH}/foobar2000/foo_sample/x64/Release/foobar2000_SDK.lib"
"${FB2K_SDK_PATH}/foobar2000/foo_sample/x64/Release/foobar2000_sdk_helpers.lib"
"${FB2K_SDK_PATH}/foobar2000/foo_sample/x64/Release/foobar2000_component_client.lib"
"${FB2K_SDK_PATH}/foobar2000/foo_sample/x64/Release/libPPUI.lib"
"${FB2K_SDK_PATH}/foobar2000/shared/shared-x64.lib"
)


target_link_libraries(${PROJECT_NAME} PRIVATE
    atls.lib
    winmm.lib
    ole32.lib
    user32.lib
    kernel32.lib
    advapi32.lib
)

# 编译器特定设置
if(MSVC)
    # 禁用特定警告
    target_compile_options(${PROJECT_NAME} PRIVATE 
        /wd4251 # 'identifier': class 'type' needs to have dll-interface to be used by clients of class 'type2'
        /wd4275 # non - DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
        /wd4996 # The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name
        /wd4200 # nonstandard extension used: zero-sized array in struct/union
        /wd4127 # conditional expression is constant
        /wd4267 # 'var' : conversion from 'size_t' to 'type', possible loss of data
    )
    
    # 设置链接器选项 - 移除静态库排除
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS ""
    )
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE
"${CMAKE_SOURCE_DIR}/yzrutil/main/yzrutil.h" 
"${CMAKE_SOURCE_DIR}/SimpleSynth/main/SimpleSynth.h"
)
