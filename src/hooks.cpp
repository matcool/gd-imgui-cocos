#include <imgui-cocos.hpp>

#include <Geode/Geode.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>
#include <Geode/modify/CCIMEDispatcher.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

#include <imgui.h>

using namespace geode::prelude;

$on_mod(Unloaded) {
	ImGuiCocos::get().destroy();
}

class $modify(CCMouseDispatcher) {
    bool dispatchScrollMSG(float y, float x) {
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
    void dispatchInsertText(const char* text, int len) {
        auto& io = ImGui::GetIO();
        if (!io.WantCaptureKeyboard) {
            CCIMEDispatcher::dispatchInsertText(text, len);
        }
        std::string str(text, len);
        io.AddInputCharactersUTF8(str.c_str());
    }

    void dispatchDeleteBackward() {
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
	switch (key) {
		case KEY_Up: return ImGuiKey_UpArrow;
		case KEY_Down: return ImGuiKey_DownArrow;
		case KEY_Left: return ImGuiKey_LeftArrow;
		case KEY_Right: return ImGuiKey_RightArrow;

		case KEY_Control: return ImGuiKey_ModCtrl;
		case KEY_Shift: return ImGuiKey_ModShift;
		case KEY_Alt: return ImGuiKey_ModAlt;
		case KEY_Enter: return ImGuiKey_Enter;

		default: return ImGuiKey_None;
	}
}

class $modify(CCKeyboardDispatcher) {
	bool dispatchKeyboardMSG(enumKeyCodes key, bool down) {
		if (!ImGui::GetIO().WantCaptureKeyboard) {
			return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down);
		}
		const auto imKey = cocosToImGuiKey(key);
		if (imKey != ImGuiKey_None) {
			ImGui::GetIO().AddKeyEvent(imKey, down);
		}
		return false;
	}
};

class $modify(CCTouchDispatcher) {
    void touches(CCSet* touches, CCEvent* event, unsigned int type) {
        auto& io = ImGui::GetIO();
        auto* touch = static_cast<CCTouch*>(touches->anyObject());
		
		if (!touch) return CCTouchDispatcher::touches(touches, event, type);

        const auto pos = ImGuiCocos::cocosToFrame(touch->getLocation());
        io.AddMousePosEvent(pos.x, pos.y);

        if (io.WantCaptureMouse) {
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

class $modify(CCEGLView) {
    void swapBuffers() {
		ImGuiCocos::get().drawFrame();
        CCEGLView::swapBuffers();
    }

    void toggleFullScreen(bool value) {
        ImGuiCocos::get().destroy();
        CCEGLView::toggleFullScreen(value);
        ImGuiCocos::get().setup();
    }
};
