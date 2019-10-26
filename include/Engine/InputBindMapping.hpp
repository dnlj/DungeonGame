#pragma once

// STD
#include <array>

// Engine
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input.hpp>
#include <Engine/InputSequence.hpp>
#include <Engine/InputState.hpp>
#include <Engine/BindId.hpp>


// TODO: Doc
namespace Engine {
	class InputBindMapping {
		public:
			// TODO: swap bid to be first? makes more sense and look nicer in code.
			InputBindMapping(InputSequence inputs, BindId bid);
			void processInput(const InputState& is);
			bool isActive() const;
			BindId getBindId() const;

		private:
			bool active = false;
			const BindId bid;
			// TODO: Do we really want to use InputState here? While it works, it seems semantically incorrect.
			InputStateSequence inputStates;
	};
};
