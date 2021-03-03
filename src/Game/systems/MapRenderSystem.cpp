// STD
#include <algorithm>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Game
#include <Game/systems/MapSystem.hpp>
#include <Game/systems/MapRenderSystem.hpp>
#include <Game/World.hpp>


namespace Game {
	MapRenderSystem::MapRenderSystem(SystemArg arg) : System{arg} {
		static_assert(World::orderAfter<MapRenderSystem, MapSystem>());
	}

	MapRenderSystem::~MapRenderSystem() {
	}

	// TODO: this should probably be part of a more generic render system.
	void MapRenderSystem::run(float dt) {
		const auto& mapSys = world.getSystem<MapSystem>();
		// TODO: these should be part of model/mesh
		auto& shader = mapSys.shader;
		auto& texture = mapSys.texture;

		glUseProgram(*shader);

		// Setup Texture
		glBindTextureUnit(0, texture->get());
		glUniform1i(5, 0);

		const auto vp = engine.camera.getProjection() * engine.camera.getView();


		const auto bounds = engine.camera.getWorldScreenBounds();
		const auto minChunk = MapSystem::blockToChunk(mapSys.worldToBlock(bounds.min));
		const auto maxChunk = MapSystem::blockToChunk(mapSys.worldToBlock(bounds.max));

		for (auto x = minChunk.x; x <= maxChunk.x; ++x) {
			for (auto y = minChunk.y; y <= maxChunk.y; ++y) {
				const auto found = mapSys.activeChunks.find({x, y});
				if (found == mapSys.activeChunks.cend()) { continue; }

				const auto pos = found->second.body->GetPosition();
				const auto mvp = glm::translate(vp, glm::vec3(pos.x, pos.y, 0.0f));
				glUniformMatrix4fv(1, 1, GL_FALSE, &mvp[0][0]);
				found->second.mesh.draw();
			}
		}
	}
}
