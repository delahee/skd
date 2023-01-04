#pragma once  

namespace r2{
	void					makeRGBOffset(r2::Sprite * spr, Pasta::Vector2 r, Pasta::Vector2 g, Pasta::Vector2 b);
	void					makeBlurX(r2::Sprite * spr, float dx = 1.0f, float offsetScale = 1.0f);
	rd::Anon*				getRGBOffsetParam(r2::Sprite * sprs);

}