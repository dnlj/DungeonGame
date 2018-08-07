#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Engine
#include <Engine/Debug/DebugDrawBox2D.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/PhysicsUserData.hpp>


namespace Game {
	// TODO: Move
	// TODO: Doc
	class PhysicsListener {
		public:
			virtual ~PhysicsListener() {};

			virtual void beginContact(const PhysicsUserData& dataA, const PhysicsUserData& dataB) {};
			virtual void endContact(const PhysicsUserData& dataA, const PhysicsUserData& dataB) {};
			//TODO: virtual void preSolve() {};
			//TODO: virtual void postSolve() {};
	};


	class PhysicsSystem : public SystemBase {
		public:
			PhysicsSystem(World& world);

			virtual void run(float dt) override;

			b2Body* createBody(Engine::ECS::Entity ent, b2BodyDef& bodyDef);


			#if defined(DEBUG_PHYSICS)
				Engine::Debug::DebugDrawBox2D& getDebugDraw();
			#endif

		private:
			class ContactListener : public b2ContactListener {
				public:
					ContactListener(PhysicsSystem& physSys);
					virtual void BeginContact(b2Contact* contact) override;
					virtual void EndContact(b2Contact* contact) override;
					// virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
					// virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

					void addListener(PhysicsListener* listener);

				private:
					PhysicsSystem& physSys;
					std::vector<PhysicsListener*> listeners;
			};

			const PhysicsUserData& getUserData(void* ptr) const;

			b2World physWorld;

			ContactListener contactListener;

			std::vector<PhysicsUserData> userData;

			#if defined(DEBUG_PHYSICS)
				Engine::Debug::DebugDrawBox2D debugDraw;
			#endif
	};
}
