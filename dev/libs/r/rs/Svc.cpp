#include "stdafx.h"
#include "Svc.hpp"

#include <algorithm>
#include <map>

#include "rd/Agent.hpp"

using namespace std;
using namespace rs;
using namespace rd;

static std::multimap<int, Agent*> registry;
static std::multimap<int, Agent*> _tmpRegistry;

bool rs::Svc::has(rd::Agent* ag) {
	auto pos = std::find_if(registry.begin(), registry.end(), [ag](const auto& kv) {
		return kv.second == ag;
	});
	return (pos != registry.end());
}

void rs::Svc::reg(std::function<void(void)> fun) {
	registry.insert(pair(0, new AnonAgent(fun)));
}

void rs::Svc::reg(std::function<void(double dt)> fun) {
	registry.insert(pair(0, new AnonAgent(fun)));
}

void rs::Svc::reg(Agent* ag, int prio){
	if (!ag)
		return;
	if(!has(ag))
		registry.insert(pair(prio, ag));
}

void rs::Svc::unreg(rd::Agent* ag) {
	auto pos = std::find_if(registry.begin(), registry.end(), [ag](const auto & kv) {
		return kv.second == ag;
	});
	if(pos != registry.end())
		registry.erase(pos);

	auto tpos = std::find_if(_tmpRegistry.begin(), _tmpRegistry.end(), [ag](const auto& kv) {
		return kv.second == ag;
	});
	if (tpos == _tmpRegistry.end()) return;
	tpos->second = nullptr;
}

void rs::Svc::update(double dt){
	_tmpRegistry = registry;
	for (auto iter = _tmpRegistry.begin(); iter != _tmpRegistry.end();) {
		rd::Agent* ag = iter->second;
		if (ag == nullptr) {
			iter++;
			continue;
		}
		ag->update(dt);
		iter++;
	}
}

void rs::Svc::im() {
	ImGui::Text("prio : name");
	for (auto& kv : registry) 
		ImGui::Text("%d : %s", kv.first, kv.second->name.c_str());
}


int rs::Svc::count() {
	return registry.size();
}