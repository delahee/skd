#pragma once

#include "TileLib.hpp"

namespace rd {
	class TileAnimPlayer;
	class TileAnim {
	public:
		bool									paused		= false;
		int										cursor		= 0;
		int										plays		= 0;
		double									delayMs		= 0;
		double									speed		= 1.0;
		double									startDate	= 0;

		Str										libName;
		Str										groupName;
		TileGroup*								groupData	= nullptr;
		
		double									curFrameCount = 0;
        std::function<bool(TileAnimPlayer*)>	onEnd = nullptr; //if returns true, cease everything for deletion is happening
        std::function<bool(TileAnimPlayer*)>	onLoop = nullptr;

		TileAnim();
		TileAnim(const TileAnim&ta);
		~TileAnim();

		inline int nbFrames() const {
			if (!groupData) return 0;
			return groupData->anim.size();
		}

		inline int getFrame(int idx) const {
			return groupData->anim[idx];
		}

		void clear();
		void reset(	const char * _group,
					TileGroup * _data,
					int _animCursor,
					int _curFrameCpt,
					int _plays = 1,
					bool _paused = false,
					float _speed = 1.0 );

		void copy(TileAnim&toCopy);
		void im();
	};
}