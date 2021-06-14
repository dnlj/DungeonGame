#pragma once

// STD
#include <vector>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/EngineInstance.hpp>
#include <Engine/Gui/Panel.hpp>
#include <Engine/Input/InputEvent.hpp>


namespace Engine::Gui {
	class Context {
		private:
			struct BFSStateData {
				glm::vec2 offset;
				const Panel* panel;
			};

			struct Vertex {
				glm::vec4 color;
				glm::vec2 pos;
				uint16 id;
				uint16 pid;
			}; static_assert(sizeof(Vertex) == sizeof(GLfloat) * 6 + sizeof(uint16) * 2);
			
			struct MultiDrawData {
				std::vector<GLint> first;
				std::vector<GLsizei> count;
			};
		private:
			std::vector<Panel*> hoverStack;
			std::vector<Vertex> verts;
			MultiDrawData multiDrawData;
			std::vector<BFSStateData> bfsCurr;
			std::vector<BFSStateData> bfsNext;

			constexpr static GLuint vertBindingIndex = 0;
			GLuint fbo = 0;
			GLuint vao = 0;
			GLuint vbo = 0;
			GLsizei vboCapacity = 0;
			ShaderRef shader;
			Texture2D clipTex;
			glm::vec4 color = {1.0f, 0.0f, 0.0f, 0.2f};

			ShaderRef quadShader;
			GLuint quadVAO;
			GLuint quadVBO;

			glm::vec2 view;
			glm::vec2 offset;
			glm::vec2 cursor = {};

			Panel* root;
			Panel* active = nullptr;
			bool hoverValid = false;

		public:
			// TODO: split shader into own class so we dont depend on engine
			Context(Engine::EngineInstance& engine);
			~Context();
			void render();
			void addRect(const glm::vec2 pos, const glm::vec2 size);
			void updateHover();

			/**
			 * Checks if we are hovering any panel.
			 */
			ENGINE_INLINE bool isHoverAny() const noexcept { return !hoverStack.empty(); }

			/**
			 * Gets the most hovered panel.
			 */
			ENGINE_INLINE Panel* getHover() noexcept { return isHoverAny() ? hoverStack.back() : nullptr; }
			ENGINE_INLINE const Panel* getHover() const noexcept { return const_cast<Context*>(this)->getHover(); }

			/**
			 * Gets the most focused panel.
			 */
			ENGINE_INLINE Panel* getFocus() noexcept { return getHover(); } // TODO: actual focus logic.
			ENGINE_INLINE const Panel* getFocus() const noexcept { return getHover(); }
			
			/**
			 * Gets the active panel.
			 */
			ENGINE_INLINE Panel* getActive() noexcept { return active; }
			ENGINE_INLINE const Panel* getActive() const noexcept { return active; }

			/**
			 * @return Indicate if the input was consumed.
			 */
			bool onMouse(const Engine::Input::InputEvent event);

			/**
			 * @return Indicate if the input was consumed.
			 */
			bool onMouseMove(const Engine::Input::InputEvent event);

			/**
			 * @return Indicate if the input was consumed.
			 */
			bool onMouseWheel(const Engine::Input::InputEvent event);
			
			/**
			 * @return Indicate if the input was consumed.
			 */
			bool onKey(const Engine::Input::InputEvent event);
			
			/**
			 * @return Indicate if the input was consumed.
			 */
			bool onChar(const wchar_t ch);// TODO: How to support full unicode and "Win + ." emoji picker? Look into WM_IME_* messages

			void onResize(const int32 w, const int32 h);
			void onFocus(const bool has);
	};
}
