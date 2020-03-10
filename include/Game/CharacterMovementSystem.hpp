#pragma once

// Engine
#include <Engine/ECS/EntityFilter.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class CharacterMovementSystem : public SystemBase {
		public:
			CharacterMovementSystem(World& world);
			void run(float dt);

		private:
			Engine::ECS::EntityFilter& filter;
	};
}
