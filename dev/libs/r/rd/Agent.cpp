#include "stdafx.h"

#include <string>
#include <string_view>
#include "Agent.hpp"

using namespace rd;

int Agent::_UID = 0;

static bool s_debug = 
#ifdef _DEBUG
	true;
#else
	false;
#endif


void rd::AgentList::dispose() {
	int sz = repo.size();
	for (int i = sz - 1; i >= 0; i--) {
		Agent * ag = repo[i];
		if(ag) ag->dispose();
	}
	repo.clear();
}

void rd::AgentList::destroy(Agent* p){
	p->detach();
	rd::Garbage::trash(p);
}

void AgentList::remove(Agent * p) {
	if (iterInProgress) 
		garbage.push_back(p);
	else {
		auto find = std::find(repo.begin(), repo.end(), p);
		if (find != repo.end())
			repo.erase(find);
		p->list = nullptr;
	}
}

rd::AgentList::~AgentList() {
	dispose();
}

void rd::AgentList::push_back(Agent * p) {
	//do not allow multi pushes
	p->detach();

	for (int iter = repo.size() - 1; iter >= 0; iter--)
		if (p == repo[iter])
			return;

	repo.push_back(p);
	p->list = this;
}

bool rd::AgentList::has(const char* _name) {
	StrRef name = _name;
	for (auto a : repo)
		if (name == StrRef(a->name.c_str()))
			return true;
	return false;
}

bool rd::AgentList::has(Agent & ag) {
	for (auto a : repo)
		if (a == &ag)
			return true;
	return false;
}

void rd::AgentList::add(std::function<void(double)> proc) {
	push_back(proc);
}

void rd::AgentList::add(std::function<void(void)> proc) {
	push_back([=](double dt) { proc(); });
}

void rd::AgentList::add(const char* name, std::function<void(void)> proc) {
	auto aa = new AnonAgent(proc, this);
	aa->name = name;
}

void rd::AgentList::add(const char* name, std::function<void(double)> proc) {
	auto aa = new AnonAgent(proc, this);
	aa->name = name;
}

void rd::AgentList::push_back(std::function<void(double)> proc) {
	new AnonAgent(proc, this);
}

void rd::AgentList::add(Agent * p) {
	//if(s_debug) trace( Str256f("adding %s" , p->name.c_str()) );
	 
	if (p->list == this)
		return;

	if (p->list) {
		p->list->remove(p);
		p->list = 0;
	}

	for (auto op : repo)
		if (p == op) {
			if( s_debug ) trace( Str256f("skipping add because duplicate %s > %s", p->name , op->name) );
			return;
		}
	repo.push_back(p);
	p->list = this;
	//if (s_debug && iterInProgress) trace(std::string("iter in progress"));
}

void rd::AgentList::update(double dt) {

	if (garbage.size()) {
		for (auto a : garbage)
			rs::Std::remove(repo, a);
		garbage.clear();
	}

	iterInProgress = true;
	int sz = repo.size();
	for (int i = 0; i < repo.size();++i) {
		Agent * ag = repo[i];
		if( ag )
			ag->update(dt);
	}
	iterInProgress = false;

	if (garbage.size()) {
		for (auto a : garbage)
			rs::Std::remove(repo, a);
		garbage.clear();
	}
}

void rd::AgentList::delay(double ms, r::proc p){
	new DelayedAgent(p, ms, this);
}

static eastl::vector<Agent*> deleteVector;

void rd::AgentList::deleteAllChildren(){
	//trace("deleteAllChildren");
	deleteVector.clear();
	iterInProgress = true;
	for (auto a : repo) {//detach everybody so we don't end up in a reentrancy issue
		if (a) {
			a->detach();
			deleteVector.push_back(a);
		}
	}
	repo.clear();
	for (auto a : deleteVector) //and do the final delete for those who were not removed or externaly managed
		if (!a->deleteSelf)
			delete a;//calls onDispose
		else
			a->dispose();//let the real dispose to its job
	iterInProgress = false;
	deleteVector.clear();
}

rd::Agent* rd::AgentList::getByName(const char* str) {
	for (auto l : repo)
		if (rd::String::equals(l->name.c_str(), str))
			return l;
	return 0;
}

rd::Agent* rd::AgentList::getByTag(const char* str){
	for (auto l : repo)
		if (rd::String::hasTag(l->tags.c_str(), str))
			return l;
	return 0;
}

rd::Agent* rd::AgentList::first() {
	if (repo.empty())return{};
	return repo[0];
}

rd::Agent::Agent(AgentList * list) {
	if (list != nullptr) 
		list->add(this);
}

rd::Agent::Agent(const char * _name, AgentList * list) : Agent(list) {
	name = _name;
	if (!_name || !*_name) {
		int here = 0;
	}
}
	
void rd::Agent::detach() {
	//if(s_debug) 
	//	traceAgent("detached",this);
	if (list != nullptr) {
		list->remove(this);
		list = nullptr;
	}
}

bool rd::Agent::im() { 
	using namespace ImGui;
	bool chg = false;
	Value("_id", _id);
	Value("name",name);
	chg|=Checkbox("deleteSelf",&deleteSelf);
	return chg; 
}

void rd::Agent::onDispose() {

}

void rd::Agent::dispose() {
	//traceAgent("dispose", this);

	if (list && list->iterInProgress) {//avoid reetrancy mess
		detach();
		rd::Garbage::trash(this);//let's try later with no parent
		return;
	}
	else {
		detach();
		onDispose();
		if (deleteSelf) {
			//traceAgent("autoDestr", this);
			deleteSelf = false;
			delete this;
		}
	}
}

Agent::~Agent() {
	//if (s_debug) traceAgent("~Agent", this);
	
	if (list) detach();
	
	deleteSelf = false;
	onDispose();
}

void rd::Agent::setName(const char* _name){
	name.setf("%s#%d", _name,_id);
}

void rd::Agent::setUniqueName(const char* _name){
	name = _name;
}

rd::AnonAgent::~AnonAgent() {
	cbk = {};
}

void rd::AnonAgent::onDispose() {
	cbk = {};
}

void rd::Agent::safeDestruction(){
	if (list) {
		if (list->iterInProgress) {
			detach();
			rs::Timer::delay([this]() { delete this; });
			return;
		}
	}

	//anyway
	delete this;
}
