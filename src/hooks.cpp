#include <imgui-cocos.hpp>

#include <Geode/Geode.hpp>
using namespace geode::prelude;

// 2.2 adds some new arguments to the dispatchers
#if GEODE_COMP_GD_VERSION >= 22000
#define IF_2_2(...) __VA_ARGS__
#else
#define IF_2_2(...)
#endif

#if GEODE_COMP_GD_VERSION >= 22070
#define IF_2_207(...) __VA_ARGS__
#else
#define IF_2_207(...)
#endif

/*

	Keyboard events

*/

#if !defined(IMGUI_COCOS_EXCLUDE_IME_DISPATCHER_HOOKS)

#include <Geode/modify/CCIMEDispatcher.hpp>
class $modify(CCIMEDispatcher) {

#if !defined(IMGUI_COCOS_EXCLUDE_INSERT_TEXT_HOOK)
	void dispatchInsertText(const char* text, int len IF_2_2(, enumKeyCodes keys)) {
		if (!ImGuiCocos::get().isInitialized())
			return CCIMEDispatcher::dispatchInsertText(text, len IF_2_2(, keys));

		auto& io = ImGui::GetIO();
		if (!io.WantCaptureKeyboard) {
			CCIMEDispatcher::dispatchInsertText(text, len IF_2_2(, keys));
		}

		std::string str(text, len);
		io.AddInputCharactersUTF8(str.c_str());
	}
#endif

#if !defined(IMGUI_COCOS_EXCLUDE_DELETE_BACKWARD_HOOK)
	void dispatchDeleteBackward() {
		if (!ImGuiCocos::get().isInitialized())
			return CCIMEDispatcher::dispatchDeleteBackward();

		auto& io = ImGui::GetIO();
		if (!io.WantCaptureKeyboard) {
			CCIMEDispatcher::dispatchDeleteBackward();
		}
		// is this really how youre supposed to do this
		io.AddKeyEvent(ImGuiKey_Backspace, true);
		io.AddKeyEvent(ImGuiKey_Backspace, false);
	}
#endif

};

#endif ///// IMGUI_COCOS_EXCLUDE_IME_DISPATCHER_HOOKS

#if !defined(IMGUI_COCOS_EXCLUDE_KEYBOARD_DISPATCHER_HOOKS)

#ifndef GEODE_IS_IOS
#include <Geode/modify/CCKeyboardDispatcher.hpp>
class $modify(CCKeyboardDispatcher) {

#if !defined(IMGUI_EXCLUDE_KEYBOARD_HOOK)
	bool dispatchKeyboardMSG(enumKeyCodes key, bool down IF_2_2(, bool repeat)) {
		if (!ImGuiCocos::get().isInitialized())
			return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down IF_2_2(, repeat));

		const bool shouldEatInput = ImGui::GetIO().WantCaptureKeyboard || ImGuiCocos::Utils::shouldBlockInput();
		if (true) { // why "shouldEatInput || !down" was here? imgui wants key events all the time -LatterRarity70
			const auto imKey = ImGuiCocos::Utils::cocosToImGuiKey(key);
			if (imKey != ImGuiKey_None) {
				ImGui::GetIO().AddKeyEvent(imKey, down);
			}
		}
		if (shouldEatInput) {
			return false;
		} else {
			return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down IF_2_2(, repeat));
		}
	}
#endif // IMGUI_EXCLUDE_KEYBOARD_HOOK

};
#endif // !GEODE_IS_IOS

#endif ///// IMGUI_COCOS_EXCLUDE_KEYBOARD_DISPATCHER_HOOKS

/*

	Mouse and touch events

*/

#if !defined(IMGUI_COCOS_EXCLUDE_MOUSE_DISPATCHER_HOOKS)

#ifndef GEODE_IS_IOS
#include <Geode/modify/CCMouseDispatcher.hpp>
class $modify(CCMouseDispatcher) {

#if !defined(IMGUI_COCOS_EXCLUDE_SCROLL_HOOK)
	bool dispatchScrollMSG(float y, float x) {
		if (!ImGuiCocos::get().isInitialized())
			return CCMouseDispatcher::dispatchScrollMSG(y, x);

		auto& io = ImGui::GetIO();
		static constexpr float scrollMult = 1.f / 10.f;
		io.AddMouseWheelEvent(x * scrollMult, -y * scrollMult);

		if (!io.WantCaptureMouse) {
			return CCMouseDispatcher::dispatchScrollMSG(y, x);
		}
		return true;
	}
#endif // !IMGUI_COCOS_EXCLUDE_SCROLL_HOOK

};
#endif // !GEODE_IS_IOS

