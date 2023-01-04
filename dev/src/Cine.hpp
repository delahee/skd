#pragma once

#include "EASTL/vector.h"
#include "rd/Agent.hpp"

class CineController;
class CineStep {
protected:
	bool				finished		= false;
	bool				initialised	= false;
	CineController*		owner = 0;
public:

			CineStep() {};
	virtual ~CineStep() {};

	virtual void update(double dt) {
		finished = true;
	};

	rd::Sig onInitialised;
	rd::Sig onDisposed;

	void init(CineController*_owner) {
		if (initialised)
			return;
		owner = _owner;
		onInitialised();
		initialised = true;
	}

	//step is finished
	virtual void dispose() {
		onDisposed();
	}

	virtual bool isFinished() {
		return finished;
	};
};

class AnonCineStep : public CineStep {
public:
	std::function<void(void)> proc;
				AnonCineStep(std::function<void(void)> f) : proc(f)	{  

				};
	virtual		~AnonCineStep() {};

	virtual void update(double dt) {
		proc();
		finished = true;
	};
};

class CineController : public rd::Agent {
public:
	eastl::vector<CineStep*>	steps;
	int							cursor = -1;
	rd::Sig						onStart;
	rd::Sig						onEnd;
	
	rd::MsgChanStr				msg;

	CineController( rd::AgentList* al) : rd::Agent(al) {
		name = "CineController";
	};

	void reset() {
		for (auto c : steps)
			delete c;
		steps.clear();
		cursor = -1;
		onStart.clear();
		onEnd.clear();
	};

public:
	CineStep* last() { return steps.empty() ? 0 : steps[steps.size() - 1]; };
	void add(std::function<void(void)> f) { steps.push_back( new AnonCineStep(f)); };
	void add(CineStep*s) { steps.push_back(s); };
	void start();
	virtual void update(double dt);
	virtual ~CineController(){
		dispose();
	}
};

class WaitForSeconds : public CineStep {
public:
	double timer = 0.0f;
	WaitForSeconds(double t) : timer(t) {
		
	};
	virtual void update(double dt) override {
		timer -= dt;
		finished = timer <= 0;
	};
};

class WaitForKeyInput : public CineStep {
public:
	Pasta::Key k;
	WaitForKeyInput(Pasta::Key _k) : k(_k) {

	};
	virtual void update(double dt) override {
		if (!finished)
			finished = rs::Input::isKeyJustPressed(Pasta::ControllerType::CT_KEYBOARD,k);
	};
};

class WaitForMessage : public CineStep {
public:
	Str key;
	rd::MsgChanStr::MsgChanHandler* h=0;
	WaitForMessage(const char * msg){
		key = msg;
		onInitialised.add([=]() {
			h = owner->msg.add([=]( const Str & _msg) {
				if (_msg == key) {
					finished = true;
					if (h) {
						owner->msg.remove(h);
						//do not do anything after this as the handler is deleted
					}
				}
			});
		});
	};
	virtual void update(double dt) override {
		
	};
};

#define CINE_STEP_BEGIN(cc)		cc->add([]() {
#define CINE_STEP_END()			});