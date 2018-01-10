// Engine
#include <Engine/Debug/Debug.hpp>
#include <Engine/Engine.hpp>

namespace Engine::Debug {
	void checkOpenGLErrors() {
		for (auto error = glGetError(); error != GL_NO_ERROR; error = glGetError()) {
			ENGINE_DEBUG(openGLErrorToString(error));
		}
	}

	void checkOpenGLShaderCompilation(GLuint shader) {
		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		if (!success) {
			GLint length = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

			std::string info(length, '\0');
			glGetShaderInfoLog(shader, length, nullptr, info.data());
			ENGINE_WARN(info);
		}
	}

	std::string openGLErrorToString(const GLint error) {
		switch (error) {
			case GL_NO_ERROR:
				return "GL_NO_ERROR";
			case GL_INVALID_ENUM:
				return "GL_INVALID_ENUM";
			case GL_INVALID_VALUE:
				return "GL_INVALID_VALUE";
			case GL_INVALID_OPERATION:
				return "GL_INVALID_OPERATION";
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				return "GL_INVALID_FRAMEBUFFER_OPERATION";
			case GL_OUT_OF_MEMORY:
				return "GL_OUT_OF_MEMORY";
			case GL_STACK_UNDERFLOW:
				return "GL_STACK_UNDERFLOW";
			case GL_STACK_OVERFLOW:
				return "GL_STACK_OVERFLOW";
			default:
				return "UNKNOWN(" + std::to_string(error) + ")";
		}
	}
}
