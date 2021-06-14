// Engine
#include <Engine/Gui/Context.hpp>


namespace Engine::Gui {
	Context::Context(Engine::EngineInstance& engine) {
		shader = engine.shaderManager.get("shaders/gui");
		view = engine.camera.getScreenSize(); // TODO: should update when resized

		{
			quadShader = engine.shaderManager.get("shaders/fullscreen_passthrough");
			const glm::vec2 quadData[] = {
				{1,1}, {-1,1}, {-1,-1},
				{-1,-1}, {1,-1}, {1,1},
			};

			// TODO: passthrough prgoram
			glCreateBuffers(1, &quadVBO);
			glNamedBufferData(quadVBO, sizeof(quadData), &quadData, GL_STATIC_DRAW);

			glCreateVertexArrays(1, &quadVAO);
			glVertexArrayVertexBuffer(quadVAO, 0, quadVBO, 0, sizeof(quadData[0]));
			glEnableVertexArrayAttrib(quadVAO, 0);
			glVertexArrayAttribBinding(quadVAO, 0, 0);
			glVertexArrayAttribFormat(quadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
		}

		glCreateFramebuffers(1, &fbo);

		glCreateVertexArrays(1, &vao);
		glCreateBuffers(1, &vbo);
		glVertexArrayVertexBuffer(vao, vertBindingIndex, vbo, 0, sizeof(Vertex));

		// Vertex
		glEnableVertexArrayAttrib(vao, 0);
		glVertexArrayAttribBinding(vao, 0, vertBindingIndex);
		glVertexArrayAttribFormat(vao, 0, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));

		glEnableVertexArrayAttrib(vao, 1);
		glVertexArrayAttribBinding(vao, 1, vertBindingIndex);
		glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));

		//glEnableVertexArrayAttrib(vao, 2);
		//glVertexArrayAttribBinding(vao, 2, vertBindingIndex);
		//glVertexArrayAttribFormat(vao, 2, 1, GL_UNSIGNED_SHORT, GL_FALSE, offsetof(Vertex, id));
		//
		//glEnableVertexArrayAttrib(vao, 3);
		//glVertexArrayAttribBinding(vao, 3, vertBindingIndex);
		//glVertexArrayAttribFormat(vao, 3, 1, GL_UNSIGNED_SHORT, GL_FALSE, offsetof(Vertex, pid));

		root = new Panel{};
		root->setPos({25, 50});
		root->setSize({512, 256});

		{
			auto child = root->addChild(new Panel{});
			child->setPos({0, 0});
			child->setSize({64, 64});

			auto childChild = child->addChild(new Panel{});
			childChild->setPos({0, 0});
			childChild->setSize({32, 32});
		}
		
		{
			auto child = root->addChild(new Panel{});
			child->setPos({128, 5});
			child->setSize({32, 64});
		}
	}

	Context::~Context() {
		glDeleteVertexArrays(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);

		glDeleteFramebuffers(1, &fbo);
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		delete root;
	}

	void Context::render() {
		if (!hoverValid) {
			updateHover();
			hoverValid = true;
		}

		const Panel* curr = root;
		offset = {};
		multiDrawData.first.emplace_back() = 0;
		
		// Breadth first traversal
		while (true) {
			while (curr) {
				if (curr->firstChild) {
					bfsNext.emplace_back(
						offset + curr->getPos(),
						curr->firstChild
					);
				}

				if (false) {}
				else if (curr == getActive()) { color = glm::vec4{1, 0, 1, 0.2}; }
				else if (curr == getHover()) { color = glm::vec4{1, 1, 0, 0.2}; }
				else { color = glm::vec4{1, 0, 0, 0.2}; }

				curr->render(*this);
				curr = curr->nextSibling;
			}

			if (bfsCurr.empty()) {
				const auto vsz = static_cast<GLint>(verts.size());
				multiDrawData.count.emplace_back() = vsz - multiDrawData.first.back();

				bfsCurr.swap(bfsNext);
				if (bfsCurr.empty()) { break; }

				multiDrawData.first.emplace_back() = vsz;
			}

			const auto& back = bfsCurr.back();
			curr = back.panel;
			offset = back.offset;
			bfsCurr.pop_back();
		}

		{
			const auto size = verts.size() * sizeof(verts[0]);
			if (size > vboCapacity) {
				vboCapacity = static_cast<GLsizei>(verts.capacity() * sizeof(verts[0]));
				glNamedBufferData(vbo, vboCapacity, nullptr, GL_DYNAMIC_DRAW);
			}
			glNamedBufferSubData(vbo, 0, size, verts.data());
		}

		glBindVertexArray(vao);
		glUseProgram(shader->get());
		glUniform2fv(0, 1, &view.x);

		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		// If we dont care about clipping we can just do
		//glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(verts.size()));

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		const glm::vec4 clear = {0,0,0,0};
		glClearNamedFramebufferfv(fbo, GL_COLOR, 0, &clear.x);
		// TODO: clear cliptext
		// TODO: do we really need a second pass even? cant we just use multiple render targest to have the second texture?

		glMultiDrawArrays(GL_TRIANGLES,
			multiDrawData.first.data(),
			multiDrawData.count.data(),
			static_cast<GLsizei>(multiDrawData.count.size())
		);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glUseProgram(quadShader->get());
		glBindTextureUnit(0, clipTex.get());
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisable(GL_BLEND);
		verts.clear();
		multiDrawData.first.clear();
		multiDrawData.count.clear();
	}

	void Context::addRect(const glm::vec2 pos, const glm::vec2 size) {
		verts.push_back({.color = color, .pos = offset + pos});
		verts.push_back({.color = color, .pos = offset + pos + glm::vec2{0, size.y}});
		verts.push_back({.color = color, .pos = offset + pos + size});

		verts.push_back({.color = color, .pos = offset + pos + size});
		verts.push_back({.color = color, .pos = offset + pos + glm::vec2{size.x, 0}});
		verts.push_back({.color = color, .pos = offset + pos});
	}

	void Context::updateHover() {
		Panel* const old = getHover();
		glm::vec2 off = {};
		Panel* curr = nullptr;

		// Rebuild offset and find where our old stack ends
		{
			auto it = hoverStack.begin();
			auto end = hoverStack.end();

			for (; it != end; ++it) {
				auto* panel = *it;

				if (panel->canHover() && panel->parent == curr && (panel->getBounds() + off).contains(cursor)) {
					off += panel->getPos();
					curr = panel;
				} else {
					break;
				}
			}

			if (it != end) {
				const auto start = it;
				auto next = it + 1;
				while (next != end) {
					(*it)->onEndChildHover(*next);
					it = next;
					++next;
				}

				hoverStack.erase(start, end);
			}
		}

		// Setup the next panel to check
		curr = curr ? curr->firstChild : root;

		while (curr) {
			if (curr->canHover() && (curr->getBounds() + off).contains(cursor)) {
				if (curr->parent && curr->parent->onBeginChildHover(curr)) {
					break;
				}

				off += curr->getPos();
				hoverStack.push_back(curr);
				curr = curr->firstChild;
			} else {
				curr = curr->nextSibling;
			}
		}

		Panel* target = getHover();
		if (target != old) {
			if (old) { old->onEndHover(); };
			if (target) { target->onBeginHover(); }
		}
	}

	bool Context::onMouse(const Engine::Input::InputEvent event) {
		//ENGINE_LOG("onMouse:",
		//	" ", event.state.value,
		//	" ", (int)event.state.id.code,
		//	" ", (int)event.state.id.type,
		//	" ", (int)event.state.id.device,
		//	" @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count()
		//);
		if (event.state.id.code == 0) {
			if (event.state.value && isHoverAny()) {
				ENGINE_DEBUG_ASSERT(active == nullptr);
				auto focus = getFocus();
				if (!focus) { return false; }
				focus->onBeginActivate();
				active = focus;
				return true;
			} else if (active) {
				active->onEndActivate();
				active = nullptr;
				return true;
			}
		}
		return false;
	}

	bool Context::onMouseMove(const Engine::Input::InputEvent event) {
		//ENGINE_LOG("onMouseMove:", " ", event.state.id.code, " ", event.state.valuef, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());
		if (event.state.id.code == 0) {
			cursor.x = event.state.valuef;
		} else {
			cursor.y = event.state.valuef;
		}
		hoverValid = false;
		return isHoverAny();
	}

	bool Context::onMouseWheel(const Engine::Input::InputEvent event) {
		// ENGINE_LOG("onMouseWheel: ", event.state.value, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());
		return false;
	}

	bool Context::onKey(const Engine::Input::InputEvent event) {
		// ENGINE_LOG("onKey: ", event.state.value, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());
		return false;
	}

	bool Context::onChar(const wchar_t ch) {
		// ENGINE_LOG("onChar: ", (int)ch);
		return false;
	}

	void Context::onResize(const int32 w, const int32 h) {
		// ENGINE_LOG("onResize: ", w, " ", h);
		if (w == view.x && h == view.y) { return; }
		view = {w, h};
		// TODO: see if there are WM_* messages for end drag or something so we dont call this 100 times when resizing.
		//clipTex.setStorage(TextureFormat::R32U, view);
		clipTex.setStorage(TextureFormat::RGBA8, view);
		glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, clipTex.get(), 0);
		ENGINE_LOG("glCheckFramebufferStatus: ", glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	void Context::onFocus(const bool has) {
		//ENGINE_LOG("onFocus: ", has);
		if (!has) {
			hoverValid = true;

			// Remove any hovers
			if (!hoverStack.empty()) {
				auto it = hoverStack.begin();
				auto end = hoverStack.end();
				auto next = it + 1;

				while (next != end) {
					(*it)->onEndChildHover(*next);
					it = next;
					++next;
				}

				(*it)->onEndHover();
				hoverStack.clear();
			}
		} else {
			hoverValid = false;
		}
	}

}
