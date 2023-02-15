#pragma once

#include <cocos2d.h>
#include <imgui.h>
#include <functional>

class ImGuiNode : public cocos2d::CCLayer, cocos2d::CCIMEDelegate {
	bool m_has_created_fonts_texture = false;
	std::function<void()> m_draw_callback;
	bool m_ime_attached = false;
	cocos2d::CCTexture2D* m_font_texture = nullptr;
public:
	static ImGuiNode* create(const std::function<void()>& draw_callback);

protected:
	bool init() override;

	void draw() override;

	void new_frame();

	bool canAttachWithIME() override;
	bool canDetachWithIME() override;

	void insertText(const char* text, int len) override;

	void deleteBackward() override;

	bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent*) override;

	void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent*) override;

	void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent*) override;

	void scrollWheel(float dy, float dx) override;

	void keyDown(cocos2d::enumKeyCodes key) override;

	void keyUp(cocos2d::enumKeyCodes key) override;

	// these dont ever get called :(
	void rightKeyDown() override;

	void rightKeyUp() override;

	void render_draw_data(ImDrawData* draw_data);

	~ImGuiNode();
};