# gd-imgui-cocos

imgui in gd using only cocos! no hooks or anything

due to cocos there are a few things missing:
- no right clicking
- keyboard input might not work properly
- mouse hovering only on windows (for now)

## setup

link to it like any other cmake library

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