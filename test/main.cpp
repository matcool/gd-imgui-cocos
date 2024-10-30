#include <Geode/Geode.hpp>
#include <imgui-cocos.hpp>

$on_mod(Loaded) {
	ImGuiCocos::get().setup([] {
		ImGui::StyleColorsLight();
	}).draw([] {
		ImGui::ShowDemoWindow();
	});
}