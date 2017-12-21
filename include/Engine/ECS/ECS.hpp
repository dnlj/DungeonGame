#pragma once

// STD
#include <vector>
#include <array>
#include <unordered_map>
#include <string>
#include <string_view>
#include <queue>
#include <bitset>

/**
 * @brief Registers a component for use with the Entity Component System.
 */
#define ENGINE_REGISTER_COMPONENT(c) \
	namespace {\
		const auto __ENGINE_ECS_COMPONENT_DEF__ ## c ## __ = Engine::ECS::detail::registerComponent<c>(#c);\
	}

/** 
 * @brief This namespace contains all functionality associated with the Entity Component System.
 * This excludes the wrapper classes such as Engine::Entity.
 */
namespace Engine::ECS {
	/** The maximum number of components registrable. Idealy this would be exactly the number of components used. */
	constexpr size_t MAX_COMPONENTS = 64;

	/** The type to use for entity ids */
	using EntityID = size_t;

	/** The type to use for component ids */
	using ComponentID = size_t;

	using ComponentBitset = std::bitset<MAX_COMPONENTS>;

	namespace detail {
		template<class T>
		using ComponentContainer = std::vector<T>;
		using AddComponentFunction = void(*)(EntityID, ComponentID);
		using GetComponentFunction = void*(*)(EntityID);

		// TODO: Move these to functions?
		extern std::unordered_map<std::string_view, ComponentID> componentIDMap;
		extern std::array<AddComponentFunction, MAX_COMPONENTS> addComponentFuncitons;
		extern std::array<GetComponentFunction, MAX_COMPONENTS> getComponentFuncitons;

		extern std::vector<ComponentBitset> entityComponentBitsets;
		extern std::vector<EntityID> entityLife;
		extern std::queue<EntityID> reusableEntityIDs;
	
		/**
		 * @brief Gets the next ComponentID.
		 * @return The next ComponentID.
		 */
		ComponentID getNextComponentID();

		/**
		 * @brief Get the bitset associated with an entity.
		 * @param[in] eid The entity id of the entity.
		 * @return A reference to the bitset associated with the entity.
		 */
		ComponentBitset& getComponentBitset(EntityID eid);

		/**
		 * @brief Get the ComponentID associated an component.
		 * @tparam Component The component.
		 * @return The id of @p Component.
		 */
		template<class Component>
		ComponentID getComponentID();

		/**
		 * @brief Get the ComponentID associated with @p name.
		 * @param[in] name The name of the component.
		 * @return The id of the component.
		 */
		ComponentID getComponentID(const std::string_view name);

		/**
		 * @brief Gets the container associated with an component.
		 * @tparam Component The component.
		 * @return A reference to the container associated with the component.
		 */
		template<class Component>
		ComponentContainer<Component>& getComponentContainer();

		/**
		 * @brief Constructs a new @p Component in the appropriate contanier and set the corresponding bit in the entities bitset.
		 * @param[in] eid The id of the entity to add this component to.
		 * @param[in] cid The id of the component to add to the entity. This must be the id associated with @p Component.
		 * @tparam Component The component.
		 * @return A reference to the container associated with the component.
		 */
		template<class Component>
		void addComponentToEntity(EntityID eid, ComponentID cid);

		/**
		 * @brief Gets a pointer the component associated with the entity.
		 * @param[in] eid The id of the entity.
		 * @tparam Component The component.
		 * @return A pointer to the component. This is invalidated if the container associated with the component type is resize.
		 */
		template<class Component>
		void* getComponentForEntity(EntityID eid);

		/**
		 * @brief Registers a component for use in the ECS.
		 * @param[in] name The name to associate with the component.
		 * @tparam Component The component.
		 * @return This is always zero.
		 */
		template<class Component>
		int registerComponent(const std::string_view name);
	}

	/**
	 * @brief Creates a new entity.
	 * @param[in] forceNew When set to true, prevents the reuse of ids.
	 * @return The id of the new entity.
	 */
	EntityID createEntity(bool forceNew = false);

	/**
	 * @brief Destroys an entity.
	 * @param[in] eid The id of the entity.
	 */
	void destroyEntity(EntityID eid);

	/** 
	 * @brief Checks is an entity is alive.
	 * @param[in] eid The id of the entity.
	 * @return True if the entity is alive; otherwise false.
	 */
	bool isAlive(EntityID eid);

	/**
	 * @brief Adds a component to an entity.
	 * @param[in] eid The id of the entity.
	 * @param[in] cid The id of the component.
	 */
	void addComponent(EntityID eid, ComponentID cid);

	/**
	 * @copybrief addComponent
	 * @param[in] eid The id of the entity.
	 * @tparam Component The component.
	 */
	template<class Component>
	void addComponent(EntityID eid);

	/**
	 * @brief Checks if an entity has a component.
	 * @param[in] eid The id of the entity.
	 * @param[in] cid The id of the component.
	 * @return True if the entity has the component; otherwise false.
	 */
	bool hasComponent(EntityID eid, ComponentID cid);

	/**
	 * @copybrief hasComponent
	 * @param[in] eid The id of the entity.
	 * @tparam Component The component.
	 * @return True if the entity has the component; otherwise false.
	 */
	template<class Component>
	bool hasComponent(EntityID eid);

	/**
	 * @brief Checks if an entity has components.
	 * @param[in] eid The id of the entity.
	 * @param[in] cbits The bitset of components.
	 * @return True if the entity has the components; otherwise false.
	 */
	bool hasComponents(EntityID eid, ComponentBitset cbits);

	/**
	 * @brief Removes a component from an entity.
	 * @param[in] eid The id of the entity.
	 * @param[in] cid The id of the component.
	 */
	void removeComponent(EntityID eid, ComponentID cid);

	/**
	 * @copybrief removeComponent
	 * @param[in] eid The id of the entity.
	 * @tparam Component The component.
	 */
	template<class Component>
	void removeComponent(EntityID eid);

	/**
	 * @brief Gets a reference the component associated with an entity.
	 * @param[in] eid The id of the entity.
	 * @tparam Component The component.
	 * @return A reference to the component. This is invalidated if the container associated with the component type is resize.
	 */
	template<class Component>
	Component& getComponent(EntityID eid);
}

#include <Engine/ECS/ECS.ipp>