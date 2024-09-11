#include "stdafx.h"
#include "NodeAgent.hpp"
#include "rd/Agent.hpp"
#include "rd/JSerialize.hpp"

using namespace r2;

eastl::vector<r2::NodeAgentFactory::NodeAgentEntry> r2::NodeAgentFactory::customCreates;

r2::NodeAgent* r2::NodeAgentFactory::create(const char* nodeName) {
	#define eq rd::String::equalsI
	if (eq(nodeName, NA_LIST))
		return new NodeAgentList();
	for(auto&cr : customCreates)
		if (rd::String::equalsI(cr.typeName, nodeName))
			return cr.ctor();
	return 0;
}

bool r2::NodeAgentFactory::imCreate(const char* prefix, NodeAgent*& res){
	using namespace ImGui;
	if (BeginCombo( prefix,"")) {
		for (auto& c : customCreates) {
			bool sel = false;
			if (Selectable(c.typeName.c_str(), &sel)) {
				res = c.ctor();
				EndCombo();
				return true;
			}
		}
		EndCombo();
	}
	return false;
}

void r2::NodeAgentFactory::addCustomType(const char* _name, std::function<NodeAgent* ()> ctor){
	for (auto& e : customCreates)
		if (e.typeName == _name) {
			traceWarning("type already registered");
			return;
		}

	customCreates.push_back({ _name,ctor });
}

bool r2::NodeAgent::im() {
	using namespace ImGui;
	bool chg = false; 
	chg|=data.im("data");
	if (TreeNode("internal")) {
		Super::im();
		TreePop();
	}
	return chg;
}

void r2::NodeAgent::apply()
{
}

r2::NodeAgent::NodeAgent(r2::Node* n, rd::AgentList* parent) : Super(parent) {
	deleteSelf = true;
	node = n;
}

r2::NodeAgent::~NodeAgent() {
	node = 0;
}

inline void r2::NodeAgent::update(double dt) {
	Agent::update(dt);
	if (cbk) cbk(node, dt);
}

void r2::NodeAgent::serialize(Pasta::JReflect& jr, const char* _name) {
	if (_name) jr.visitObjectBegin(_name);
	//Str ltype;
	//jr.visit(ltype, "type");
	jr.visit(data, "data");
	if (_name) jr.visitObjectEnd(_name);
}

r2::NodeAgentList::NodeAgentList(r2::Node* n, rd::AgentList* parent) : Super(n, parent) {

}

void r2::NodeAgentList::update(double dt){
	Super::update(dt);
	agents.update(dt);
}

r2::NodeAgentList::~NodeAgentList(){
	deleteSelf = false;
	agents.dispose();
	dispose();
}

bool r2::NodeAgentList::im(){
	using namespace ImGui;
	bool chg = false;
	for (auto a : agents.repo) {
		if (!a) continue;
		PushID(a);
		auto asCmp = dynamic_cast<r2::NodeAgent*>(a);
		if (TreeNode(asCmp->type)) {
			chg |= a->im();
			TreePop();
		}
		PopID();
	}
	return chg;
}

void r2::NodeAgentList::serializeArray(Pasta::JReflect& jr, eastl::vector<r2::NodeAgent*>& arr, u32 size, const char* name) {
	u32 similarCount = 0;
	if (jr.visitArrayBegin(name, size)) {
		if (jr.isReadMode() && size)
			arr.resize(size);
		for (u32 i = 0; i < size; ++i) {
			jr.visitIndexBegin(i);
			jr.visitObjectBegin(nullptr);
			if (jr.isReadMode()) {
				Str tname;
				jr.visit(tname, "type");
				arr[i] = NodeAgentFactory::create(tname.c_str());
			}
			jr.visit(*arr[i], nullptr);
			jr.visitObjectEnd(nullptr);
			if (jr.visitIndexEnd())
				++similarCount;
		}
	}
	jr.visitArrayEnd(name);
	if (!jr.isReadMode() && similarCount == size)
		jr.m_jvalue->EraseMember(name);
}

void r2::NodeAgentList::serialize(Pasta::JReflect& jr, const char* _name){
	if (_name) jr.visitObjectBegin(_name);

	auto ags = getNodeAgents();
	serializeArray(jr, ags, ags.size(), "children");
	
	if (_name) jr.visitObjectEnd(_name);
}

eastl::vector<r2::NodeAgent*> r2::NodeAgentList::getNodeAgents(){
	eastl::vector<NodeAgent*> res;
	for (auto a : agents.repo) {
		auto asNodeAgent = dynamic_cast<r2::NodeAgent*>(a);
		if (asNodeAgent)res.push_back(asNodeAgent);
	}
	return res;
}


#undef eq

