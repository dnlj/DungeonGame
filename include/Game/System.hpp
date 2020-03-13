#pragma once

// STD
#include <tuple>

// Engine
#include <Engine/EngineInstance.hpp>


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

			void setup() {}
			void tick(float dt) {}
			void run(float dt) {}
	};
}