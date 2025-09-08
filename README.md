# gd-imgui-cocos

imgui in gd using only cocos! ~~no hooks or anything~~ for geode

## setup

link to it like any other cmake library, you can also use cpm which comes with geode

```cmake
CPMAddPackage("gh:matcool/gd-imgui-cocos#commithash") # specify a commit! (or replace "#commithash" to "#geode")
target_link_libraries(${PROJECT_NAME} imgui-cocos)
```

You may specify what version of imgui you want to use by setting `IMGUI_VERSION` before including the library:

```cmake
set(IMGUI_VERSION "v1.70")
CPMAddPackage("gh:matcool/gd-imgui-cocos#...")
```

Or even get imgui yourself, and setting the `HAS_IMGUI` option:
```cmake
add_subdirectory(my-epic-imgui)

set(HAS_IMGUI ON)
CPMAddPackage("gh:matcool/gd-imgui-cocos#...")
```

Optionally you can exclude some [hooks](/blob/geode/src/hooks.cpp) from imgui-cocos (to make own impl for example):
- `IMGUI_COCOS_EXCLUDE_IME_DISPATCHER_HOOKS`
    - `IMGUI_COCOS_EXCLUDE_INSERT_TEXT_HOOK`
    - `IMGUI_COCOS_EXCLUDE_DELETE_BACKWARD_HOOK`
- `IMGUI_COCOS_EXCLUDE_KEYBOARD_DISPATCHER_HOOKS`
    - `IMGUI_EXCLUDE_KEYBOARD_HOOK`
- `IMGUI_COCOS_EXCLUDE_MOUSE_DISPATCHER_HOOKS`
    - `IMGUI_COCOS_EXCLUDE_SCROLL_HOOK`
- `IMGUI_COCOS_EXCLUDE_TOUCH_DISPATCHER_HOOKS`
    - `IMGUI_EXCLUDE_TOUCHES_HOOK`
- `IMGUI_COCOS_EXCLUDE_EGLVIEW_HOOKS`
    - `IMGUI_COCOS_EXCLUDE_TOGGLE_FULLSCREEN_HOOK`
- `IMGUI_COCOS_EXCLUDE_DIRECTOR_HOOKS`
    - `IMGUI_COCOS_EXCLUDE_DRAW_SCENE_HOOK`
```cmake
target_link_libraries(${PROJECT_NAME} imgui-cocos)
target_compile_definitions(imgui-cocos INTERFACE IMGUI_COCOS_EXCLUDE_SCROLL_HOOK)
target_compile_definitions(imgui-cocos INTERFACE IMGUI_COCOS_EXCLUDE_DELETE_BACKWARD_HOOK)
```

## usage

Make sure you call the `setup` method! Even if you don't need the callback you can use the overload with no args.

```cpp
#include <imgui-cocos.hpp>

$on_mod(Loaded) {
    ImGuiCocos::get().setup([] {
        // this runs after imgui has been setup,
        // its a callback as imgui will be re initialized when toggling fullscreen,
        // so use this to setup any themes and or fonts!
    }).draw([] {
        ImGui::Begin("My awesome window");

        ImGui::Button("Awesome button");
        
        ImGui::End();
    });
}
```

This code will create a floating window that will persist throughout scenes, and should always be on top of everything

## random

Heres how you can add your own fonts from mod resources, because i always forget. \
Make sure that in your `mod.json` the ttf is a `file`, instead of a font! you don't want it as a gd font, you want the actual ttf file

```cpp
ImGuiCocos::get().setup([] {
    // you should do this in setup! ok thx
    auto* font = ImGui::GetIO().Fonts->AddFontFromFileTTF(
        CCFileUtils::get()->fullPathForFilename("whatever.ttf"_spr, 0).c_str(), 
        16.0f
    );
    // do something with the font.. like io.FontDefault or something
})//.draw(... etc
```