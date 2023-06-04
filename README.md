# gd-imgui-cocos

imgui in gd using only cocos! ~~no hooks or anything~~ for geode

## setup

link to it like any other cmake library, you can also use cpm which comes with geode

```cmake
CPMAddPackage("gh:matcool/gd-imgui-cocos#commithash") # specify a commit!

target_link_libraries(${PROJECT_NAME} imgui-cocos)
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