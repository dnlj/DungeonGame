#pragma once

// FreeType
#include <ft2build.h>
#include <freetype/freetype.h>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Gui/FontGlyphSet.hpp>


namespace Engine::Gui { // TODO: move
	struct FontId {
		FT_Face font;
		int32 size;

		ENGINE_INLINE bool operator==(const FontId& other) const noexcept {
			return font == other.font && size == other.size;
		}
	};
}

template<> 
struct Engine::Hash<Engine::Gui::FontId> {
	size_t operator()(const Engine::Gui::FontId& val) const {
		static_assert(sizeof(size_t) == sizeof(val.font));
		size_t seed = reinterpret_cast<const size_t&>(val.font);
		hashCombine(seed, val.size);
		return seed;
	}
};

namespace Engine::Gui {
	class FontManager {
		public: // TODO: private
			FT_Library ftlib;

		private:
			FlatHashMap<std::string, FT_Face> pathToFace;
			FlatHashMap<FontId, std::unique_ptr<FontGlyphSet>> fontIdToGlyphSet;


		public:
			FontManager();
			~FontManager();

			FontId createFont(const std::string& path, int32 size);

			// TODO: temp, want to use find so we dont create invalid sets
			auto& getFontGlyphSet(FontId fid) { return fontIdToGlyphSet[fid]; }
	};
}

