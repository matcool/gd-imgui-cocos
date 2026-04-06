#include <imgui-cocos.hpp>

#include <Geode/Geode.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>
#include <Geode/modify/CCIMEDispatcher.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

#include <imgui.h>

using namespace geode::prelude;

#ifndef GEODE_IS_IOS
class $modify(ImGuiCocosCCMouseDispatcher, CCMouseDispatcher) {
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
};
#endif

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

#if GEODE_COMP_GD_VERSION >= 22080
	#define IF_2_208(...) __VA_ARGS__
#else
	#define IF_2_208(...)
#endif

class $modify(ImGuiCocosCCIMEDispatcher, CCIMEDispatcher) {
	void dispatchInsertText(const char* text, int len IF_2_2(, enumKeyCodes keys)) {
		if (!ImGuiCocos::get().isInitialized())
			return CCIMEDispatcher::dispatchInsertText(text, len IF_2_2(, keys));

		auto& io = ImGui::GetIO();
		if (!io.WantCaptureKeyboard) {
			CCIMEDispatcher::dispatchInsertText(text, len IF_2_2(, keys));
		}

#if GEODE_COMP_GD_VERSION >= 22000
		switch (keys) {
			case KEY_Left:
			case KEY_Right:
				return; // cocos sends 'a' text alongside the keycode
			default: break;
		}
#endif

		std::string str(text, len);
		io.AddInputCharactersUTF8(str.c_str());
	}

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
};

ImGuiKey cocosToImGuiKey(cocos2d::enumKeyCodes key) {
	if (key >= KEY_A && key <= KEY_Z) {
		return static_cast<ImGuiKey>(ImGuiKey_A + (key - KEY_A));
	}
	if (key >= KEY_Zero && key <= KEY_Nine) {
		return static_cast<ImGuiKey>(ImGuiKey_0 + (key - KEY_Zero));
	}
	switch (key) {
		case KEY_Up: return ImGuiKey_UpArrow;
		case KEY_Down: return ImGuiKey_DownArrow;
		case KEY_Left: return ImGuiKey_LeftArrow;
		case KEY_Right: return ImGuiKey_RightArrow;

		case KEY_Control: return ImGuiKey_LeftCtrl;
		case KEY_LeftControl: return ImGuiKey_LeftCtrl;
		case KEY_RightContol: return ImGuiKey_RightCtrl;
		case KEY_LeftWindowsKey: return ImGuiKey_LeftSuper;
		case KEY_RightWindowsKey: return ImGuiKey_RightSuper;
		case KEY_Shift: return ImGuiKey_LeftShift;
		case KEY_LeftShift: return ImGuiKey_LeftShift;
		case KEY_RightShift: return ImGuiKey_RightShift;
		case KEY_Alt: return ImGuiKey_LeftAlt;
		case KEY_LeftMenu: return ImGuiKey_LeftAlt;
		case KEY_RightMenu: return ImGuiKey_RightAlt;
		case KEY_Enter: return ImGuiKey_Enter;

		case KEY_Home: return ImGuiKey_Home;
		case KEY_End: return ImGuiKey_End;
		case KEY_Delete: return ImGuiKey_Delete;
		case KEY_Backspace: return ImGuiKey_Backspace;
		case KEY_Tab: return ImGuiKey_Tab;
		case KEY_Escape: return ImGuiKey_Escape;

		default: return ImGuiKey_None;
	}
}

bool shouldBlockInput() {
	auto& inst = ImGuiCocos::get();
	return inst.isVisible() && inst.getInputMode() == ImGuiCocos::InputMode::Blocking;
}

// Use Geode 5.0.0 keyboard event system
// would use a geode version check here, but there is no macro for it..
#if GEODE_COMP_GD_VERSION >= 22080

#ifdef GEODE_IS_MACOS
// this is a workaround for ListenerResult::Stop preventing dispatchInsertText from getting called on macOS
static bool s_shouldEatInput = false;

class $modify(ImGuiCocosCCKeyboardDispatcher, CCKeyboardDispatcher) {
	bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double time) {
		if (!s_shouldEatInput)
			return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, time);
		s_shouldEatInput = false;
		return false;
	}
};
#endif

