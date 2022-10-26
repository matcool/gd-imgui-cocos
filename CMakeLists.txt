cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(imgui-cocos)

add_library(imgui-cocos src/main.cpp)

include(cmake/CPM.cmake)

CPMAddPackage("gh:ocornut/imgui#v1.88")

target_compile_definitions(imgui-cocos PRIVATE CC_DLL=)

target_include_directories(imgui-cocos PUBLIC ${imgui_SOURCE_DIR} include)

target_sources(imgui-cocos PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
)

target_link_libraries(imgui-cocos "${IMGUI_COCOS_TARGET}")

if(WIN32)
    target_link_libraries(imgui-cocos opengl32 ${CMAKE_CURRENT_SOURCE_DIR}/glew32.lib)
elseif(APPLE)
    target_link_libraries(imgui-cocos "-framework OpenGL")
elseif(ANDROID)
    target_link_libraries(imgui-cocos GLESv2)
endif()
