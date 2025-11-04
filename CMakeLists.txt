cmake_minimum_required(VERSION 3.14)

project(imgui-cocos)

add_library(imgui-cocos INTERFACE)

set_target_properties(imgui-cocos PROPERTIES CXX_VISIBILITY_PRESET hidden)

target_sources(imgui-cocos INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hooks.cpp
)

target_include_directories(imgui-cocos INTERFACE include)

option(HAS_IMGUI "If imgui is already provided externally" OFF)
set(IMGUI_VERSION "v1.92.4" CACHE STRING "What imgui version to download and use")
option(IMGUI_HOOK_EARLY "Hook swapBuffers early, as to show imgui under mod menus. Disable this if you're a mod menu" ON)

if (IMGUI_HOOK_EARLY)
    target_compile_definitions(imgui-cocos INTERFACE IMGUI_COCOS_HOOK_EARLY=1)
endif()

if (NOT HAS_IMGUI)
    if (NOT COMMAND CPMAddPackage)
        include(cmake/CPM.cmake)
    endif()

    CPMAddPackage("gh:ocornut/imgui#${IMGUI_VERSION}")

    target_include_directories(imgui-cocos INTERFACE ${imgui_SOURCE_DIR})

    target_sources(imgui-cocos INTERFACE
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
    )
endif()

if (PROJECT_IS_TOP_LEVEL)
    add_subdirectory(test)
endif()