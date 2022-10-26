# gd-imgui-cocos

imgui in gd using only cocos! no hooks or anything

due to cocos there are a few things missing:
- no right clicking
- keyboard input might not work properly
- mouse hovering only on windows (for now)

## setup

set the `IMGUI_COCOS_TARGET` cmake variable before importing to either
- `cocos2d` if you're using cocos-headers
- `geode-sdk` if you're using geode

```cmake
set(IMGUI_COCOS_TARGET geode-sdk)
```

and then link to it ofc

## usage

```cpp
#include <imgui-cocos.hpp>
#include <imgui.h>

// ... somewhere in the code, like a button callback
auto* node = ImGuiNode::create([]() {
    ImGui::Begin("hello");
    ImGui::Button("Hello world!!");
    ImGui::End();
});
// IMPORTANT!! there can only be one ImGuiNode at a time,
// since ImGui stuff uses singletons (ImGui::GetIO() for example)
// so when one already exists, create will return a nullptr
if (node) {
    node->setZOrder(999);
    CCDirector::sharedDirector()->getRunningScene()->addChild(node);
}
```

another thing to keep in mind is since this is just a node in the current scene, it will go away when you switch scenes, meaning you will have to implement the code for moving it to another scene yourself (like the AchievementNotifier in gd)