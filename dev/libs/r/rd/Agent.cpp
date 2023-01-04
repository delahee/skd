#include "stdafx.h"

#include "Agent.hpp"

using namespace rd;

int Agent::_UID = 0;

void rd::AgentList::dispose() {
	int sz = repo.size();
	for (int i = sz - 1; i >= 0; i--) {
		Agent * ag = repo[i];
		ag->dispose();
	}
	repo.erase(repo.begin(), repo.end());
}

void AgentList::remove(Agent * p) {
	auto find = std::find(repo.begin(), repo.end(), p);
	if(find != repo.end())
		repo.erase(find);
	p->list = nullptr;
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

void rd::AgentList::add(Agent * p) {
	if (p->list) p->list->remove(p);

	for (int iter = repo.size() - 1; iter >= 0; iter--)
		if (p == repo[iter])
			return;

	repo.push_back(p);
	p->list = this;
}

void rd::AgentList::update(double dt) {
	int sz = repo.size();
	for( int i = sz-1; i >= 0; i-- ){
		Agent * ag = repo[i];
		ag->update(dt);
	}
}

void rd::AgentList::deleteAllChildren()
{
	for (int iter = repo.size() - 1; iter >= 0; iter--) {
		Agent * ag = repo[iter];
		delete ag;
	}
	repo.clear();
}

rd::Agent::Agent(AgentList * list) {
	if (list != nullptr) {
		this->list = list;
		list->add(this);
	}
}

void rd::Agent::detach() {
	if (list != nullptr) {
		list->remove(this);
		list = nullptr;
	}
}

void rd::Agent::dispose() {
	detach();
	if (deleteSelf) {
		deleteSelf = false;
		delete this;
	}
}


Agent::~Agent() {
	deleteSelf = false;
	dispose();
}

void rd::AnonAgent::dispose() {
	Agent::dispose();
	cbk = nullptr;
}
