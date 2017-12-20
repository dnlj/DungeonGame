// Engine
#include <Engine/Entity.hpp>

namespace Engine {
	Entity::Entity(ECS::EntityID eid) : eid{eid} {
	}

	ECS::EntityID Entity::getID() const {
		return eid;
	};

	void Entity::addComponent(ECS::ComponentID cid) {
		ECS::addComponent(eid, cid);
	};

	bool Entity::hasComponent(ECS::ComponentID cid) const {
		return ECS::hasComponent(eid, cid);
	}

	void Entity::removeComponent(ECS::ComponentID cid) {
		ECS::removeComponent(eid, cid);
	}
}