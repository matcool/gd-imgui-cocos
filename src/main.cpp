#include <array>
#include <imgui-cocos.hpp>

using namespace cocos2d;

// based off ccDrawSolidPoly
// but modified to use textures
void draw_triangle(const std::array<CCPoint, 3>& poli, const std::array<ccColor4F, 3>& colors, const std::array<CCPoint, 3>& uvs) {
	auto* shader = CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor);
	shader->use();
	shader->setUniformsForBuiltins();

	ccGLEnableVertexAttribs(kCCVertexAttribFlag_PosColorTex);

	static_assert(sizeof(CCPoint) == sizeof(ccVertex2F), "so the cocos devs were right then");
	
	glVertexAttribPointer(kCCVertexAttrib_Position, 2, GL_FLOAT, GL_FALSE, 0, poli.data());
	glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_FLOAT, GL_FALSE, 0, colors.data());
	glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, 0, uvs.data());

	glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
}

ImGuiKey cocos_to_imkey(cocos2d::enumKeyCodes key) {
	switch (key) {
		case enumKeyCodes::KEY_A: return ImGuiKey_A;
		case enumKeyCodes::KEY_B: return ImGuiKey_B;
		case enumKeyCodes::KEY_C: return ImGuiKey_C;
		case enumKeyCodes::KEY_D: return ImGuiKey_D;
		case enumKeyCodes::KEY_E: return ImGuiKey_E;
		case enumKeyCodes::KEY_F: return ImGuiKey_F;
		case enumKeyCodes::KEY_G: return ImGuiKey_G;
		case enumKeyCodes::KEY_H: return ImGuiKey_H;
		case enumKeyCodes::KEY_I: return ImGuiKey_I;
		case enumKeyCodes::KEY_J: return ImGuiKey_J;
		case enumKeyCodes::KEY_K: return ImGuiKey_K;
		case enumKeyCodes::KEY_L: return ImGuiKey_L;
		case enumKeyCodes::KEY_M: return ImGuiKey_M;
		case enumKeyCodes::KEY_N: return ImGuiKey_N;
		case enumKeyCodes::KEY_O: return ImGuiKey_O;
		case enumKeyCodes::KEY_P: return ImGuiKey_P;
		case enumKeyCodes::KEY_Q: return ImGuiKey_Q;
		case enumKeyCodes::KEY_R: return ImGuiKey_R;
		case enumKeyCodes::KEY_S: return ImGuiKey_S;
		case enumKeyCodes::KEY_T: return ImGuiKey_T;
		case enumKeyCodes::KEY_U: return ImGuiKey_U;
		case enumKeyCodes::KEY_V: return ImGuiKey_V;
		case enumKeyCodes::KEY_W: return ImGuiKey_W;
		case enumKeyCodes::KEY_X: return ImGuiKey_X;
		case enumKeyCodes::KEY_Y: return ImGuiKey_Y;
		case enumKeyCodes::KEY_Z: return ImGuiKey_Z;

		case enumKeyCodes::KEY_Zero: return ImGuiKey_0;
		case enumKeyCodes::KEY_One: return ImGuiKey_1;
		case enumKeyCodes::KEY_Two: return ImGuiKey_2;
		case enumKeyCodes::KEY_Three: return ImGuiKey_3;
		case enumKeyCodes::KEY_Four: return ImGuiKey_4;
		case enumKeyCodes::KEY_Five: return ImGuiKey_5;
		case enumKeyCodes::KEY_Six: return ImGuiKey_6;
		case enumKeyCodes::KEY_Seven: return ImGuiKey_7;
		case enumKeyCodes::KEY_Eight: return ImGuiKey_8;
		case enumKeyCodes::KEY_Nine: return ImGuiKey_9;

		case enumKeyCodes::KEY_ArrowUp: return ImGuiKey_UpArrow;
		case enumKeyCodes::KEY_ArrowDown: return ImGuiKey_DownArrow;
		case enumKeyCodes::KEY_ArrowLeft: return ImGuiKey_LeftArrow;
		case enumKeyCodes::KEY_ArrowRight: return ImGuiKey_RightArrow;

		default: return ImGuiKey_None;
	}
}

ImVec2 cocos_to_frame(const CCPoint pos) {
	auto* director = CCDirector::sharedDirector();
	const auto frame_size = director->getOpenGLView()->getFrameSize();
	const auto win_size = director->getWinSize();

	return ImVec2(
		pos.x / win_size.width * frame_size.width,
		(1.f - pos.y / win_size.height) * frame_size.height
	);
}

