#include <Geode/Geode.hpp>
#include <imgui-cocos.hpp>

$on_mod(Loaded) {
	ImGuiCocos::get().setup([] {
		ImGui::StyleColorsLight();
		GEODE_MOBILE(ImGui::GetIO().FontGlobalScale = 2.f);
	}).draw([] {
		ImGui::ShowDemoWindow();

		if (ImGui::Begin("CCNodeImage Test")) {
			ImGuiCocosExt::DragScroll();

			//dialogIcon_052
			static cocos2d::CCSprite* dialogIcon_052;
			if (!dialogIcon_052) {
				dialogIcon_052 = cocos2d::CCSprite::create("dialogIcon_052.png");
				dialogIcon_052->setAnchorPoint(cocos2d::CCPointZero);
			}
			ImGuiCocosExt::CCNodeImage(dialogIcon_052, 120);

			ImGui::SameLine();

			//GJ_logo_001
			static cocos2d::CCSprite* GJ_logo_001;
			if (!GJ_logo_001) {
				GJ_logo_001 = cocos2d::CCSprite::createWithSpriteFrameName("GJ_logo_001.png");
				GJ_logo_001 ? GJ_logo_001->setAnchorPoint(cocos2d::CCPointZero) : void();
			}
			ImGuiCocosExt::CCNodeImage(GJ_logo_001);

			//label
			static cocos2d::CCLabelBMFont* label;
			if (!label) {
				label = cocos2d::CCLabelBMFont::create("Funny bitmap font label.", "bigFont.fnt");
				label ? label->setAnchorPoint(cocos2d::CCPointZero) : void();
			}
			ImGuiCocosExt::CCNodeImage(label);

			//scene
			ImGuiCocosExt::CCNodeImage(cocos2d::CCScene::get());

			static std::string text = "Hello, world!\nAnd this is a\nmulti-line text.";
			ImGuiCocosExt::BetterInputText("BetterInputText", &text);
		};
		ImGui::End();

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