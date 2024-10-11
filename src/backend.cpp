#include <Geode/Geode.hpp>
#include <imgui.h>
#include <imgui-cocos.hpp>
#include <utility>

#ifdef GEODE_IS_WINDOWS
	// so msvc shuts up
	#define sscanf sscanf_s
#endif

using namespace geode::prelude;

ImGuiCocos& ImGuiCocos::get() {
	static ImGuiCocos inst;
	return inst;
}

ImGuiCocos::ImGuiCocos() {
	m_setupCall = m_drawCall = [] {};
}

ImGuiCocos& ImGuiCocos::setup(std::function<void()> fun) {
	m_setupCall = std::move(fun);
	return this->setup();
}

ImGuiCocos& ImGuiCocos::draw(std::function<void()> fun) {
	m_drawCall = std::move(fun);
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

bool ImGuiCocos::isVisible() const {
	return m_visible;
}

void ImGuiCocos::setInputMode(InputMode mode) {
	m_inputMode = mode;
}

ImGuiCocos::InputMode ImGuiCocos::getInputMode() {
	return m_inputMode;
}

void ImGuiCocos::setForceLegacy(bool force) {
	m_forceLegacy = force;
}

bool ImGuiCocos::getForceLegacy() const {
	return m_forceLegacy;
}

bool ImGuiCocos::isInitialized() const {
	return m_initialized;
}

ImGuiCocos& ImGuiCocos::setup() {
	if (m_initialized) return *this;

	ImGui::CreateContext();

	auto& io = ImGui::GetIO();

	static const int glVersion = [] {
	#if defined(GEODE_IS_ANDROID)
		// android uses GLES v2
		return 200;
	#endif
		int major = 0;
		int minor = 0;
	#if defined(GEODE_IS_WINDOWS)
		// macos opengl is really outdated, and doesnt have these enums
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
	#endif
		if (major == 0 && minor == 0) {
			auto* verStr = reinterpret_cast<const char*>(glGetString(GL_VERSION));
			if (!verStr || sscanf(verStr, "%d.%d", &major, &minor) != 2) {
				// failed to parse version string, just assume opengl 2.1
				return 210;
			}
		}
		return major * 100 + minor * 10;
	}();

	io.BackendPlatformName = "gd-imgui-cocos + Geode";
	io.BackendPlatformUserData = this;
	if (glVersion >= 320) {
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
	}

	// use static since imgui does not own the pointer!
	static const auto iniPath = (Mod::get()->getSaveDir() / "imgui.ini").string();
	io.IniFilename = iniPath.c_str();

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

void ImGuiCocos::reload() {
	m_reloading = true;
}

ImVec2 ImGuiCocos::cocosToFrame(const CCPoint& pos) {
	auto* director = CCDirector::sharedDirector();
	const auto frameSize = director->getOpenGLView()->getFrameSize() * geode::utils::getDisplayFactor();
	const auto winSize = director->getWinSize();

	return {
		pos.x / winSize.width * frameSize.width,
		(1.f - pos.y / winSize.height) * frameSize.height
	};
}

CCPoint ImGuiCocos::frameToCocos(const ImVec2& pos) {
	auto* director = CCDirector::sharedDirector();
	const auto frameSize = director->getOpenGLView()->getFrameSize() * geode::utils::getDisplayFactor();
	const auto winSize = director->getWinSize();

	return {
		pos.x / frameSize.width * winSize.width,
		(1.f - pos.y / frameSize.height) * winSize.height
	};
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

	// reload imgui context if requested
	if (m_reloading) {
		this->destroy();
		this->setup();
		m_reloading = false;
	}
}

void ImGuiCocos::newFrame() {
	auto& io = ImGui::GetIO();

	// opengl2 new frame
	auto* director = CCDirector::sharedDirector();
	const auto winSize = director->getWinSize();
	const auto frameSize = director->getOpenGLView()->getFrameSize() * geode::utils::getDisplayFactor();

	// glfw new frame
	io.DisplaySize = ImVec2(frameSize.width, frameSize.height);
	io.DisplayFramebufferScale = ImVec2(
		winSize.width / frameSize.width,
		winSize.height / frameSize.height
	);
	if (director->getDeltaTime() > 0.f) {
		io.DeltaTime = director->getDeltaTime();
	} else {
		io.DeltaTime = 1.f / 60.f;
	}

#ifdef GEODE_IS_DESKTOP
	const auto mouse = cocosToFrame(geode::cocos::getMousePos());
	io.AddMousePosEvent(mouse.x, mouse.y);
#endif

	auto* kb = director->getKeyboardDispatcher();
	io.KeyAlt = kb->getAltKeyPressed() || kb->getCommandKeyPressed(); // look
	io.KeyCtrl = kb->getControlKeyPressed();
	io.KeyShift = kb->getShiftKeyPressed();
}

static bool hasExtension(const std::string& ext) {
	static auto exts = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
	if (exts == nullptr)
		return false;

	return std::string(exts).find(ext) != std::string::npos;
}

static void drawTriangle(const std::array<CCPoint, 3>& poly, const std::array<ccColor4F, 3>& colors, const std::array<CCPoint, 3>& uvs) {
	auto* shader = CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor);
	shader->use();
	shader->setUniformsForBuiltins();

	ccGLEnableVertexAttribs(kCCVertexAttribFlag_PosColorTex);

	static_assert(sizeof(CCPoint) == sizeof(ccVertex2F), "so the cocos devs were right then");

	glVertexAttribPointer(kCCVertexAttrib_Position, 2, GL_FLOAT, GL_FALSE, 0, poly.data());
	glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_FLOAT, GL_FALSE, 0, colors.data());
	glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, 0, uvs.data());

	glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
}

