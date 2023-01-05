#include "stdafx.h"
#include "Garbage.hpp"

static eastl::vector<r2::Node*>			nodeBin;
static eastl::vector<r2::BatchElem*>	beBin;

using namespace rd;
void Garbage::init() {
	nodeBin = {};
	beBin = {};

	nodeBin.reserve(32);
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
		}
	);
}

void Garbage::trash(r2::Node* n) {
	nodeBin.push_back(n);
}

void Garbage::trash(r2::BatchElem* n) {
	beBin.push_back(n);
}
