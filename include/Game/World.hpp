#pragma once

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// Engine
#include <Engine/ECS/World.hpp>
#include <Engine/EngineInstance.hpp>

// Game
#include <Game/Common.hpp>

#include <Game/systems/InputSystem.hpp>
#include <Game/systems/ActionSystem.hpp>
#include <Game/systems/CharacterMovementSystem.hpp>
#include <Game/systems/PhysicsOriginShiftSystem.hpp>
#include <Game/systems/PhysicsSystem.hpp>
#include <Game/systems/CharacterSpellSystem.hpp>
#include <Game/systems/CameraTrackingSystem.hpp>
#include <Game/systems/SubWorldSystem.hpp>
#include <Game/systems/MapSystem.hpp>
#include <Game/systems/MapRenderSystem.hpp>
#include <Game/systems/SpriteSystem.hpp>
#include <Game/systems/NetworkingSystem.hpp>
#include <Game/systems/UISystem.hpp>

#include <Game/comps/PhysicsComponent.hpp>
#include <Game/comps/SpriteComponent.hpp>
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/ConnectionComponent.hpp>
#include <Game/comps/NeighborsComponent.hpp>
#include <Game/comps/NetworkStatsComponent.hpp>


namespace Game {
	using SystemsSet = Meta::TypeSet::TypeSet<
		// Inputs
		InputSystem,
		////////////////
		////////////////
		////////////////
		////////////////
		////////////////
		////////////////
		////////////////
		////////////////
		// TODO: this may be the problem. We want ot run network before action so that we get all inputs.
		// but we also want ot run action before networking so we send the latest info...
		// I guess we could read in pre tick and write in post tick?
		// but dont we network in `run`? so it is always after anyways?????
		// This is likely the issue. Figure out when to run networking code.
		////////////////
		////////////////
		////////////////
		////////////////
		////////////////
		////////////////
		////////////////
		////////////////
		NetworkingSystem, // Needs to be before any game logic since we sync state at start of frame.
		ActionSystem,

		// Game Logic
		CharacterMovementSystem,
		PhysicsOriginShiftSystem,
		PhysicsSystem,
		CharacterSpellSystem,
		CameraTrackingSystem,
		//SubWorldSystem,
		MapSystem,

		// Rendering
		MapRenderSystem,
		SpriteSystem,
		UISystem
	>;
	
	using ComponentsSet = Meta::TypeSet::TypeSet<
		MapEditComponent,
		PhysicsComponent,
		SpriteComponent,
		ActionComponent,
		ConnectionComponent,
		NeighborsComponent,
		NetworkStatsComponent,
		struct CharacterSpellComponent, // TODO: rn xFlag?
		struct PlayerFlag
	>;

	// TODO: we could get rid of CRTP here by forward declaring all systems/components/flags...
	// TODO: cont. - (effectively forward decl the sets) and making World a typedef and templating the ECS::World constructor...
	// TODO: cont. - Would need to move the set defs into own file then. Not sure if worth. Probably is. CRTP is a little stinky.
	class World : public Engine::ECS::World<World, tickrate, snapshots, SystemsSet, ComponentsSet> {
		public:
			World(Engine::EngineInstance& engine)
				: Engine::ECS::World<World, tickrate, snapshots, SystemsSet, ComponentsSet>(std::tie(*this, engine)) {
			}
	};
}