$execute {
	KeyboardInputEvent().listen([](auto& evt) {
		if (!ImGuiCocos::get().isInitialized())
			return ListenerResult::Propagate;

		const bool shouldEatInput = ImGui::GetIO().WantCaptureKeyboard || shouldBlockInput();
		const bool isDown = evt.action != KeyboardInputData::Action::Release;
		if (shouldEatInput || !isDown) {
			const auto imKey = cocosToImGuiKey(evt.key);
			if (imKey != ImGuiKey_None) {
				ImGui::GetIO().AddKeyEvent(imKey, isDown);

				// handle pasting
				if (isDown && evt.key == KEY_V
			#ifdef GEODE_IS_MACOS
					&& (evt.modifiers & KeyboardModifier::Super)
			#else
					&& (evt.modifiers & KeyboardModifier::Control)
			#endif
				) {
					ImGui::GetIO().AddInputCharactersUTF8(clipboard::read().c_str());
				}
			}
		}

	#ifdef GEODE_IS_MACOS
		s_shouldEatInput = shouldEatInput;
		return ListenerResult::Propagate;
	#else
		return shouldEatInput;
	#endif
	}).leak();
}
#else
// otherwise, hook dispatchKeyboardMSG
#ifndef GEODE_IS_IOS
class $modify(ImGuiCocosCCKeyboardDispatcher, CCKeyboardDispatcher) {
	bool dispatchKeyboardMSG(enumKeyCodes key, bool down IF_2_2(, bool repeat) IF_2_208(, double time)) {
		if (!ImGuiCocos::get().isInitialized())
			return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down IF_2_2(, repeat) IF_2_208(, time));

		const bool shouldEatInput = ImGui::GetIO().WantCaptureKeyboard || shouldBlockInput();
		if (shouldEatInput || !down) {
			const auto imKey = cocosToImGuiKey(key);
			if (imKey != ImGuiKey_None) {
				ImGui::GetIO().AddKeyEvent(imKey, down);
			}
		}
		if (shouldEatInput) {
			return false;
		} else {
			return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down IF_2_2(, repeat) IF_2_208(, time));
		}
	}
};
#endif
#endif

class $modify(ImGuiCocosCCTouchDispatcher, CCTouchDispatcher) {
	void touches(CCSet* touches, CCEvent* event, unsigned int type) {
		if (!ImGuiCocos::get().isInitialized() || !touches)
			return CCTouchDispatcher::touches(touches, event, type);

		auto& io = ImGui::GetIO();
		auto* touch = static_cast<CCTouch*>(touches->anyObject());

		if (!touch) return CCTouchDispatcher::touches(touches, event, type);

		// add mouse source events, so imgui can handle touches right
		if (geode::cocos::getMousePos().isZero()) {
			// touch->getLocation() can be different from geode::cocos::getMousePos()!
			const auto pos = ImGuiCocos::cocosToFrame(touch->getLocation());
			io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
			io.AddMousePosEvent(pos.x, pos.y);
		}

		if (io.WantCaptureMouse || shouldBlockInput()) {
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
};

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

#include <Geode/modify/CCEGLView.hpp>

class $modify(ImGuiCocosCCEGLView, CCEGLView) {
#ifdef IMGUI_COCOS_HOOK_EARLY
	static void onModify(auto& self) {
		if (!self.setHookPriorityPre("cocos2d::CCEGLView::swapBuffers", Priority::Early)) {
			log::warn("Failed to set hook priority for swapBuffers");
		}
	}
#endif

	void swapBuffers() {
		if (ImGuiCocos::get().isInitialized())
			ImGuiCocos::get().drawFrame();

		CCEGLView::swapBuffers();
	}

#ifdef GEODE_IS_WINDOWS
	void toggleFullScreen(bool value IF_2_2(, bool borderless) IF_2_207(, bool fix)) {
		if (!ImGuiCocos::get().isInitialized())
			return CCEGLView::toggleFullScreen(value IF_2_2(, borderless) IF_2_207(, fix));

		ImGuiCocos::get().destroy();
		CCEGLView::toggleFullScreen(value IF_2_2(, borderless) IF_2_207(, fix));
		ImGuiCocos::get().setup();
	}
#endif
};

#else

#include <Geode/modify/CCDirector.hpp>

class $modify(ImGuiCocosCCDirector, CCDirector) {
	void drawScene() {
		CCDirector::drawScene();
		if (ImGuiCocos::get().isInitialized())
			ImGuiCocos::get().drawFrame();
	}
};

#endif
