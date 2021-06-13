#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// STD
#include <string>

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Graphics {
	class Shader {
		private:
			GLuint program = 0;

		public:
			Shader() noexcept = default;
			Shader(const std::string& path);
			Shader(Shader&& other) noexcept;
			Shader& operator=(Shader&& other) noexcept;

			~Shader();

			void load(const std::string& path);
			void unload();

			ENGINE_INLINE auto get() const noexcept { return program; }
	};
};
