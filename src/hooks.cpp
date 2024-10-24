#include <imgui-cocos.hpp>

#include <Geode/Geode.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>
#include <Geode/modify/CCIMEDispatcher.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

#include <imgui.h>

using namespace geode::prelude;

$on_mod(Unloaded) {
	ImGuiCocos::get().destroy();
}

class $modify(CCMouseDispatcher) {
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

class $modify(CCIMEDispatcher) {
	void dispatchInsertText(const char* text, int len, enumKeyCodes keys) {
		if (!ImGuiCocos::get().isInitialized())
			return CCIMEDispatcher::dispatchInsertText(text, len, keys);

		auto& io = ImGui::GetIO();
		if (!io.WantCaptureKeyboard) {
			CCIMEDispatcher::dispatchInsertText(text, len, keys);
		}
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

		case KEY_Control: return ImGuiKey_ModCtrl;
		case KEY_Shift: return ImGuiKey_ModShift;
		case KEY_Alt: return ImGuiKey_ModAlt;
		case KEY_Enter: return ImGuiKey_Enter;

		case KEY_Home: return ImGuiKey_Home;
		case KEY_End: return ImGuiKey_End;
		case KEY_Delete: return ImGuiKey_Delete;

		default: return ImGuiKey_None;
	}
}

bool shouldBlockInput() {
	auto& inst = ImGuiCocos::get();
	return inst.isVisible() && inst.getInputMode() == ImGuiCocos::InputMode::Blocking;
}

class $modify(CCKeyboardDispatcher) {
	bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool idk) {
		if (!ImGuiCocos::get().isInitialized())
			return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, idk);

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
			return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, idk);
		}
	}
};

class $modify(CCTouchDispatcher) {
	void touches(CCSet* touches, CCEvent* event, unsigned int type) {
		if (!ImGuiCocos::get().isInitialized() || !touches)
			return CCTouchDispatcher::touches(touches, event, type);

		auto& io = ImGui::GetIO();
		auto* touch = static_cast<CCTouch*>(touches->anyObject());

		if (!touch) return CCTouchDispatcher::touches(touches, event, type);

		const auto pos = ImGuiCocos::cocosToFrame(touch->getLocation());
		io.AddMousePosEvent(pos.x, pos.y);

		if (io.WantCaptureMouse || shouldBlockInput()) {
			if (type == CCTOUCHBEGAN) {
				io.AddMouseButtonEvent(0, true);
			} else if (type == CCTOUCHENDED || type == CCTOUCHCANCELLED) {
				io.AddMouseButtonEvent(0, false);
			}
			if (type == CCTOUCHMOVED) {
				CCTouchDispatcher::touches(touches, event, CCTOUCHCANCELLED);
			}
		} else {
			if (type != CCTOUCHMOVED) {
				io.AddMouseButtonEvent(0, false);
			}
			CCTouchDispatcher::touches(touches, event, type);
		}
	}
};

#ifdef GEODE_IS_WINDOWS

#include <Geode/modify/CCEGLView.hpp>

class $modify(CCEGLView) {
	void swapBuffers() {
		if (ImGuiCocos::get().isInitialized())
			ImGuiCocos::get().drawFrame();

		CCEGLView::swapBuffers();
	}

	void toggleFullScreen(bool value, bool borderless) {
		if (!ImGuiCocos::get().isInitialized())
			return CCEGLView::toggleFullScreen(value, borderless);

		ImGuiCocos::get().destroy();
		CCEGLView::toggleFullScreen(value, borderless);
		ImGuiCocos::get().setup();
	}
};

#else

#include <Geode/modify/CCDirector.hpp>

class $modify(CCDirector) {
	void drawScene() {
		CCDirector::drawScene();
		if (ImGuiCocos::get().isInitialized())
			ImGuiCocos::get().drawFrame();
	}
};

#endif
