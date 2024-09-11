#pragma once

#include <cstddef>

#include "1-graphics/Graphic.h"

#include "../r2/Tile.hpp"
#include "../r2/Batch.hpp"
#include "../r2/BatchElem.hpp"
#include "TileLib.hpp"
#include "TileAnim.hpp"
#include "TileAnimPlayer.hpp"

namespace rd {
	class ABatchElem : public r2::BatchElem, public rd::IAnimated {
		typedef r2::BatchElem Super;
	public:
		bool					usePivot = false;

		float					pivotX = 0.0f;
		float					pivotY = 0.0f;

		int						frame = 0;

		FrameData*				frameData = nullptr;

		Str						groupName;
		TileAnimPlayer			player;
		
	public:
								ABatchElem( const char * _groupName=nullptr, TileLib * _lib = nullptr, r2::Batch * _master = nullptr);
								ABatchElem(const ABatchElem & e);
		virtual					~ABatchElem();
		
		virtual void			reset();
		virtual	r2::BatchElem*	clone(r2::BatchElem* nu = nullptr) const;
		virtual void			dispose() override;

		TileAnimPlayer*			play(const char* g, bool loop = false);
		TileAnimPlayer*			play(const std::string & g, bool loop = false);
		TileAnimPlayer*			play(const Str & g, bool loop = false);
		TileAnimPlayer*			play(const char* g, int nbPlay, bool loop);
		TileAnimPlayer*			playAndLoop(const char * g);
		TileAnimPlayer*			playAndLoop(const std::string & str)	{ return playAndLoop(str.c_str()); };
		TileAnimPlayer*			playAndLoop(const Str & str)			{ return playAndLoop(str.c_str()); };

        void					replay(int nb = 1);
        void					replayAndLoop();
		void					stop();
		
		r2::Tile*				syncTile();

		void					set(rd::TileLib * lib,		const std::string & g, int frame = 0, bool stopAllAnims = false);
		void					set(rd::TileLib * lib = 0,	const char * g = 0, int frame = 0, bool stopAllAnims= false);
		
		bool					isReady();

		/**
		beware, must be updated by hand
		*/
		virtual void			update(double dt) override;
				
		virtual void			setWidth(float _w);
		virtual void			setHeight(float _h);

		void					setFrame(int frame);
		int						getFrame() { return frame;  };
		TileLib *				getLib() { return lib; }

		Vector2					getCenterRatio();

		virtual void			setCenterRatio(double px = 0.5, double py = 0.5) override;
		void					setSpeed( float val );
		void					mulSpeed( float val );

		static ABatchElem *		fromPool(const char * _groupName = nullptr, TileLib * _lib = nullptr, r2::Batch * _master = nullptr);
		virtual void			toPool();
		
		void					setLib(TileLib* lib);
		virtual void			im()override;
		static void				updatePool(double dt);

		virtual BeType			getType() const;
		virtual void			serialize(Pasta::JReflect& jr, const char* _name = nullptr);

		void					setFlippedX(bool onOff);
		void					setFlippedY(bool onOff);
		bool					isFlippedX();
		bool					isFlippedY();
		ABatchElem*				andDestroy();
	protected:
		bool					flippedX = false;
		bool					flippedY = false;
		TileGroup*				group = nullptr;
		TileLib*				lib = nullptr;
	};
}