bool imgui_node_exists = false;

ImGuiNode* ImGuiNode::create(const std::function<void()>& draw_callback) {
	if (imgui_node_exists) return nullptr;

	auto* obj = new ImGuiNode;
	if (obj && obj->init()) {
		imgui_node_exists = true;
		obj->m_draw_callback = draw_callback;
		obj->autorelease();
	} else {
		CC_SAFE_DELETE(obj);
	}
	return obj;
}

bool ImGuiNode::init() {
	if (!CCLayer::init()) return false;

	ImGui::CreateContext();

	auto& io = ImGui::GetIO();

	io.BackendPlatformUserData = this;
	io.BackendPlatformName = "cocos2d-2.2.3 GD";

	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	m_font_texture = new CCTexture2D;
	m_font_texture->initWithData(pixels, kCCTexture2DPixelFormat_RGBA8888, width, height, CCSize(static_cast<float>(width), static_cast<float>(height)));

	io.Fonts->SetTexID(reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(m_font_texture->getName())));

	// if using alks fixes comment this out
#ifndef GEODE_PLATFORM_NAME
	CCDirector::sharedDirector()->getTouchDispatcher()->incrementForcePrio(2);
#endif

	this->setTouchMode(kCCTouchesOneByOne);
	// this->setTouchPriority(-999);
	this->setTouchEnabled(true);
	
	this->setKeypadEnabled(true);
	this->setKeyboardEnabled(true);
	this->setMouseEnabled(true);

	return true;
}

void ImGuiNode::draw() {
	ccGLBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	this->new_frame();

	ImGui::NewFrame();

	m_draw_callback();

	ImGui::Render();

	this->render_draw_data(ImGui::GetDrawData());
}

void ImGuiNode::new_frame() {
	auto& io = ImGui::GetIO();
	
	// opengl2 new frame

	auto* director = CCDirector::sharedDirector();
	const auto win_size = director->getWinSize();
	const auto frame_size = director->getOpenGLView()->getFrameSize();

	// glfw new frame
	io.DisplaySize = ImVec2(frame_size.width, frame_size.height);
	io.DisplayFramebufferScale = ImVec2(
		win_size.width / frame_size.width,
		win_size.height / frame_size.height
	);
	io.DeltaTime = director->getDeltaTime();

#if _WIN32
	// not in cocos coords!! frame coords instead
	const auto mouse_pos = director->getOpenGLView()->getMousePosition();
	io.AddMousePosEvent(mouse_pos.x, mouse_pos.y);
#endif

	// this method kinda stinks since it prevents modifier keys
	// and other keys from being pressed while its active,
	// but its the best i can do thats cross platform
	// though if we wanted this in something like geode i think 
	// we should def hook the platform specific keyboard functions
	if (io.WantTextInput) {
		if (!m_ime_attached) {
			this->attachWithIME();
		}
		m_ime_attached = true;
	} else {
		if (m_ime_attached) {
			this->detachWithIME();
		}
		m_ime_attached = false;
	}

	auto* kb = director->getKeyboardDispatcher();
	io.KeyAlt = kb->getAltKeyPressed() || kb->getCommandKeyPressed(); // look
	io.KeyCtrl = kb->getControlKeyPressed();
	io.KeyShift = kb->getShiftKeyPressed();
}

bool ImGuiNode::canAttachWithIME() { return true; }
bool ImGuiNode::canDetachWithIME() { return true; }

void ImGuiNode::insertText(const char* text, int len) {
	std::string str(text, len);
	ImGui::GetIO().AddInputCharactersUTF8(str.c_str());
}

void ImGuiNode::deleteBackward() {
	// is this really how youre supposed to do this
	ImGui::GetIO().AddKeyEvent(ImGuiKey_Backspace, true);
	ImGui::GetIO().AddKeyEvent(ImGuiKey_Backspace, false);
}

bool ImGuiNode::ccTouchBegan(CCTouch* touch, CCEvent*) {
	auto& io = ImGui::GetIO();
	const auto pos = cocos_to_frame(touch->getLocation());
	io.AddMousePosEvent(pos.x, pos.y);
	// avoid getting mouse stuck in down
	// since ccTouchEnded is only called when we swallow
	if (io.WantCaptureMouse) {
		io.AddMouseButtonEvent(0, true);
	} else {
		io.AddMouseButtonEvent(0, true);
		io.AddMouseButtonEvent(0, false);
	}
	return io.WantCaptureMouse;
}

