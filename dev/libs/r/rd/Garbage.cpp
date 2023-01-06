#include "stdafx.h"
#include "Garbage.hpp"

static eastl::vector<r2::Node*>			nodeBin;
static eastl::vector<r2::BatchElem*>	beBin;
static eastl::vector<rd::Agent*>		agBin;

using namespace rd;
void Garbage::init() {
	nodeBin = {};
	beBin = {};
	agBin = {};

	nodeBin.reserve(32);
	agBin.reserve(32);
	beBin.reserve(256);

	Svc::reg(
		[](){
			if (nodeBin.size()) {
				for (auto n : nodeBin)
					n->destroy();
				nodeBin.clear();
			}

			if (beBin.size()) {
				for (auto be : beBin)
					be->destroy();
				beBin.clear();
			}
			if (agBin.size()) {
				for (auto ag : agBin)
					ag->dispose();
				agBin.clear();
			}
		}
	);
}

void Garbage::trash(r2::Node* n) {
	if (n == nullptr) return;
	nodeBin.push_back(n);
}

void Garbage::trash(rd::Agent* n){
	if (n == nullptr) return;
	agBin.push_back(n);
}


void Garbage::trash(r2::BatchElem* n) {
	if (n == nullptr) return;
	beBin.push_back(n);
}
