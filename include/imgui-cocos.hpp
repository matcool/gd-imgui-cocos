#pragma once

#include <cocos2d.h>
#include <functional>
#include <string>
#include <imgui.h>

class ImGuiCocos {
private:
    cocos2d::CCTexture2D* m_fontTexture = nullptr;
    bool m_initialized = false;
    bool m_visible = true;
    std::function<void()> m_setupCall, m_drawCall;

    ImGuiCocos();

    void newFrame();
    void renderFrame();
public:
    ImGuiCocos(const ImGuiCocos&) = delete;
    ImGuiCocos(ImGuiCocos&&) = delete;

    static ImGuiCocos& get();

    void setup();
    // called on mod unloaded
    void destroy();
    // called on swapBuffers
    void drawFrame();

    void onSetup(std::function<void()>);
    void onDraw(std::function<void()>);

    ImGuiCocos& setup(std::function<void()> fun) {
        this->onSetup(fun);
        this->setup();
        return *this;
    }

    ImGuiCocos& draw(std::function<void()> fun) {
        this->onDraw(fun);
        return *this;
    }

    void toggle() { m_visible = !m_visible; }
    bool isVisible() { return m_visible; }
    void setVisible(bool v) { m_visible = v; }
	
	static ImVec2 cocosToFrame(const cocos2d::CCPoint& pos);
	static cocos2d::CCPoint frameToCocos(const ImVec2& pos);
};
