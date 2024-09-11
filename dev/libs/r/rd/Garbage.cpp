#include "stdafx.h"
#include "Garbage.hpp"

static eastl::vector<r2::Node*>			nodeBin;
static eastl::vector<r2::BatchElem*>	beBin;
static eastl::vector<rd::Agent*>		agBin;
static eastl::vector<r::Object*>		objBin;

using namespace rd;
void Garbage::init() {
	nodeBin = {};
	beBin = {};
	agBin = {};
	objBin = {};

	nodeBin.reserve(32);
	agBin.reserve(32);
	beBin.reserve(32);
	objBin.reserve(32);

	Svc::reg( "Garbage Service", gc);
}

void Garbage::gc() {
	if (nodeBin.size()) {
		for (auto n : nodeBin) {
			if (n) {
				//traceNode("garbage", n);
				n->destroy();
			}
		}
		nodeBin.clear();
	}

	if (beBin.size()) {
		for (auto be : beBin)
			if (be) be->destroy();
		beBin.clear();
	}
	if (agBin.size()) {
		for (auto ag : agBin) {
			try {
				//traceAgent("binning ", ag);
				//if you crash here, you probably deleted an agent in the parent loop and it was not removed from list yet
				//you may also have sent it to garbage AND deleted it
				//get up to the deleted thing and call safeDestruction rather than delete this
				ag->dispose();
			}
			catch (std::exception&) {
				traceError("exception occured during garbage, thing was probably already deleted");
			}
		}
		agBin.clear();
	}
	if (objBin.size()) {
		for (auto o : objBin)
			delete o;
		objBin.clear();
	}
}

void Garbage::trashObject(r::Object * n) {
	if (n == nullptr) return;
	if (rs::Std::exists(objBin, n)) return;//already in here
	objBin.push_back(n);
}

void Garbage::trash(r2::Node* n) {
	if (n == nullptr) return;
	if (rs::Std::exists(nodeBin, n)) return;//already in here
	nodeBin.push_back(n);
}

void Garbage::trash(rd::Agent* n){
	if (n == nullptr) return;
	if (rs::Std::exists(agBin,n)) return;//already in here
	agBin.push_back(n);
}

void Garbage::trash(r2::BatchElem* n) {
	if (n == nullptr) return;
	if (rs::Std::exists(beBin, n)) return;//already in here
	beBin.push_back(n);
}
