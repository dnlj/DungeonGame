#pragma once

// STD
#include <string>
#include <vector>

// Engine
#include <Engine/BindPressListener.hpp>
#include <Engine/BindReleaseListener.hpp>


// TODO: Doc
namespace Engine {
	class Bind {
		public:
			Bind(std::string name);

			void press();

			void release();

			// TODO handle hold listeners? or should we have the listeners implement that themselves?

			void addPressListener(BindPressListener* listener);

			void addReleaseListener(BindReleaseListener* listener);

			const std::string name;

		private:
			int active = 0;
			std::vector<BindPressListener*> pressListeners;
			std::vector<BindReleaseListener*> releaseListeners;
	};
}
