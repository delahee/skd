#pragma once

#include <string>
#include <functional>
#include "EASTL/vector.h"
#include "TileAnim.hpp"
#include "IAnimated.hpp"
#include "Sig.hpp"
#include "Tween.hpp"

namespace rd {
	class TileAnim;
	class TileAnimPlayer;

	struct TileAnimPlayerExtension {
		TileAnimPlayerExtension(TileAnimPlayer* _owner) {
			owner = _owner;
		}
		TileAnimPlayer* owner = nullptr;
		rd::Sig			onAnimFrame;
		rd::Sig			onAnimEnd;

		void			reset();
	};

	class AnimBus {
	public:
		Str					name;
		double				maxSpeed = 16.0;
		double				minSpeed = 0.000000001f;
		double				speed = 1.0;
		AnimBus*			parent = nullptr;

		double				getFinalSpeed() { update(); return finalSpeed; };
		void				update();
		void				im();
	protected:
		double				finalSpeed = 1.0;
	};

	class TileAnimPlayer {
	public:
		bool							debug = false;
		bool							needUpdates = true;
		bool							destroyed = false;
		bool							isPlaying = false;

		AnimBus*						bus = nullptr;
		IAnimated*						spr = nullptr;
		TileAnimPlayerExtension*		ext = nullptr;

		double							frameRate = 24.0;//default is taken from lib
		double							speed = 1.0;
		std::function<void(int)>		onFrame;//on cursor frame hit, beware to translate it to effective frame

		eastl::vector<rd::TileAnim*>	stack;
		eastl::vector<rd::TileAnim>		history;
		//Str							lastPlayed;
		TileAnimPlayerExtension*		extend();
	protected:
		//lib is self managed
		TileLib*						lib = nullptr;

	public:
										TileAnimPlayer(IAnimated*_spr = nullptr);
										TileAnimPlayer(const TileAnimPlayer& pl) = delete; //use operator= 

		TileAnimPlayer&					operator=(const TileAnimPlayer& ot);

		virtual							 ~TileAnimPlayer();

		TileAnim *						getCurrentAnim();
		bool							isFinished();
		void							update(double dt);

		void							stopUpdates();
		void							startUpdates();

		void							resume();
		void							pause();
		void							stop();
		bool							hasAnim();
		void							nextAnim();

		/**
		returns true if animation flow is likely to interrupt
		if false is returned, you should schedule a next animation with a chain OR ask a delay play next
		or ask for forgiveness when it explodes
		*/
		void							setOnEnd(std::function<bool(rd::TileAnimPlayer*)> fun);

		//a bit experimental ish, favour to add extension via extend() for safety
        void							addOnEnd(std::function<bool(rd::TileAnimPlayer*)> fun);

        void							setOnLoop(std::function<bool(rd::TileAnimPlayer*)> fun);

		TileAnim*						getLastAnim();
		TileAnimPlayer*					play(const char * group, int nbPlays = 1, bool queueAnim = false);

		TileAnimPlayer *				playAndLoop(const char * k);
		TileAnimPlayer *				playAndLoop(const std::string& k) { return playAndLoop(k.c_str()); };
		TileAnimPlayer *				playAndLoop(const Str& k) { return playAndLoop(k.c_str()); };

		TileAnimPlayer *				chainAndLoop(const char * k);
		TileAnimPlayer*					chainAndLoop(const std::string& k) { return chainAndLoop(k.c_str()); };
		TileAnimPlayer*					chainAndLoop(const Str& k) { return chainAndLoop(k.c_str()); };

		TileAnimPlayer *				loop();
		TileAnimPlayer *				chain(const char * id);

		//retrieves the [0...1...n-1] index depending on the graphic designer's mood and the temperature in syberia during asset creation...
		int								getFrameNumber();
		int								getCursor();


		void							setFrame(int frame);
		void							setCursor(int cursor);
		
		double							getFinalAnimSpeed();
		float							getDuration();

		bool							isReady();

		void							unsync();

		//pool back anims
		void							dispose();

		//pool back anims and restore freshness
		void							reset();

		void							copy(TileAnimPlayer&toCopy);
		void							syncCursorToSpr();
		void							dumpState();
		void							im();
	protected:
		void							initCurrentAnim();
		void							_update(double dt);
	};
}