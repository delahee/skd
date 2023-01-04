#include "stdafx.h"

#include "GaussianBlur.hpp"

using namespace std;
using namespace r2;
using namespace r2::svc;

bool GaussianBlur::recordTile = false;
eastl::vector<r2::Tile> GaussianBlur::debugTiles ;
GaussianBlur::GaussianBlur()
{
	capture = new Capture();
	paddedCapture = new Capture();
	kH = new Kernel();
	kV = new Kernel();
}

static GaussianBlur * g = nullptr;

GaussianBlur & GaussianBlur::get() {
	if (g == nullptr)
		g = new GaussianBlur();
	return *g;
}

GaussianBlur::~GaussianBlur() {
	delete capture;
	delete paddedCapture;
	delete kH;
	delete kV;
}

r2::Tile * GaussianBlur::make(r2::Node * n, const GaussianBlurCtrl& ctrl) {
	lastCtrl = ctrl;
	auto size = ctrl.size;
	if (size.x <= 0 && size.y <= 0) 
		return capture->make(n, ctrl.filter);

	capture->make(n, ctrl.filter, 0);//TODO : we could avoid this, maybe later
	return make(capture->getWorkingTile(), ctrl);
}

void r2::svc::GaussianBlur::im(){
	ImGui::PushID(this);
	if (ImGui::CollapsingHeader("gblur")) {
		
		ImGui::Indent();
		ImGui::LabelText("cpad","%f",computedPad);
		
		ImGui::Text("capture");
		capture->im();
		ImGui::Text("paddedCapture");
		paddedCapture->im();
		
		ImGui::Text("kH");
		kH->im();
		ImGui::Text("kV");
		kV->im();
		if (ImGui::CollapsingHeader("Last Ctrl")) {
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.6f);
			lastCtrl.im();
			ImGui::PopStyleVar(ImGuiStyleVar_Alpha);
		}
		ImGui::Unindent();
		
	}
	ImGui::PopID();
}

r2::Tile * GaussianBlur::make(r::Texture * tex, const GaussianBlurCtrl& ctrl) {
	lastCtrl = ctrl;
	auto size = ctrl.size;
	if (size.x <= 0 && size.y <= 0) {
		capture->dummy.mapTexture(tex);
		capture->rd.isFresh = true;
		return &capture->dummy;
	}

	stub.mapTexture(tex);
	return make(&stub, ctrl);
}

r2::Tile* GaussianBlur::make(r2::Tile * n, const GaussianBlurCtrl& ctrl){
	lastCtrl = ctrl;
	auto size = ctrl.size;

	if (size.x == 0 && size.y == 0) {
		workingTile.copy(*n);
		dummy.copy(*n);
		return &dummy;
	}
		
	auto filter = ctrl.filter;
	auto scale = ctrl.offsetScale;

	//make a capture of adequate size
	double mx = Kernel::getKernelWidth(size.x);
	double my = Kernel::getKernelWidth(size.y);

	double maxForPad = max(mx, my);
	int maxForPadI = ceil(maxForPad);

	if (ctrl.doPad) {
		paddedCapture->make(n, std::vector<BmpOp>(), filter, maxForPadI);
		n = paddedCapture->getWorkingTile();
		computedPad = maxForPad;
	}
	else {
		maxForPad = 0; maxForPadI = 0;
		computedPad = maxForPad;
	}

	if (size.y <= 0)//only blur X
	{
		kH->makeBlur1D_KeepOffset(n, size.x, scale.x, filter, true);
		r2::Tile* tH = kH->rd.getWorkingTile();
		workingTile.copy(*tH);
		return kH->rd.getDrawingTile();
	}

	if (size.x <= 0)//only blur X
	{
		kV->makeBlur1D_KeepOffset(n, size.y, scale.y, filter, false);
		r2::Tile* tV = kV->rd.getWorkingTile();
		//if (maxForPad)
		//	tV->translateCenterDiscrete(-maxForPad, -maxForPad+1);
		workingTile.copy(*kV->rd.getWorkingTile());
		return kV->rd.getDrawingTile();
	}

	kH->makeBlur1D_KeepOffset(n, size.x, scale.x, filter, true);
	n = kH->getWorkingTile();
	kV->makeBlur1D_KeepOffset(n, size.y, scale.y, filter, false);
	n = kV->rd.getWorkingTile();
	workingTile.copy(*n);
	return kV->rd.getDrawingTile();
}