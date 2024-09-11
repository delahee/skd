#pragma once

#include "r2/Batch.hpp"
#include "TileLib.hpp"

namespace rd {

	struct Slice3Param {
		rd::TilePackage		tile;

		int					targetWidth = 4;
		int					marginLeft = 0;
		int					marginRight = 0;

		Slice3Param();
		Slice3Param(const r2::Tile* t, int tw, int ml, int mr);
		Slice3Param(const Slice3Param& sp);
		Slice3Param& operator=(const Slice3Param& sp);
		Slice3Param(const r2::Tile* t);
		~Slice3Param();

		Slice3Param clone() const{
			Slice3Param sp;
			sp.tile = tile.clone();
			sp.targetWidth = targetWidth;
			sp.marginLeft = marginLeft;
			sp.marginRight = marginRight;
			return sp;
		};

		void setTile(const r2::Tile* t);
	};

	class Slice3 : public r2::Batch {
	protected:
		Slice3Param		param;

		r2::Tile*		tileLeft = 0;
		r2::Tile*		tileCenter = 0;
		r2::Tile*		tileRight = 0;

		r2::BatchElem * bLeft		= nullptr;
		r2::BatchElem * bCenter		= nullptr;
		r2::BatchElem * bRight		= nullptr;
	public:
								Slice3(r2::Node * parent, const Slice3Param & param);
		virtual					~Slice3();

		void					set(const Slice3Param& param);

		void					setTargetSize(int width);
		void					updateMargins(int marginLeft = -1, int marginRight = -1);
		virtual void			im() override;
		virtual	NodeType		getType() const { return NodeType::NT_SLICE3; };
		virtual void			serialize(Pasta::JReflect& f, const char* name = nullptr);

	};

	struct Slice9Param {
		rd::TilePackage tile;

		int targetWidth = 4;
		int targetHeight = 4;

		int marginLeft = 0;
		int marginRight = 0;
		int marginTop = 0;
		int marginBottom = 0;

		Slice9Param(const r2::Tile* t, int tw, int th, int ml, int mr, int mt, int mb) {
			tile.setAnon( t->clone() );
			tile.tile->setCenterRatio(0, 0);
			targetWidth = tw;
			targetHeight = th;

			marginLeft = ml;
			marginRight = mr;
			marginTop = mt;
			marginBottom = mb;
		};

		Slice9Param();

		Slice9Param(const Slice9Param& sp) {
			tile = sp.tile;
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

		void setTile(const rd::TilePackage&tp) {
			tile = tp;
		}

		void setTile(const r2::Tile* t) {
			tile.dispose();
			if (t != nullptr) {
				tile.setAnon(t->clone());
				tile.tile->setCenterRatio(0, 0);
			}
		};

		~Slice9Param() {
			tile.dispose();
		};
	};

	class Slice9 : public r2::Batch {
	protected:
		Slice9Param						param;
		rd::TilePackage					tile;

		r2::Tile*		tileTopLeft		= 0;
		r2::Tile*		tileTopCenter	= 0;
		r2::Tile*		tileTopRight	= 0;
		r2::Tile*		tileLeft		= 0;
		r2::Tile*		tileCenter		= 0;
		r2::Tile*		tileRight		= 0;
		r2::Tile*		tileBotLeft		= 0;
		r2::Tile*		tileBotCenter	= 0;
		r2::Tile*		tileBotRight	= 0;

		r2::BatchElem * bTopLeft		= 0;
		r2::BatchElem * bTopCenter		= 0;
		r2::BatchElem * bTopRight		= 0;
										  
		r2::BatchElem * bLeft			= 0;
		r2::BatchElem * bCenter			= 0;
		r2::BatchElem * bRight			= 0;
										  
		r2::BatchElem * bBotLeft		= 0;
		r2::BatchElem * bBotCenter		= 0;
		r2::BatchElem * bBotRight		= 0;
	public:
								Slice9(r2::Node * parent, const Slice9Param & _param);
		virtual					~Slice9();

		void					set(const Slice9Param& _param);

		void					setTargetSize(int width, int height);
		void					updateMargins(int marginLeft = -1, int marginRight = -1, int marginTop = -1, int marginBottom = -1);
		virtual void			im() override;
		virtual	NodeType		getType() const { return NodeType::NT_SLICE9; };
		virtual void			serialize(Pasta::JReflect& f, const char* name = nullptr);


	};
}