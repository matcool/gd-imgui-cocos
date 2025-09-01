#include <Geode/Geode.hpp>
#include <imgui-cocos.hpp>

$on_mod(Loaded) {
	ImGuiCocos::get().setup([] {
		ImGui::StyleColorsLight();
	}).draw([] {
		ImGui::ShowDemoWindow();
		ImGui::ShowMetricsWindow();
	});
}

#ifndef GEODE_IS_IOS
#include <Geode/modify/CCKeyboardDispatcher.hpp>
class $modify(ImGuiKeybindHook, cocos2d::CCKeyboardDispatcher) {
	bool dispatchKeyboardMSG(cocos2d::enumKeyCodes key, bool isKeyDown, bool isKeyRepeat) {
		if (key == cocos2d::enumKeyCodes::KEY_F4 && isKeyDown) {
			ImGuiCocos::get().toggle();
		}
		return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat);
	}
};
#endif