#endif ///// IMGUI_COCOS_EXCLUDE_MOUSE_DISPATCHER_HOOKS


#if !defined(IMGUI_COCOS_EXCLUDE_TOUCH_DISPATCHER_HOOKS)

#include <Geode/modify/CCTouchDispatcher.hpp>
class $modify(CCTouchDispatcher) {

#if !defined(IMGUI_EXCLUDE_TOUCHES_HOOK)
	void touches(CCSet* touches, CCEvent* event, unsigned int type) {
		if (!ImGuiCocos::get().isInitialized() || !touches)
			return CCTouchDispatcher::touches(touches, event, type);

		auto& io = ImGui::GetIO();
		auto* touch = static_cast<CCTouch*>(touches->anyObject());

		if (!touch) return CCTouchDispatcher::touches(touches, event, type);

		// add mouse source events, so imgui can handle touches right -LatterRarity70

		if (geode::cocos::getMousePos().isZero()) { // no multiple pos event ways (backend.cpp)
			// i mean touch->getLocation() can be different of geode::cocos::getMousePos() -LatterRarity70
			const auto pos = ImGuiCocos::cocosToFrame(touch->getLocation());
			io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen); // !!!
			io.AddMousePosEvent(pos.x, pos.y);
		}

		if (io.WantCaptureMouse || ImGuiCocos::Utils::shouldBlockInput()) {
			if (type == CCTOUCHBEGAN) {
        		io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
				io.AddMouseButtonEvent(0, true);
			} else if (type == CCTOUCHENDED || type == CCTOUCHCANCELLED) {
        		io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
				io.AddMouseButtonEvent(0, false);
			}
			if (type == CCTOUCHMOVED) {
				CCTouchDispatcher::touches(touches, event, CCTOUCHCANCELLED);
			}
		} else {
			if (type != CCTOUCHMOVED) {
        		io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
				io.AddMouseButtonEvent(0, false);
			}
			CCTouchDispatcher::touches(touches, event, type);
		}
	}
#endif // IMGUI_EXCLUDE_TOUCHES_HOOK

};


#endif ///// IMGUI_COCOS_EXCLUDE_TOUCH_DISPATCHER_HOOKS


/*

	Drawing

*/


// need imgui to be drawn inbetween glClear and swapBuffers:
// drawScene() {
//   glClear();
//   draw current scene();
//   <- here!
//   swapBuffers();
// }
// swapBuffers on android and macos doesnt do anything, so hooking it might not work,
// and because it doesnt do anything just drawing imgui at the end of drawScene works fine

#if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_IOS)

#if !defined(IMGUI_COCOS_EXCLUDE_EGLVIEW_HOOKS)

#include <Geode/modify/CCEGLView.hpp>
class $modify(CCEGLView) {

#if !defined(IMGUI_COCOS_EXCLUDE_SWAP_BUFFERS_HOOK)
	void swapBuffers() {
		if (ImGuiCocos::get().isInitialized())
			ImGuiCocos::get().drawFrame();

		CCEGLView::swapBuffers();
	}
#endif // IMGUI_COCOS_EXCLUDE_SWAP_BUFFERS_HOOK

#ifdef GEODE_IS_WINDOWS 
#if !defined(IMGUI_COCOS_EXCLUDE_TOGGLE_FULLSCREEN_HOOK)
	void toggleFullScreen(bool value IF_2_2(, bool borderless) IF_2_207(, bool fix)) {
		if (!ImGuiCocos::get().isInitialized())
			return CCEGLView::toggleFullScreen(value IF_2_2(, borderless) IF_2_207(, fix));

		ImGuiCocos::get().destroy();
		CCEGLView::toggleFullScreen(value IF_2_2(, borderless) IF_2_207(, fix));
		ImGuiCocos::get().setup();
	}
#endif // IMGUI_COCOS_EXCLUDE_TOGGLE_FULLSCREEN_HOOK
#endif // GEODE_IS_WINDOWS

};

#endif //// IMGUI_COCOS_EXCLUDE_EGLVIEW_HOOKS

#else

#if !defined(IMGUI_COCOS_EXCLUDE_DIRECTOR_HOOKS)

#include <Geode/modify/CCDirector.hpp>
class $modify(CCDirector) {

#if !defined(IMGUI_COCOS_EXCLUDE_DRAW_SCENE_HOOK)
	void drawScene() {
		CCDirector::drawScene();
		if (ImGuiCocos::get().isInitialized())
			ImGuiCocos::get().drawFrame();
	}
#endif

};

#endif //// IMGUI_COCOS_EXCLUDE_DIRECTOR_HOOKS

#endif
