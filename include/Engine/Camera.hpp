#pragma once

// GLM
#include <glm/glm.hpp>


// TODO: Doc
namespace Engine {
	class Camera {
		public:
			void setAsOrtho(unsigned int width, unsigned int height, float scale);
			unsigned int getWidth() const;
			unsigned int getHeight() const;
			float getScale() const;

			/**
			 * @param[in] position The position vector.
			 */
			void setPosition(const glm::vec2& position);

			/**
			 * @return The position vector.
			 */
			glm::vec3 getPosition() const;

			/**
			 * @return The projection matrix.
			 */
			const glm::mat4& getProjection() const;

			/**
			 * @return The view matrix.
			 */
			const glm::mat4& getView() const;

			/**
			 * Converts a point from screen space to world space.
			 * @return The world space position of the point.
			 */
			glm::vec2 screenToWorld(glm::vec2 point) const;

		private:
			glm::mat4 view{1.0f};
			glm::mat4 projection{1.0f};
			unsigned int width = 0;
			unsigned int height = 0;
			float scale = 1.0f;
	};
}
