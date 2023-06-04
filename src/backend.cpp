#include <Geode/Geode.hpp>
#include <imgui.h>
#include <imgui-cocos.hpp>

using namespace geode::prelude;

ImGuiCocos& ImGuiCocos::get() {
	static ImGuiCocos inst;
	return inst;
}

ImGuiCocos::ImGuiCocos() {
	m_setupCall = m_drawCall = [] {};
}

ImGuiCocos& ImGuiCocos::setup(std::function<void()> fun) {
	m_setupCall = fun;
	return this->setup();
}

ImGuiCocos& ImGuiCocos::draw(std::function<void()> fun) {
	m_drawCall = fun;
	return *this;
}

void ImGuiCocos::toggle() {
	this->setVisible(!m_visible);
}

void ImGuiCocos::setVisible(bool v) {
	m_visible = v;
	if (!m_visible) {
		auto& io = ImGui::GetIO();
		io.WantCaptureKeyboard = false;
		io.WantCaptureMouse = false;
		io.WantTextInput = false;
	}
}

bool ImGuiCocos::isVisible() {
	return m_visible;
}

ImGuiCocos& ImGuiCocos::setup() {
	if (m_initialized) return *this;

	ImGui::CreateContext();

	auto& io = ImGui::GetIO();

	io.BackendPlatformName = "cocos2d-2.2.3 GD";
	io.BackendPlatformUserData = this;

	m_initialized = true;

	// call the setup function before creating the font texture,
	// to allow for custom fonts
	m_setupCall();

	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	m_fontTexture = new CCTexture2D;
	m_fontTexture->initWithData(pixels, kCCTexture2DPixelFormat_RGBA8888, width, height, CCSize(static_cast<float>(width), static_cast<float>(height)));

	io.Fonts->SetTexID(reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(m_fontTexture->getName())));

	return *this;
}

void ImGuiCocos::destroy() {
	if (!m_initialized) return;

	ImGui::DestroyContext();
	delete m_fontTexture;
	m_initialized = false;
}

// based off ccDrawSolidPoly
// but modified to use textures
static void drawTriangle(const std::array<CCPoint, 3>& poli, const std::array<ccColor4F, 3>& colors, const std::array<CCPoint, 3>& uvs) {
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

ImVec2 ImGuiCocos::cocosToFrame(const CCPoint& pos) {
	auto* director = CCDirector::sharedDirector();
	const auto frameSize = director->getOpenGLView()->getFrameSize();
	const auto winSize = director->getWinSize();

	return ImVec2(
		pos.x / winSize.width * frameSize.width,
		(1.f - pos.y / winSize.height) * frameSize.height
	);
}

CCPoint ImGuiCocos::frameToCocos(const ImVec2& pos) {
	auto* director = CCDirector::sharedDirector();
	const auto frameSize = director->getOpenGLView()->getFrameSize();
	const auto winSize = director->getWinSize();

	return CCPoint(
		pos.x / frameSize.width * winSize.width,
		(1.f - pos.y / frameSize.height) * winSize.height
	);
}

void ImGuiCocos::drawFrame() {
	if (!m_initialized || !m_visible) return;

	ccGLBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// starts a new frame for imgui
	this->newFrame();
	ImGui::NewFrame();

	// actually draws stuff with imgui functions
	m_drawCall();

	// renders the triangles onto the screen
	ImGui::Render();
	this->renderFrame();
}

void ImGuiCocos::newFrame() {
	auto& io = ImGui::GetIO();
	
	// opengl2 new frame
	auto* director = CCDirector::sharedDirector();
	const auto winSize = director->getWinSize();
	const auto frameSize = director->getOpenGLView()->getFrameSize();

	// glfw new frame
	io.DisplaySize = ImVec2(frameSize.width, frameSize.height);
	io.DisplayFramebufferScale = ImVec2(
		winSize.width / frameSize.width,
		winSize.height / frameSize.height
	);
	io.DeltaTime = director->getDeltaTime();

	const auto mouse = cocosToFrame(geode::cocos::getMousePos());
	io.AddMousePosEvent(mouse.x, mouse.y);

	auto* kb = director->getKeyboardDispatcher();
	io.KeyAlt = kb->getAltKeyPressed() || kb->getCommandKeyPressed(); // look
	io.KeyCtrl = kb->getControlKeyPressed();
	io.KeyShift = kb->getShiftKeyPressed();
}

void ImGuiCocos::renderFrame() {
	auto* drawData = ImGui::GetDrawData();

	const auto frameSize = CCDirector::sharedDirector()->getOpenGLView()->getFrameSize();
	const auto winSize = CCDirector::sharedDirector()->getWinSize();
	const auto frameToCocos = [&](const ImVec2 pos) {
		return CCPoint(
			pos.x / frameSize.width * winSize.width,
			(1.f - pos.y / frameSize.height) * winSize.height
		);
	};

	glEnable(GL_SCISSOR_TEST);

	for (int i = 0; i < drawData->CmdListsCount; ++i) {
		auto* list = drawData->CmdLists[i];
		auto* idxBuffer = list->IdxBuffer.Data;
		auto* vtxBuffer = list->VtxBuffer.Data;
		for (auto& cmd : list->CmdBuffer) {
			const auto textureID = reinterpret_cast<std::uintptr_t>(cmd.GetTexID());
			ccGLBindTexture2D(static_cast<GLuint>(textureID));

			const auto rect = cmd.ClipRect;
			const auto orig = frameToCocos(ImVec2(rect.x, rect.y));
			const auto end = frameToCocos(ImVec2(rect.z, rect.w));

			if (end.x <= orig.x || end.y >= orig.y)
				continue;

			CCDirector::sharedDirector()->getOpenGLView()->setScissorInPoints(orig.x, end.y, end.x - orig.x, orig.y - end.y);

			for (unsigned int i = 0; i < cmd.ElemCount; i += 3) {
				const auto a = vtxBuffer[idxBuffer[cmd.IdxOffset + i + 0]];
				const auto b = vtxBuffer[idxBuffer[cmd.IdxOffset + i + 1]];
				const auto c = vtxBuffer[idxBuffer[cmd.IdxOffset + i + 2]];
				
				const std::array<CCPoint, 3> points = {
					frameToCocos(a.pos),
					frameToCocos(b.pos),
					frameToCocos(c.pos),
				};

				static constexpr auto ccc4FromImColor = [](const ImColor color) {
					// beautiful
					return ccc4f(color.Value.x, color.Value.y, color.Value.z, color.Value.w);
				};

				const std::array<ccColor4F, 3> colors = {
					ccc4FromImColor(a.col),
					ccc4FromImColor(b.col),
					ccc4FromImColor(c.col),
				};

				const std::array<CCPoint, 3> uvs = {
					ccp(a.uv.x, a.uv.y),
					ccp(b.uv.x, b.uv.y),
					ccp(c.uv.x, c.uv.y),
				};

				drawTriangle(points, colors, uvs);
			}
		}
	}

	glDisable(GL_SCISSOR_TEST);
}
