#pragma once

#include "Sprite.hpp"
#include "Tile.hpp"

namespace rd {
	class TileLib;
}

namespace r2 {
	class Bitmap : public Sprite {
		typedef Sprite Super;
	public:
		Tile *			tile		= nullptr;
		bool			ownsTile	= true;

	public:
							Bitmap( Tile * t, Node * parent = nullptr);
							Bitmap( Pasta::Texture * t, Node * parent = nullptr);
							Bitmap( Node * parent = nullptr);
		virtual				~Bitmap();

		void				releaseTile();

		virtual void		dispose();
		virtual void		reset() override;

		virtual Node *		clone(Node * n)override;
		static Bitmap *		fromImageFile(const char* path, r2::Node * parent = 0, r2::TexFilter filter = r2::TexFilter::TF_INHERIT);
		static Bitmap *		fromTexture(Pasta::Texture * tex, r2::Node * parent = nullptr);
		static Bitmap *		fromTile( r2::Tile * t, r2::Node * parent = 0, bool own = false);
		static Bitmap *		fromColor( const r::Color & c, r2::Node * parent = 0);
		static Bitmap *		fromColor32( r::u32 col, r2::Node * parent = 0);
		static Bitmap *		fromColor24( r::u32 col, r2::Node * parent = 0);

		//tile is stolen, not copied in its current state
		static Bitmap *		fromBatchElem( r2::BatchElem * e);

		static Bitmap *		fromLib( rd::TileLib * t, const char * name, r2::Node * parent = 0);
		static Bitmap *		fromLib( rd::TileLib * t, const std::string & name, r2::Node * parent = 0);
		static Bitmap*		fromLib(rd::TileLib* t, const Str& name, r2::Node* parent = 0) { return fromLib(t, name.c_str(), parent); };

		virtual Tile *		getPrimaryTile();

		virtual void		setWidth(float width) override;
		virtual void		setHeight(float height) override;

		virtual Bounds		getMyLocalBounds() override;

		virtual r::Vector2	getCenterRatio();
		virtual void		setCenterRatio( double px = 0.5, double py = 0.5 );
		void				setCenterRatioPixel( int x,int y );

		void				setTile(r2::Tile* t, bool own = false);
		void				copyTile(const r2::Tile * t);

		//todo remove this
		void				loadTileByRef(const r2::Tile & t);
		void				loadTileByPtr(const r2::Tile * t);

		std::string			toString();
		virtual void		im() override;
		
		virtual void		serialize(Pasta::JReflect & jr, const char * _name = nullptr) override;

		virtual	NodeType	getType() const override { return NodeType::NT_BMP; };
	};
}
