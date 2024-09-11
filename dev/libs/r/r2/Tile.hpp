#pragma once

#include "1-graphics/Texture.h"
#include "r2/Bounds.hpp"

namespace r2 {

	enum R2TileFlags {
		R2_TILE_MUST_DESTROY_TEX = (1 << 0),
		R2_TILE_IS_POOLED = (1 << 1),
		R2_TILE_TARGET_FLIPED = (1 << 2),
	};

	class Tile {
	protected:
		Pasta::Texture*			tex = nullptr;
	public:
		double					x = 0;//rectangle x upon texture
		double					y = 0;//rectangle y upon texture
		double					width = 0;//rectangle width upon texture
		double					height = 0;//rectangle height upon texture

		double					dx = 0;//dx is the offset to apply for pivots
		double					dy = 0;//dy is the offset to apply for pivots

		int						flags = 0;

		int						uid = 0;//for debug purpose, never clone it

		double					u1 = 0.0;
		double					v1 = 0.0;
		double					u2 = 1.0;
		double					v2 = 1.0;
		const char*				debugName = 0;

	public:
								Tile();
								Tile(Pasta::Texture* tex, int rectX = 0, int rectY = 0, int w = -1, int h = -1, int dx = 0, int dy = 0);
								Tile(const Tile& t);

								~Tile();

		void					destroy();
		void					set(Pasta::Texture* tex, int rectX = 0, int rectY = 0, int w = -1, int h = -1, int dx = 0, int dy = 0);
		void					clear();
		void					disposeTexture();

		bool					isTargetFlip();
		void					translatePos(double _dx, double _dy);
		void					translateCenterDiscrete(double _dx, double _dy);

		void					setPos(double x, double y);
		void					setSize(double w, double h);

		void					setTexture(Pasta::Texture* tex, double u1, double v1, double u2, double v2);

		//discrete pixel coord
		void					setCenterDiscrete(double px = 0, double py = 0);


		void					snapToPixel();
		//homogeneous coord
		void					setCenterRatio(double px = 0.5, double py = 0.5);
		void					setUV(double u1, double v1, double u2, double v2);

		void					copy(const Tile& t);//cloned do not retain flags
		void					swap(Tile& t);//swap does swap flags for dealiasing
		//cloned do not retain flags
		Tile*					clone() const;


		void					flipX();
		void					flipY();

		void					resetTargetFlip();

		void					targetFlipY();
		void					textureFlipY();

		inline Pasta::Texture*	getTexture() { return tex; };

		//just changes the textures backing, see@mapTexture to set texture and uv accordingly
		void					setTexture(Pasta::Texture* _tex);

		void					enableTextureOwnership(bool onOff);

		static Tile*			fromPool(Pasta::Texture* tex, int rectX = 0, int rectY = 0, int w = -1, int h = -1, int dx = 0, int dy = 0);
		static Tile*			fromWhite();
		static Tile*			fromImageFile(const std::string& path, r2::TexFilter filter = r2::TexFilter::TF_NEAREST);

		static Tile*			fromColor(const r::Color & col);
		//does not acquire autority to release texture data
		static Tile*			fromTexture(Pasta::Texture* tex);
		//does not acquire autority to release texture data
		static Tile*			fromTextureData(Pasta::TextureData* texData, r2::TexFilter filter = r2::TexFilter::TF_NEAREST);

		void					toPool();

		std::string				toString() const {
			return std::to_string(x) + "," + std::to_string(y) + " " + std::to_string(width) + "x" + std::to_string(height);
		}

		double					onePixelX() { return 1.0 / tex->getWidth(); };
		double					onePixelY() { return 1.0 / tex->getHeight(); };

		Vector2					getCenterRatio() { return { (float)getCenterRatioX(), (float)getCenterRatioY() }; }
		double					getCenterRatioX() { return (double)-dx / width; };
		double					getCenterRatioY() { return (double)-dy / height; };
		inline bool				isPooled() { return flags & R2TileFlags::R2_TILE_IS_POOLED; };
		inline bool				mustDestroyTex() { return flags & R2TileFlags::R2_TILE_MUST_DESTROY_TEX; };

		void					mapTexture(Pasta::Texture* _tex);
		r2::Bounds				getBounds();

		void					smallPreview();
		bool					im(bool preview = true, bool pick = true);

		r::u64					getHash();

		r2::Tile*				subRectTLDR(const Vector2i & tl, const Vector2i &dr);
		r2::Tile*				subRectPosSize(const Vector2i & tl, const Vector2i &size);
	};


}