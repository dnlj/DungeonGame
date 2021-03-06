#pragma once

// STD
#include <tuple>

// Engine
#include <Engine/EngineInstance.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/RenderLayer.hpp>


namespace Game {
	class World; // Forward declaration

	/** The argument type system constructors require. */
	using SystemArg = const std::tuple<World&, Engine::EngineInstance&>&;

	/**
	 * A base class for game systems.
	 */
	class System {
		protected:
			World& world;
			Engine::EngineInstance& engine;

		public:
			System(SystemArg arg)
				: world{std::get<World&>(arg)}
				, engine{std::get<Engine::EngineInstance&>(arg)} {
			};

			ENGINE_INLINE void setup() {}

			ENGINE_INLINE void preTick() {}
			ENGINE_INLINE void tick() {}
			ENGINE_INLINE void postTick() {}

			ENGINE_INLINE void run(float32 dt) {}
			ENGINE_INLINE void render(const RenderLayer layer) {}

			ENGINE_INLINE void preStoreSnapshot() {}
			ENGINE_INLINE void postLoadSnapshot() {}
	};
}
