#pragma once

#include <cocos2d.h>
#include <functional>
#include <string>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

// little helper function to convert ImTexture2D <=> GLuint,
// supporting both versions of imgui where this was a void* and is now a u64
// (templated because c++ is stupid)
namespace ImGui {
	template <class T = ImTextureID>
	inline static GLuint toGLTexture(std::type_identity_t<T> tex) {
		if constexpr (std::is_same_v<T, void*>) {
			return static_cast<GLuint>(reinterpret_cast<std::uintptr_t>(tex));
		}
		else {
			return static_cast<GLuint>(tex);
		}
	}
	template <class T = ImTextureID>
	inline static T fromGLTexture(GLuint tex) {
		if constexpr (std::is_same_v<T, void*>) {
			return reinterpret_cast<T>(tex);
		}
		else {
			return static_cast<T>(tex);
		}
	}
}

class ImGuiCocos {
public:
	enum class InputMode {
		// This is the default behavior, which will let clicks go through to gd,
		// and keyboard inputs too as long as imgui isn't using it.
		Default,
		// This mode blocks any input from going through as long as imgui is toggled on.
		Blocking,
	};
private:
	cocos2d::CCTexture2D* m_fontTexture = nullptr;
	bool m_initialized = false;
	bool m_visible = true;
	bool m_reloading = false;
	bool m_forceLegacy = false;
	std::vector<std::function<void()>> m_setupCalls, m_drawCalls, m_destroyCalls;
	InputMode m_inputMode = InputMode::Default;
	ImGuiMouseCursor m_lastCursor = ImGuiMouseCursor_COUNT;

	ImGuiCocos();

	void newFrame();
	void renderFrame() const;
	void legacyRenderFrame() const; // uses OpenGL 2.0 for rendering, for compatibility with older devices
public:
	ImGuiCocos(const ImGuiCocos&) = delete;
	ImGuiCocos(ImGuiCocos&&) = delete;

	static ImGuiCocos& get();

	// called on mod unloaded
	void destroy();
	// called on swapBuffers
	void drawFrame();

	ImGuiCocos& setup(std::function<void()> fun);
	ImGuiCocos& setup();

	ImGuiCocos& draw(std::function<void()> fun);

	ImGuiCocos& onDestroy(std::function<void()> fun);

	// used to reinitialize imgui context
	void reload();

	void toggle();
	[[nodiscard]] bool isVisible() const;
	void setVisible(bool v);

	void setInputMode(InputMode mode);
	InputMode getInputMode();

	void setForceLegacy(bool force);
	[[nodiscard]] bool getForceLegacy() const;

	[[nodiscard]] bool isInitialized() const;
	
	static ImVec2 cocosToFrame(const cocos2d::CCPoint& pos);
	static cocos2d::CCPoint frameToCocos(const ImVec2& pos);

	// Contains some helper functions. ImGuiCocos::Utils have ImGuiCocosExt typedef
	class Utils {
	public:

		static ImGuiKey cocosToImGuiKey(cocos2d::enumKeyCodes key) {
			if (key >= cocos2d::KEY_A && key <= cocos2d::KEY_Z) {
				return static_cast<ImGuiKey>(ImGuiKey_A + (key - cocos2d::KEY_A));
			}
			if (key >= cocos2d::KEY_Zero && key <= cocos2d::KEY_Nine) {
				return static_cast<ImGuiKey>(ImGuiKey_0 + (key - cocos2d::KEY_Zero));
			}
			switch (key) {
			case cocos2d::KEY_Up: return ImGuiKey_UpArrow;
			case cocos2d::KEY_Down: return ImGuiKey_DownArrow;
			case cocos2d::KEY_Left: return ImGuiKey_LeftArrow;
			case cocos2d::KEY_Right: return ImGuiKey_RightArrow;

			case cocos2d::KEY_Control: return ImGuiKey_ModCtrl;
			case cocos2d::KEY_Shift: return ImGuiKey_ModShift;
			case cocos2d::KEY_Alt: return ImGuiKey_ModAlt;
			case cocos2d::KEY_Enter: return ImGuiKey_Enter;

			case cocos2d::KEY_Home: return ImGuiKey_Home;
			case cocos2d::KEY_End: return ImGuiKey_End;
			case cocos2d::KEY_Delete: return ImGuiKey_Delete;

			default: return ImGuiKey_None;
			}
		}
		static bool shouldBlockInput() {
			auto& inst = ImGuiCocos::get();
			return inst.isVisible() && inst.getInputMode() == ImGuiCocos::InputMode::Blocking;
		}

