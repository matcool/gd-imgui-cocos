cmake_minimum_required(VERSION 3.14)

project(imgui-cocos)

add_library(imgui-cocos INTERFACE)

include(cmake/CPM.cmake)

CPMAddPackage("gh:ocornut/imgui#v1.88")

target_include_directories(imgui-cocos INTERFACE ${imgui_SOURCE_DIR} include)

target_sources(imgui-cocos INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
)

if(WIN32)
    target_link_libraries(imgui-cocos INTERFACE opengl32 ${CMAKE_CURRENT_SOURCE_DIR}/glew32.lib)
elseif(APPLE)
    target_link_libraries(imgui-cocos INTERFACE "-framework OpenGL")
elseif(ANDROID)
    target_link_libraries(imgui-cocos INTERFACE GLESv2)
endif()
