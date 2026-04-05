#include <Geode/Geode.hpp>
#include <imgui-cocos.hpp>
#include <DroidSans_ttf.h>

#ifdef GEODE_IS_WINDOWS
	#define strcpy strcpy_s
#endif

$on_mod(Loaded) {
	ImGuiCocos::get().setup([] {
		ImGui::StyleColorsDark();
		auto& io = ImGui::GetIO();
		ImFontConfig cfg;
		strcpy(cfg.Name, "DroidSans");
		cfg.FontDataOwnedByAtlas = false;
		io.Fonts->AddFontFromMemoryTTF(DroidSans_ttf_data, DroidSans_ttf_size, 18.f, &cfg);
		io.Fonts->AddFontDefault();
	}).draw([] {
		if (ImGui::Begin("So much text!")) {
			static float size = 18;
			ImGui::DragFloat("Size", &size, 0.25f, 5.f, 100.f);

			ImGui::PushFont(NULL, size);
			ImGui::Text("Wow! this window sure has a lot of text.");
			ImGui::Text("와! 이 창에는 정말 많은 텍스트가 있네요.");
			ImGui::Text("Ого! В этом окне и правда много текста.");
			ImGui::Text("哇！这个窗口里的文字真多。");
			ImGui::Text("すごい！このウィンドウにはたくさんのテキストがありますね。");
			ImGui::Text("वाह! इस विंडो में तो बहुत सारा टेक्स्ट है।");
			ImGui::PopFont();
		}
		ImGui::End();

		ImGui::ShowDemoWindow();
	});
}

#ifndef GEODE_IS_IOS
#include <Geode/modify/CCKeyboardDispatcher.hpp>
class $modify(ImGuiKeybindHook, cocos2d::CCKeyboardDispatcher) {
	bool dispatchKeyboardMSG(cocos2d::enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double ts) {
		if (key == cocos2d::enumKeyCodes::KEY_F4 && isKeyDown) {
			ImGuiCocos::get().toggle();
		}
		return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, ts);
	}
};
#endif