void ImGuiNode::ccTouchEnded(CCTouch* touch, CCEvent*) {
	const auto pos = cocos_to_frame(touch->getLocation());
	ImGui::GetIO().AddMousePosEvent(pos.x, pos.y);
	ImGui::GetIO().AddMouseButtonEvent(0, false);
}

void ImGuiNode::ccTouchMoved(CCTouch* touch, CCEvent*) {
	const auto pos = cocos_to_frame(touch->getLocation());
	ImGui::GetIO().AddMousePosEvent(pos.x, pos.y);
}

void ImGuiNode::scrollWheel(float dy, float dx) {
	ImGui::GetIO().AddMouseWheelEvent(dx, dy / -6.f);
}

void ImGuiNode::keyDown(cocos2d::enumKeyCodes key) {
	ImGui::GetIO().AddKeyEvent(cocos_to_imkey(key), true);
}

void ImGuiNode::keyUp(cocos2d::enumKeyCodes key) {
	ImGui::GetIO().AddKeyEvent(cocos_to_imkey(key), false);
}

// these dont ever get called :(
void ImGuiNode::rightKeyDown() {
	ImGui::GetIO().AddMouseButtonEvent(1, true);
}

void ImGuiNode::rightKeyUp() {
	ImGui::GetIO().AddMouseButtonEvent(1, false);
}

void ImGuiNode::render_draw_data(ImDrawData* draw_data) {
	const auto frame_size = CCDirector::sharedDirector()->getOpenGLView()->getFrameSize();
	const auto win_size = CCDirector::sharedDirector()->getWinSize();
	const auto frame_to_cocos = [&](const ImVec2 pos) {
		return CCPoint(
			pos.x / frame_size.width * win_size.width,
			(1.f - pos.y / frame_size.height) * win_size.height
		);
	};

	glEnable(GL_SCISSOR_TEST);

	const auto clip_scale = draw_data->FramebufferScale;

	for (int i = 0; i < draw_data->CmdListsCount; ++i) {
		auto* list = draw_data->CmdLists[i];
		auto* idx_buffer = list->IdxBuffer.Data;
		auto* vtx_buffer = list->VtxBuffer.Data;
		for (auto& cmd : list->CmdBuffer) {
			const auto tex_id = reinterpret_cast<std::uintptr_t>(cmd.GetTexID());
			ccGLBindTexture2D(static_cast<GLuint>(tex_id));

			const auto rect = cmd.ClipRect;
			const auto orig = frame_to_cocos(ImVec2(rect.x, rect.y));
			const auto end = frame_to_cocos(ImVec2(rect.z, rect.w));
			if (end.x <= orig.x || end.y >= orig.y)
				continue;
			CCDirector::sharedDirector()->getOpenGLView()->setScissorInPoints(orig.x, end.y, end.x - orig.x, orig.y - end.y);

			for (unsigned int i = 0; i < cmd.ElemCount; i += 3) {
				const auto a = vtx_buffer[idx_buffer[cmd.IdxOffset + i + 0]];
				const auto b = vtx_buffer[idx_buffer[cmd.IdxOffset + i + 1]];
				const auto c = vtx_buffer[idx_buffer[cmd.IdxOffset + i + 2]];
				std::array<CCPoint, 3> points = {
					frame_to_cocos(a.pos),
					frame_to_cocos(b.pos),
					frame_to_cocos(c.pos),
				};
				const auto ccc4_from_imcolor = [](const ImColor color) {
					// beautiful
					return ccc4f(color.Value.x, color.Value.y, color.Value.z, color.Value.w);
				};
				std::array<ccColor4F, 3> colors = {
					ccc4_from_imcolor(a.col),
					ccc4_from_imcolor(b.col),
					ccc4_from_imcolor(c.col),
				};

				std::array<CCPoint, 3> uvs = {
					ccp(a.uv.x, a.uv.y),
					ccp(b.uv.x, b.uv.y),
					ccp(c.uv.x, c.uv.y),
				};

				draw_triangle(points, colors, uvs);
			}
		}
	}

	glDisable(GL_SCISSOR_TEST);
}

ImGuiNode::~ImGuiNode() {
	ImGui::DestroyContext();
	imgui_node_exists = false;
	delete m_font_texture;
}