void ImGuiCocos::legacyRenderFrame() {
	glEnable(GL_SCISSOR_TEST);

	auto* drawData = ImGui::GetDrawData();

	for (int i = 0; i < drawData->CmdListsCount; ++i) {
		auto* list = drawData->CmdLists[i];
		auto* idxBuffer = list->IdxBuffer.Data;
		auto* vtxBuffer = list->VtxBuffer.Data;
		for (auto& cmd : list->CmdBuffer) {
			ccGLBindTexture2D(static_cast<GLuint>(reinterpret_cast<intptr_t>(cmd.GetTexID())));

			const auto rect = cmd.ClipRect;
			const auto orig = frameToCocos(ImVec2(rect.x, rect.y));
			const auto end = frameToCocos(ImVec2(rect.z, rect.w));
			if (end.x <= orig.x || end.y >= orig.y)
				continue;
			CCDirector::sharedDirector()->getOpenGLView()->setScissorInPoints(orig.x, end.y, end.x - orig.x, orig.y - end.y);

			for (unsigned int j = 0; j < cmd.ElemCount; j += 3) {
				const auto a = vtxBuffer[idxBuffer[cmd.IdxOffset + j + 0]];
				const auto b = vtxBuffer[idxBuffer[cmd.IdxOffset + j + 1]];
				const auto c = vtxBuffer[idxBuffer[cmd.IdxOffset + j + 2]];
				std::array<CCPoint, 3> points = {
					frameToCocos(a.pos),
					frameToCocos(b.pos),
					frameToCocos(c.pos),
				};
				static constexpr auto ccc4FromImColor = [](const ImColor color) {
					// beautiful
					return ccc4f(color.Value.x, color.Value.y, color.Value.z, color.Value.w);
				};
				std::array<ccColor4F, 3> colors = {
					ccc4FromImColor(a.col),
					ccc4FromImColor(b.col),
					ccc4FromImColor(c.col),
				};

				std::array<CCPoint, 3> uvs = {
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

void ImGuiCocos::renderFrame() const {
#ifdef GEODE_IS_MACOS
	static bool hasVAO = hasExtension("GL_APPLE_vertex_array_object");
#else
	static bool hasVAO = hasExtension("GL_ARB_vertex_array_object");
#endif
	if (!hasVAO || m_forceLegacy)
		return legacyRenderFrame();

	auto* drawData = ImGui::GetDrawData();

	const bool hasVtxOffset = ImGui::GetIO().BackendFlags | ImGuiBackendFlags_RendererHasVtxOffset;

	glEnable(GL_SCISSOR_TEST);

	GLuint vao = 0;
	GLuint vbos[2] = {0, 0};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(2, &vbos[0]);

	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[1]);

	glEnableVertexAttribArray(kCCVertexAttrib_Position);
	glVertexAttribPointer(kCCVertexAttrib_Position, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), reinterpret_cast<void*>(offsetof(ImDrawVert, pos)));

	glEnableVertexAttribArray(kCCVertexAttrib_TexCoords);
	glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), reinterpret_cast<void*>(offsetof(ImDrawVert, uv)));

	glEnableVertexAttribArray(kCCVertexAttrib_Color);
	glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), reinterpret_cast<void*>(offsetof(ImDrawVert, col)));

	auto* shader = CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor);
	shader->use();
	shader->setUniformsForBuiltins();

	for (int i = 0; i < drawData->CmdListsCount; ++i) {
		auto* list = drawData->CmdLists[i];

		// convert vertex coords to cocos space
		for (auto& j : list->VtxBuffer) {
			const auto point = frameToCocos(j.pos);
			j.pos = ImVec2(point.x, point.y);
		}

		glBufferData(GL_ARRAY_BUFFER, list->VtxBuffer.Size * sizeof(ImDrawVert), list->VtxBuffer.Data, GL_STREAM_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, list->IdxBuffer.Size * sizeof(ImDrawIdx), list->IdxBuffer.Data, GL_STREAM_DRAW);

		for (auto& cmd : list->CmdBuffer) {
			if (cmd.UserCallback != nullptr) {
				cmd.UserCallback(list, &cmd);
				continue;
			}

			const auto textureID = reinterpret_cast<std::uintptr_t>(cmd.GetTexID());
			ccGLBindTexture2D(static_cast<GLuint>(textureID));

			const auto rect = cmd.ClipRect;
			const auto orig = frameToCocos(ImVec2(rect.x, rect.y));
			const auto end = frameToCocos(ImVec2(rect.z, rect.w));

			if (end.x <= orig.x || end.y >= orig.y)
				continue;

			CCDirector::sharedDirector()->getOpenGLView()->setScissorInPoints(orig.x, end.y, end.x - orig.x, orig.y - end.y);

			if (hasVtxOffset) {
			#if !defined(GEODE_IS_ANDROID)
				glDrawElementsBaseVertex(GL_TRIANGLES, cmd.ElemCount, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(cmd.IdxOffset * sizeof(ImDrawIdx)), cmd.VtxOffset);
			#endif
			} else {
				glDrawElements(GL_TRIANGLES, cmd.ElemCount, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(cmd.IdxOffset * sizeof(ImDrawIdx)));
			}
		}
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDeleteBuffers(2, &vbos[0]);
	glDeleteVertexArrays(1, &vao);

	glDisable(GL_SCISSOR_TEST);
}
