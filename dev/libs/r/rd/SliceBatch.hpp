#pragma once

#include "r2/Batch.hpp"

namespace rd {

	struct Slice3Param {
		r2::Tile *	tile = nullptr;

		int			targetWidth = 0;

		int			marginLeft	= 0;
		int			marginRight	= 0;

		bool		ownTile = false;

		Slice3Param();
		Slice3Param(const r2::Tile* t, int tw, int ml, int mr) {
			tile = t->clone();
			ownTile = true;
			targetWidth = tw;
			marginLeft = ml;
			marginRight = mr;
		};

		Slice3Param(const Slice3Param& sp);
		Slice3Param& operator=(const Slice3Param& sp) {
			setTile(sp.tile);

			targetWidth = sp.targetWidth;
			marginLeft = sp.marginLeft;
			marginRight = sp.marginRight;
			return *this;
		}

		Slice3Param(const r2::Tile*t) {
			setTile(t);
		}

		void setTile(const r2::Tile* t);

		~Slice3Param() {
			if(tile&& ownTile) tile->destroy();
			tile = 0;
			ownTile = false;
		}
	};

	class Slice3 : public r2::Batch {
	protected:
		Slice3Param		param;

		r2::Tile*		tileLeft = nullptr;
		r2::Tile*		tileCenter = nullptr;
		r2::Tile*		tileRight = nullptr;
		r2::BatchElem * bLeft		= nullptr;
		r2::BatchElem * bCenter		= nullptr;
		r2::BatchElem * bRight		= nullptr;
	public:
						Slice3(r2::Node * parent, const Slice3Param & param);

		void			set(const Slice3Param& param);
		virtual			~Slice3();

		void			setTargetSize(int width);
		void			updateMargins(int marginLeft = -1, int marginRight = -1);
		virtual void	im() override;
	};

	struct Slice9Param {
		r2::Tile * tile = nullptr;;

		int targetWidth = 0;
		int targetHeight = 0;

		int marginLeft = 0;
		int marginRight = 0;
		int marginTop = 0;
		int marginBottom = 0;

		bool ownTile = false;

		Slice9Param(const r2::Tile* t, int tw, int th, int ml, int mr, int mt, int mb) {
			tile = t->clone();
			ownTile = true;
			targetWidth = tw;
			targetHeight = th;

			marginLeft = ml;
			marginRight = mr;
			marginTop = mt;
			marginBottom = mb;
		};

		Slice9Param();

		Slice9Param(const Slice9Param& sp) {
			tile = sp.tile->clone();
			ownTile = true;
		}

		Slice9Param& operator=(const Slice9Param& sp) {
			setTile(sp.tile);

			targetWidth = sp.targetWidth;
			targetHeight = sp.targetHeight;
			marginLeft = sp.marginLeft;
			marginRight = sp.marginRight;
			marginTop = sp.marginTop;
			marginBottom = sp.marginBottom;
			return *this;
		};

		Slice9Param(const r2::Tile* t) {
			setTile(t);
		}

		void setTile(const r2::Tile* t) {
			if (tile) {
				if (ownTile)
					rd::Pools::tiles.free(tile);
				tile = 0;
				ownTile = 0;
			}
			tile = t->clone();
			ownTile = true;
		}

		~Slice9Param() {
			if (ownTile)
				rd::Pools::tiles.free(tile);
			tile = 0;
			ownTile = false;
		}
	};

	class Slice9 : public r2::Batch {
	protected:
		Slice9Param		param;

		r2::Tile * tileTopLeft = nullptr;
		r2::Tile * tileTopCenter = nullptr;
		r2::Tile * tileTopRight = nullptr;

		r2::Tile * tileLeft = nullptr;
		r2::Tile * tileCenter = nullptr;
		r2::Tile * tileRight = nullptr;
		
		r2::Tile * tileBotLeft = nullptr;
		r2::Tile * tileBotCenter = nullptr;
		r2::Tile * tileBotRight = nullptr;

		r2::BatchElem * bTopLeft = nullptr;
		r2::BatchElem * bTopCenter = nullptr;
		r2::BatchElem * bTopRight = nullptr;

		r2::BatchElem * bLeft = nullptr;
		r2::BatchElem * bCenter = nullptr;
		r2::BatchElem * bRight = nullptr;

		r2::BatchElem * bBotLeft = nullptr;
		r2::BatchElem * bBotCenter = nullptr;
		r2::BatchElem * bBotRight = nullptr;
	public:
		Slice9(r2::Node * parent, const Slice9Param & _param);
		void set(const Slice9Param& _param);
		virtual ~Slice9();

		void setTargetSize(int width, int height);
		void updateMargins(int marginLeft = -1, int marginRight = -1, int marginTop = -1, int marginBottom = -1);
		virtual void im() override;
	};
}