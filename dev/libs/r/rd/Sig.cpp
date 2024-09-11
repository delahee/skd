#include <stdafx.h>
#include "Sig.hpp"

rd::Sig::Sig() {
}

rd::SignalHandler* rd::Sig::add(std::function<void(void)> f) {
	SignalHandler* sig = new SignalHandler(f);
	signals.push_back(sig);
	return sig;
}

void rd::Sig::remove(rd::SignalHandler* handler) {
	if (!handler) return;

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
}

rd::SignalHandler* rd::Sig::addOnce(std::function<void(void)> f) {
	SignalHandler* sig = new SignalHandler(f);
	signalsOnce.push_back(sig);
	return sig;
}

void rd::Sig::trigger() {
	isTriggering = true;
	triggerredCount++;
	for (auto s : signals) 
		s->function();
	if (signalsOnce.size() > 0) {
		for (auto s : signalsOnce) s->function();
		for (auto s : signalsOnce) delete s;
		signalsOnce.clear();
	}
	isTriggering = false;
}

void rd::Sig::operator()() {
	trigger();
}

void rd::Sig::clear(){
	if(signals.size())
	for (auto sig : signals)
		delete sig;

	if (signalsOnce.size())
	for (auto sigOnce : signalsOnce)
		delete sigOnce;

	signals.clear();
	signalsOnce.clear();
}

rd::Sig::~Sig() {
	for (auto sig : signals)
		delete sig;
	for (auto sigOnce : signalsOnce)
		delete sigOnce;

	signals.clear();
	signalsOnce.clear();
}

rd::SignalHandler::SignalHandler(std::function<void(void)>& f) {
	function = f;
}
