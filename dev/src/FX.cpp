

#include "stdafx.h"
#include "FX.hpp"



class BlinkAgent : public rd::Agent{
public:
	r2::Sprite* sp = 0;
	double timer = 0.5f;

	BlinkAgent(r2::Sprite* _sp, double _timer): rd::Agent(nullptr),timer(_timer),sp(_sp){

	}

	virtual void update(double dt) {
		int tm = rs::Timer::frameCount + sp->uid;
		tm = r::Math::umod(tm, 8);
		sp->alpha = (tm < 4);
		timer -= dt;
		if(timer<=0){
			dispose();
			sp->alpha = 1;
		}
	}
};
void FX::blink(r2::Sprite* sp) {
	auto sc = sp->getScene();
	sc->al.add(new BlinkAgent( sp, 0.2f ));
}
