#include "stdafx.h"

#include "NodeExplorer.hpp"

using namespace std;
using namespace r2;
using namespace r2::im;

std::vector<NodeExplorer*>	NodeExplorer::ALL;

#define SUPER Agent

NodeExplorer::NodeExplorer(r2::Node*_n,bool killPrevious) : n(_n) {
	if (killPrevious) {
		killAll();
	}
	rd::Agent::name = "Node Explorer";
	name = n->name.cpp_str() + " uid:" + to_string(n->uid) + "###NodeInspector";
	name = std::string("Properties: ") + name;
	rs::Svc::reg(this);
	ALL.push_back(this);
	sigDelete = n->onDeletion.addOnce([this]() { onDeletion(n); });
}

NodeExplorer::~NodeExplorer() {
	SUPER::dispose();
	if(sigDelete) n->onDeletion.remove(sigDelete);
	sigDelete = nullptr;
	n = nullptr;

	//can happen if external deletion for iteration
	auto iter = std::find(NodeExplorer::ALL.begin(), NodeExplorer::ALL.end(), this);
	if (iter != ALL.end())NodeExplorer::ALL.erase(iter);
	rs::Svc::unreg(this);
}

void NodeExplorer::update(double dt) {
	ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
	ImGui::Begin(name.c_str(), &opened, ImGuiWindowFlags_MenuBar);
	ImGui::PushID(name.c_str());
	if(keepTargetHighlighted || ImGui::IsWindowFocused())
		r2::Im::bounds(n);
	n->im();
	ImGui::PopID();
	ImGui::End();
	if (!opened) {
		safeDestruction();
		return;
	}
}

void NodeExplorer::cancel(r2::Node* _n) {
	if (_n == nullptr) return;
	for (NodeExplorer* ne : ALL)
		if (ne->n == _n) {
			ne->opened = false;
			return;
		}
}

NodeExplorer* NodeExplorer::edit(r2::Node* _n) {
#ifdef PASTA_FINAL
	return 0;
#endif
	if (_n == nullptr) return 0;
	for (NodeExplorer * ne : ALL)
		if (ne->n == _n) 
			return ne;
	return new NodeExplorer(_n,true);
}

void NodeExplorer::onDeletion(r2::Node * _n) {
	if (_n == nullptr) return;
	for (auto ne = ALL.begin(); ne != ALL.end();) {
		if ((*ne)->n == _n) {
			NodeExplorer* lne = *ne;
			ne = ALL.erase(ne);
			delete(lne);
		}
		else
			ne++;
	}

	for (r2::Node* c : _n->children) 
		onDeletion(c);
}

void r2::im::NodeExplorer::killAll(){
	if (ALL.empty()) return;
	auto copy = std::vector(ALL);
	ALL.clear();
	for (auto& n : copy) 
		delete n;
}


#undef SUPER