#pragma once

namespace rd {
	struct SignalHandler {
		Str							name;
		std::function<void(void)>	function;

		SignalHandler(std::function<void(void)>& f);
	};

	struct Sig {
		Str										name;
		std::any								anyData = nullptr;
		void* ptrData = nullptr;
		std::vector<rd::SignalHandler*>			signals;
		std::vector<rd::SignalHandler*>			signalsOnce;
		bool									isTriggering = false;
		int										triggerredCount = 0;

		Sig();
		~Sig();

		rd::SignalHandler*	add(std::function<void(void)> f);
		rd::SignalHandler*	addOnce(std::function<void(void)> f);

		void				remove(rd::SignalHandler*);
		void				trigger();

		inline int			size() {
			return signals.size() + signalsOnce.size();
		};

		//calls trigger
		void operator()();

		inline void operator+=(std::function<void(void)> f) {
			add(f);
		};

		void clear();
	};

	

	template <typename Ty>
	struct MsgChan  {
		typedef std::function<void(Ty)> Func;
		struct MsgChanHandler {
			Str						name;
			Func function;

			MsgChanHandler() {

			};
			MsgChanHandler(Func f) {
				function = f;
			};
		};

		std::vector<MsgChanHandler*>				signals;
		std::vector<MsgChanHandler*>				signalsOnce;
		bool										isTriggering = false;

		MsgChan(){}
		virtual ~MsgChan() {
			for (auto sig : signals)			delete sig;
			for (auto sigOnce : signalsOnce)	delete sigOnce;

			signals.clear();
			signalsOnce.clear();
		}

		MsgChanHandler* add(Func f, MsgChanHandler * _sig = nullptr) {
			MsgChanHandler* sig = nullptr;
			if (_sig) {
				sig = _sig;
				sig->function = f;
			}
			else 
				sig = new MsgChanHandler(f);
			signals.push_back(sig);
			return sig;
		};

		inline int			size() {
			return signals.size() + signalsOnce.size();
		};

		auto add(const char * name, Func f) {
			MsgChanHandler* sig = new MsgChanHandler(f);
			sig->name = name;
			signals.push_back(sig);
			return sig;
		};

		inline void operator+=(Func f) {
			add(f);
		};

		auto addOnce(std::function<void(Ty)> f) {
			MsgChanHandler* sig = new MsgChanHandler(f);
			signalsOnce.push_back(sig);
			return sig;
		};

		auto addOnce(const char * name, Func f) {
			MsgChanHandler* sig = new MsgChanHandler(f);
			sig->name = name;
			signalsOnce.push_back(sig);
			return sig;
		};

		inline void removeByName(const char* handlerName) {
			StrRef ref(handlerName);
			for (auto iter = signals.begin(); iter != signals.end(); ) {
				MsgChanHandler* s = *iter;
				if ( s->name == ref){
					iter = signals.erase(iter);
					break;
				}
				else
					iter++;
			}
			for (auto iter = signalsOnce.begin(); iter != signalsOnce.end(); ) {
				MsgChanHandler* s = *iter;
				if (s->name == ref) {
					iter = signalsOnce.erase(iter);
					break;
				}
				else
					iter++;
			}
		};

		inline void remove(MsgChanHandler*handler) {
			auto idx = std::find(signals.begin(), signals.end(), handler);
			if (idx != signals.end()) {
				signals.erase(idx);
				delete handler;
				return;
			}

			idx = std::find(signalsOnce.begin(), signalsOnce.end(), handler);
			if (idx != signalsOnce.end()) {
				signalsOnce.erase(idx);
				delete handler;
				return;
			}
		};

		virtual void trigger(Ty data) {
			isTriggering = true;
			for (auto s : signals) s->function(data);
			if (signalsOnce.size() > 0) {
				for (auto s : signalsOnce) s->function(data);
				for (auto s : signalsOnce) delete s;
				signalsOnce.clear();
			}
			isTriggering = false;
		};

		inline void operator()(Ty data) {
			trigger(data);
		};

		void clear() {
			for (auto sig : signals)			delete sig;
			for (auto sigOnce : signalsOnce)	delete sigOnce;
			signals.clear();
			signalsOnce.clear();
		};
	};

	typedef MsgChan<float>				MsgChanF;
	typedef MsgChan<int>				MsgChanI;
	typedef MsgChan<bool>				MsgChanB;
	typedef MsgChan<std::string>		MsgChanStdStr;
	typedef MsgChan<const char *>		MsgChanCStr;
	typedef MsgChan<Str>				MsgChanStr;
	typedef MsgChan<const r::Vector2&>	MsgChanVec2;
}


