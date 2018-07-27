#pragma once

// STD
#include <vector>

// Engine
#include <Engine/ECS/Common.hpp>


// TODO: Doc
namespace Engine::ECS {
	class EntityFilter {
		private:
			std::vector<Entity> entities;

		public:
			using ConstIterator = decltype(entities)::const_iterator;

			// TODO: Make private. Move to constructor
			ComponentBitset componentsBits;

			void add(Entity ent, const ComponentBitset& cbits);
			void remove(Entity ent);

			bool empty() const;

			std::size_t size() const;

			ConstIterator begin();
			ConstIterator end();

			ConstIterator begin() const;
			ConstIterator end() const;

			ConstIterator cbegin() const;
			ConstIterator cend() const;
	};
}