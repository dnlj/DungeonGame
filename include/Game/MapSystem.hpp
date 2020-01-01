#pragma once

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/EngineInstance.hpp>
#include <Engine/ShaderManager.hpp>

// Game
#include <Game/MapChunk.hpp>
#include <Game/MapGenerator.hpp>

// TODO: Document the different coordinate systems and terms used here.
// TODO: convert to use sized types - int32, float32, etc.
// TODO: Change all mentions of "tile" to "block". Its a more common term.
// TODO: Standardize terms to be `xyzOffset`, `xyzPosition`, `xyzIndex`
/**
 * World Coordinates - Always relative to Box2D origin. When mapOffset is changed
 * Region - A grouping of chunks. Used for saving/loading.
 *
 * xyzOffset - The position of xyz relative to the current origin shift.
 * xyzPos - The position of xyz in absolute terms. Always the same regardless of the origin shift.
 * xyzIndex - The index of xyz into its storage array.
 *
 */
namespace Game {
	class MapSystem : public SystemBase {
		public:
			MapSystem(World& world);
			void setup(Engine::EngineInstance& engine);
			void run(float dt) override;

			// TODO: Name? this isnt consistent with our other usage of offset
			const glm::ivec2& getChunkOffset() const; // TODO: Remove? Dont think this is used anywhere.

			// TODO: Name? this isnt consistent with our other usage of offset
			// TODO: Doc. Gets the size of the current offset in blocks coordinates
			glm::ivec2 getBlockOffset() const;

			void setValueAt(const glm::vec2 wpos);

			// TODO: Doc
			template<MapChunk::EditMemberFunction func>
			void applyEdit();

			// TODO: do we want these multi step conversion functions? or do we just want world>block>chunk>region
			/**
			 * Converts from a world position to chunk coordinates.
			 * @param[in] worldPos The position in world space.
			 * @return The chunk coordinates.
			 */
			glm::ivec2 worldToChunk(const glm::vec2 worldPos) const;

			/**
			 * Converts world coordinates to absolute block coordinates.
			 * TODO: Finish docs
			 */
			glm::ivec2 worldToBlock(const glm::vec2 worldPos) const;

			// TODO: Doc
			glm::ivec2 blockToChunk(const glm::ivec2 block) const;

			/**
			 * Converts from a chunk position to world coordinates.
			 * @param[in] chunkPos The position in chunk space.
			 * @return The world coordinates.
			 */
			glm::vec2 chunkToWorld(const glm::ivec2 chunkPos) const;

			/**
			 * Converts chunk coordinates to absolute block coordinates.
			 * TODO: Finish docs
			 */
			glm::ivec2 chunkToBlock(const glm::ivec2 chunkPos) const;

			// TODO: Doc
			glm::ivec2 chunkToRegion(const glm::ivec2 chunk);

			/**
			 * Converts from region coordinates to chunk coordinates.
			 * @param region The region coordinates.
			 * @return The chunk coordinates.
			 */
			glm::ivec2 regionToChunk(glm::ivec2 region);
		private:
			/**
			 * Get the chunk at a position.
			 * @param[in] pos The position in chunk coordinates.
			 * @return The chunk at the position.
			 */
			MapChunk& getChunkAt(glm::ivec2 pos);

			// TODO: doc
			void ensureRegionLoaded(glm::ivec2 region);

			// TODO: Doc
			MapChunk& ensureChunkLoaded(glm::ivec2 pos);

			// TODO: Doc
			void loadChunk(const glm::ivec2 pos);

			// TODO: Doc
			void loadRegion(const glm::ivec2 region);

			// TODO: Doc
			void updateOrigin();

			/** Number of chunks before shifting the origin */
			constexpr static int originRange = 4;

			/** Size of regions in chunks */
			//constexpr static glm::ivec2 regionSize = {16, 16};
			constexpr static glm::ivec2 regionSize = {3, 3};

			/** The number of regions in the map */
			constexpr static glm::ivec2 regionCount = {3, 3};

			/** Offset (in chunks) of current origin */
			glm::ivec2 mapOffset = {0, 0}; // TODO: is there a reason to not just use a block offset? We never use this without also referencing originRange anyways.

			/** The number of chunks in the map */
			constexpr static glm::ivec2 mapSize = regionCount * regionSize;

			MapChunk chunks[mapSize.x][mapSize.y];
			glm::ivec2 loadedRegions[regionCount.x][regionCount.y] = {};

			Engine::Input::InputManager* input;
			const Engine::Camera* camera;
			Engine::Shader shader;
			Engine::Texture texture;
			Game::MapGenerator<
				Game::BiomeA,
				Game::BiomeC
			> mgen{1234};
	};
}

#include <Game/MapSystem.ipp>
