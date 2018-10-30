#pragma once

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// Engine
#include <Engine/ECS/World.hpp>
#include <Engine/SystemBase.hpp>


namespace Game {
	class PhysicsSystem;
	class SpriteSystem;
	class CharacterMovementSystem;
	class CharacterSpellSystem;
	class CameraTrackingSystem;
	class MapRenderSystem;
	class MapSystem;

	class PhysicsComponent;
	class SpriteComponent;
	class CharacterMovementComponent;
	class CharacterSpellComponent;
	class InputComponent;

	using SystemsSet = Meta::TypeSet::TypeSet<
		PhysicsSystem,
		SpriteSystem,
		CharacterMovementSystem,
		CharacterSpellSystem,
		CameraTrackingSystem,
		MapRenderSystem,
		MapSystem
	>;
	
	using ComponentsSet = Meta::TypeSet::TypeSet<
		PhysicsComponent,
		SpriteComponent,
		CharacterMovementComponent,
		CharacterSpellComponent,
		InputComponent
	>;

	using World = Engine::ECS::World<SystemsSet, ComponentsSet>;
	using SystemBase = Engine::SystemBase<World>;
}
