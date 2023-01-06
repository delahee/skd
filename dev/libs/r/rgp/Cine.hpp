#pragma once

#include "EASTL/vector.h"
#include "rd/Agent.hpp"

namespace rgp {
	class CineController;
	class CineStep {
	public:
		bool				finished = false;
		rd::Vars			vars;
		CineController*		owner = 0;
		std::string			name;
	protected:
		bool				initialised = false;
	public:

		CineStep();
		virtual ~CineStep() {};

		virtual void update(double dt) {
			finished = true;
		};

		rd::Sig onInitialised;
		rd::Sig onDisposed;

		void init(CineController* _owner) {
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
		bool once = false;
		bool autoFinish = true;
		std::function<void(AnonCineStep*)> proc;
		explicit AnonCineStep(std::function<void(void)> f);
		explicit AnonCineStep(std::function<void(AnonCineStep*)> f);

		virtual		~AnonCineStep() {
			proc = 0;
		};

		virtual void update(double dt);
	};

	class CineController : public rd::Agent {
	public:
		eastl::vector<CineStep*>	steps;
		int							cursor = -1;

		rd::Sig						onStart;
		rd::Sig						onEnd;

		double						currentStepDuration = 0;
		rd::Vars					vars;
		rd::MsgChanStr				msg;

		double						speed = 1.0;

		CineController(const char * _name , rd::AgentList* al) : rd::Agent(al) {
			name = _name;
		};

		void reset() {
			for (auto c : steps)
				delete c;
			steps.clear();
			cursor = -1;
			onStart.clear();
			onEnd.clear();
		};

		virtual ~CineController() {
			dispose();
		};

		void im();

	public:
		CineStep*		last() { return steps.empty() ? 0 : steps[steps.size() - 1]; };
		
		void			add(CineStep* s) { steps.push_back(s); };
		void			addInstant(CineStep* s);
		void			add(std::function<void(void)> f);
		void			add(std::function<void(AnonCineStep*)> f);

		//returns false if no steps are present in the cinematic sequence
		bool			start();
		void			pause();
		virtual void	update(double dt);

		CineStep*		instantWaitForSeconds(double tSec);
		CineStep*		waitForSeconds(double tSec);
		CineStep*		waitForMs(double tMSec);

	};

	class WaitForSeconds : public CineStep {
	public:
		double timer = 0.0f;
		WaitForSeconds(double t) : timer(t) {
			name = strrchr(__FUNCTION__, ':')+1;
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
			name = strrchr(__FUNCTION__, ':')+1;
		};
		virtual void update(double dt) override {
			if (!finished)
				finished = rs::Input::isKeyJustPressed(Pasta::ControllerType::CT_KEYBOARD, k);
		};
	};

	class WaitForMessage : public CineStep {
	public:
		Str key;
		rd::MsgChanStr::MsgChanHandler* h = 0;
		WaitForMessage(const char* msg) {
			key = msg;
			onInitialised.add([=]() {
				h = owner->msg.add([=](const Str& _msg) {
					if (_msg == key) {
						finished = true;
						if (h) {
							owner->msg.remove(h);
							//do not do anything after this as the handler is deleted
						}
					}
					});
				});
			name = strrchr(__FUNCTION__, ':')+1;
		};
		virtual void update(double dt) override {

		};
	};

}