		// !!!CALL INSIDE WINDOW!!! Scrolls window when user dragging on empty space of a window (useful for android users)
		static auto DragScroll(ImGuiWindow* window = nullptr) {
			ImGuiContext& g = *ImGui::GetCurrentContext();
			window = window ? window : g.CurrentWindow;
			if (!window) return; // No window

			// Calculate scroll delta based on mouse movement
			auto mouse_dt = ImGui::GetIO().MouseDelta;
			ImVec2 delta = ImGui::GetIO().MouseDownDuration[0] > 0.1f
				? ImVec2(-mouse_dt.x, -mouse_dt.y) : ImVec2(0, 0);

			// Create invisible button for capturing drag events on empty space
			ImGuiID id = window->GetID("##scrolldraggingoverlay");
			ImGui::KeepAliveID(id);

			bool hovered = false;
			bool held = false;
			ImGuiButtonFlags button_flags = ImGuiButtonFlags_MouseButtonLeft;

			// Only capture if nothing else is hovered and we're within window bounds
			if (g.HoveredId == 0) { // If nothing hovered so far in the frame (not same as IsAnyItemHovered()!)
				ImGui::ButtonBehavior(window->Rect(), id, &hovered, &held, button_flags);
			}

			const float scroll_threshold = 0.1f;
			if (held) if (window->ScrollbarX && std::abs(delta.x) >= scroll_threshold)
				ImGui::SetScrollX(window, window->Scroll.x + delta.x);
			if (held) if (window->ScrollbarY && std::abs(delta.y) >= scroll_threshold)
				ImGui::SetScrollY(window, window->Scroll.y + delta.y);
		}

		static auto BetterInputText(const char* label, std::string* str, float width = ImGui::CalcItemWidth(), ImGuiInputTextFlags flags = 0) {
			auto g = ImGui::GetCurrentContext();
			auto io = ImGui::GetIO();
			auto style = g->Style;
			int lines_count = std::count(str->begin(), str->end(), '\n') + 1;
			const auto size = ImGui::CalcItemSize({ 0, 0 }, width, (g->FontSize * lines_count) + style.FramePadding.y * 2.0f);
			auto textunput_rtn = ImGui::InputTextMultiline(label, str, size, flags);
			return textunput_rtn;
		}

		static void CCNodeImage(cocos2d::CCNode* node, float maxWidth = ImGui::GetContentRegionAvail().x) {
			if (!node) return;

			auto sf = cocos2d::CCDirector::get()->m_fContentScaleFactor;
			cocos2d::CCDirector::get()->m_fContentScaleFactor = (2.f);

			struct Entry {
				geode::Ref<cocos2d::CCNode> node;
				geode::Ref<cocos2d::CCRenderTexture> rt;
				cocos2d::CCSize lastSize;
			};
			static std::map<cocos2d::CCNode*, Entry> created;

			auto& E = created[node];

			auto currentSize = E.node ? E.node->getContentSize() : node->getContentSize();

			int w = static_cast<int>(currentSize.width);
			int h = static_cast<int>(currentSize.height);

			if (!E.node || !E.rt) {
				E.node = node;
				E.rt = cocos2d::CCRenderTexture::create(w, h);
				if (!E.rt) return;
			}

			E.rt->m_nWidth = w;
			E.rt->m_nHeight = h;

			//fk
			GLint oldViewport[4];
			glGetIntegerv(GL_VIEWPORT, oldViewport);
			GLboolean oldBlend;
			glGetBooleanv(GL_BLEND, &oldBlend);
			GLint oldBlendSrc, oldBlendDst;
			glGetIntegerv(GL_BLEND_SRC_ALPHA, &oldBlendSrc);
			glGetIntegerv(GL_BLEND_DST_ALPHA, &oldBlendDst);
			GLboolean oldDepthTest;
			glGetBooleanv(GL_DEPTH_TEST, &oldDepthTest);
			GLboolean oldCullFace;
			glGetBooleanv(GL_CULL_FACE, &oldCullFace);
			GLfloat oldClearColor[4];
			glGetFloatv(GL_COLOR_CLEAR_VALUE, oldClearColor);

			E.rt->beginWithClear(0, 0, 0, 0);

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			E.node->visit();

			E.rt->end();

			glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
			if (oldBlend) glEnable(GL_BLEND);
			else glDisable(GL_BLEND);
			glBlendFunc(oldBlendSrc, oldBlendDst);
			if (oldDepthTest) glEnable(GL_DEPTH_TEST);
			else glDisable(GL_DEPTH_TEST);
			if (oldCullFace) glEnable(GL_CULL_FACE);
			else glDisable(GL_CULL_FACE);
			glClearColor(oldClearColor[0], oldClearColor[1], oldClearColor[2], oldClearColor[3]);

			auto sprite = E.rt->getSprite();
			if (!sprite or !sprite->getTexture()) return;

			GLuint texID = sprite->getTexture()->getName();
			auto tw = sprite->getTexture()->getPixelsWide();
			auto th = sprite->getTexture()->getPixelsHigh();

			if (tw <= 0 or th <= 0) return;

			float scale = maxWidth / tw;
			ImVec2 itemSize = ImVec2(tw * scale, th * scale);
			ImGui::SetNextItemWidth(itemSize.x);
			ImGui::Image(
				ImGui::fromGLTexture(texID),
				itemSize,
				ImVec2(0, 1),
				ImVec2(1, 0)
			);

			cocos2d::CCDirector::get()->m_fContentScaleFactor = (sf);
		}

		static auto& get() { return ImGuiCocos::get(); }
	};
};

typedef ImGuiCocos::Utils ImGuiCocosExt;
