cmake_minimum_required(VERSION 3.15)

option(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES "Enable VST 3 Plug-in Examples" OFF)
option(SMTG_ENABLE_VST3_HOSTING_EXAMPLES "Enable VST 3 Hosting Examples" OFF)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "")

project(SimpleSynthVST VERSION 1.0.0.0 DESCRIPTION "SimpleSynth VST 3 Plug-in")

# 配置VST 3 SDK路径
set(VST3_SDK_PATH "${REFERENCE_DIR}/VST_SDK/vst3sdk")
if(NOT EXISTS ${VST3_SDK_PATH})
    message(FATAL_ERROR "VST3 SDK not found at ${VST3_SDK_PATH}")
endif()

add_subdirectory(${VST3_SDK_PATH} ${PROJECT_BINARY_DIR}/vst3sdk)

smtg_enable_vst3_sdk()

# 源文件
file(GLOB_RECURSE PLUGIN_SRC "*.h" "*.cpp")

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

smtg_add_vst3plugin(${PROJECT_NAME}  ${PLUGIN_SRC} ${YZRUTIL_SRC} ${SIMPLEDSP_SRC} ${SIMPLESYNTH_SRC})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("USE O3")
    target_compile_options(${PROJECT_NAME} PRIVATE -O3)
endif()

# 链接SimpleSynth核心库
target_link_libraries(${PROJECT_NAME} PRIVATE sdk)

set(SMTG_ENABLE_VSTGUI_SUPPORT 1)

if(SMTG_ENABLE_VSTGUI_SUPPORT)
    set(SMTG_VSTGUI_ROOT "${VST3_SDK_PATH}/vstgui4/vstgui")
    target_sources(${PROJECT_NAME}
        PRIVATE
            resource/editor.uidesc
    )
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            vstgui_support
    )
    smtg_target_add_plugin_resources(${PROJECT_NAME}
        RESOURCES
            "resource/editor.uidesc"
            "resource/switch.png"
            "resource/lang.properties"
    )
endif(SMTG_ENABLE_VSTGUI_SUPPORT)

smtg_target_add_plugin_snapshots (${PROJECT_NAME}
    RESOURCES
        resource/4FEB6B95AF135EA997BC114BBF65C357_snapshot.png
        resource/4FEB6B95AF135EA997BC114BBF65C357_snapshot_2.0x.png
)

smtg_target_configure_version_file(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE
"${CMAKE_SOURCE_DIR}/yzrutil/main"
"${CMAKE_SOURCE_DIR}/SimpleDSP/main"
"${CMAKE_SOURCE_DIR}/SimpleSynth/main"
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
"${CMAKE_SOURCE_DIR}/yzrutil/main/yzrutil.h" 
"${CMAKE_SOURCE_DIR}/SimpleSynth/main/SimpleSynth.h")

#if(SMTG_MAC)
#    smtg_target_set_bundle(${PROJECT_NAME}
#        BUNDLE_IDENTIFIER com.yourcompany.simplesynth
#        COMPANY_NAME "YourCompany"
#    )
#    smtg_target_set_debug_executable(${PROJECT_NAME}
#        "/Applications/VST3PluginTestHost.app"
#        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
#    )
#elseif(SMTG_WIN)
#    target_sources(${PROJECT_NAME} PRIVATE 
#        resource/win32resource.rc
#    )
#    if(MSVC)
#        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
#        # 使用绝对路径，确保路径正确
#        set(DEBUG_HOST_PATH "E:/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe")
#        
#        # 检查文件是否存在
#        if(EXISTS ${DEBUG_HOST_PATH})
#            message("Debug host found: ${DEBUG_HOST_PATH}")
#            smtg_target_set_debug_executable(${PROJECT_NAME}
#                "${DEBUG_HOST_PATH}"
#                "--pluginfolder \"\$(OutDir)\""
#            )
#        else()
#            message(WARNING "Debug host not found at: ${DEBUG_HOST_PATH}")
#        endif()
#    endif()
#endif(SMTG_MAC)

if(WIN32)
    # 设置字符集相关选项
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        UNICODE
        _UNICODE
        _CRT_SECURE_NO_WARNINGS
    )
    
    # 设置链接器选项
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/MANIFEST:NO /LTCG"
    )
    
    # 确保资源编译器使用正确的编码
    set_source_files_properties(resource/win32resource.rc PROPERTIES
        COMPILE_FLAGS "/c65001"
    )
endif()