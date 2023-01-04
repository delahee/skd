#pragma once

#include "r/Color.hpp"

namespace r2 { class Tile; }
namespace r2 {
	namespace ext {
		class Texels {
		public:
			static inline Color fromTexData(const r::u8* texel, int texelByteSize) {
				r::Color c(1, 1, 1, 1);
				switch (texelByteSize) {
				case 3:
					c.r = *texel / 255.0f;
					c.g = *(texel + 1) / 255.0f;
					c.b = *(texel + 2) / 255.0f;
					break;
				case 4:
					c.r = *texel / 255.0f;
					c.g = *(texel + 1) / 255.0f;
					c.b = *(texel + 2) / 255.0f;
					c.a = *(texel + 3) / 255.0f;
					break;
				default:
					break;
				}
				return c;
			};

			static r::Color		getTexel(Pasta::TextureData* td, const r2::Tile* t, int x, int y);

			static void			setTexel(Pasta::TextureData* td, int x, int y, r::u8 val);
			static void			setTexel(Pasta::TextureData* td, int x, int y, r::Color val);

			static r::u8		getTexelU8(Pasta::TextureData* td, int x, int y);
			static r::u32		getTexelU32(Pasta::TextureData* td, const r2::Tile* t, int x, int y);

			static void			setTexelU8(Pasta::TextureData* td, int x, int y, r::u8 val);

			static r::Color		random(Pasta::TextureData* td, const r2::Tile* t);
			static r::Vector2	randomPos(Pasta::TextureData* td, const r2::Tile* t);
			static r::Vector2	randomPos(Pasta::TextureData* td);
		};
	}
}