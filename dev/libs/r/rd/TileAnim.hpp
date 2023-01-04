#pragma once

#include <string>
#include <vector>
#include <functional>


#include "TileLib.hpp"
#include "TileAnimPlayer.hpp"

namespace rd {
	class TileAnim {
	public:
		int										cursor		= 0;
		int										plays		= 0;
		double									speed		= 1.0;
		bool									paused		= false;

		Str										libName;
		Str										groupName;
		TileGroup*								groupData	= nullptr;
		
		double									curFrameCount = 0;
		std::function<bool(TileAnimPlayer*)>	onEnd		= nullptr; //if returns true, cease everything for deletion is happening

		TileAnim();
		TileAnim(const TileAnim&ta);
		~TileAnim();

		inline int nbFrames() {
			if (!groupData) return 0;
			return groupData->anim.size();
		}

		inline int getFrame(int idx) {
			return groupData->anim[idx];
		}

		void clear();
		void reset(	const char * _group,
					TileGroup * _data,
					int _animCursor,
					int _curFrameCpt,
					int _plays =1,
					bool _paused=false,
					float _speed = 1.0 );

		void copy(TileAnim&toCopy);
	};
}