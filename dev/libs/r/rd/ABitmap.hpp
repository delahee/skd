#pragma once

#include <cstddef>

#include "1-graphics/Graphic.h"

#include "../r2/Sprite.hpp"
#include "../r2/Tile.hpp"
#include "../r2/Bitmap.hpp"
#include "TileLib.hpp"
#include "TileAnim.hpp"
#include "TileAnimPlayer.hpp"

namespace rd {
	//Animated Bitmap
	class ABitmap : public r2::Bitmap, public rd::IAnimated {
	public:
		bool			destroyed = false;
		
		bool			usePivot = false;

		float			pivotX = 0.0f;
		float			pivotY = 0.0f;
		
		int				frame = 0;

		FrameData*		frameData	= nullptr;

		std::string		groupName;
		TileAnimPlayer  player;
		
	public:
						ABitmap( r2::Node * _parent = nullptr );
						ABitmap( const char * _groupName, TileLib * _lib = nullptr,  r2::Node * _parent = nullptr );
		
		virtual			~ABitmap();
		
		Node *			clone(Node * n);
		virtual void	dispose() override;

		void			play(const char* g, bool loop = false);
		void			playAndLoop(const std::string & str) { playAndLoop(str.c_str()); };
		void			playAndLoop(const char * g);
		void			replay(int nb = 1);
		void			stop();

		r2::Tile*		syncTile();

		void			set(TileLib * l, const std::string & str, int frame = 0, bool stopAllAnims = false);
		void			set(rd::TileLib * lib = nullptr, const char * g = nullptr, int frame = 0, bool stopAllAnims= false);
		
		bool			isReady();
		virtual void	update(double dt);
				
		virtual void	setWidth(float _w);
		virtual void	setHeight(float _h);

		void			setFrame(int frame);
		int				getFrame() { return frame;  };
		TileLib *		getLib() { return lib; }


		virtual r::Vector2	getCenterRatio();
		virtual void	setCenterRatio(double px = 0.5, double py = 0.5) override;
		virtual void	setCenterRatioPx(int px, int py);
		virtual void	im()override;

		virtual void	serialize(Pasta::JReflect & jr, const char * _name = nullptr) override;

		static	rd::ABitmap* make(TileLib* l = nullptr, const char* str = nullptr, r2::Node* parent = nullptr);
		static	rd::ABitmap* fromLib(TileLib* l = nullptr, const char* str = nullptr, r2::Node* parent = nullptr);
		static	rd::ABitmap* fromPool(TileLib* l = nullptr, const char* str = nullptr, r2::Node* parent = nullptr);

		virtual	NodeType		getType() const override { return NodeType::NT_ABMP; };

		void			setLib(TileLib* lib);
		bool			isFinished();

		static rd::ABitmap* mk(const char* _groupName, TileLib* _lib = nullptr, r2::Node* _parent = nullptr);

		//there are may ways to express bounds
		//here we use te "graphical definition"
		//if you want the logical definition, you may want to retrieve the frame data
		virtual r2::Bounds	getMyLocalBounds();

		void			setFlippedX(bool onOff);
		void			setFlippedY(bool onOff);
	protected:
		bool			flippedX = false;
		bool			flippedY = false;
		TileGroup*		group = nullptr;
		TileLib*		lib = nullptr;
	};
}
