#include "stdafx.h"
#include "r2/Node.hpp"
#include "r2/Text.hpp"
#include "r2/Graphics.hpp"
#include "r2/Scene.hpp"
#include "r2/BatchElem.hpp"
#include "BatchElemExplorer.hpp"
#include "NodeExplorer.hpp"
#include "r2/im/TilePicker.hpp"
#include "4-menus/imgui_internal.h"

using namespace std;
using namespace r2;
using namespace r2::im;

static double dmin = -1000.0;
static double dmax = 1000.0;
static const char* blends[] = { "Opaque", "Clip Alpha", "Alpha", "Add", "Screen", "Multiply", "Alpha Multiply", "Erase" };

std::vector<BatchElemExplorer*>	BatchElemExplorer::ALL;

#define SUPER Agent

BatchElemExplorer::BatchElemExplorer(r2::BatchElem*_n, bool killPrevious) : n(_n) {
	if (killPrevious) 
		killAll();
	rd::Agent::name = "BatchElem Explorer";
	name = n->name.cpp_str() + " uid:" + to_string(n->uid);
	name = string("Batch Element Properties: ") + name.cpp_str() + "###BatchElemInspector";
	if (n->batch) {
		Scene* sc = n->batch->getScene();
		if (sc)
			sc->al.push_back(this);
	}
	else
		rs::Sys::exitFrameProcesses.add(this);

	nUid = n->uid;
	ALL.push_back(this);
	doFocus = true;
}

BatchElemExplorer::~BatchElemExplorer() {
	SUPER::dispose();
	n = nullptr;

	//can happen if external deletion for iteration
	auto iter = std::find(BatchElemExplorer::ALL.begin(), BatchElemExplorer::ALL.end(), this);
	if (iter != ALL.end())BatchElemExplorer::ALL.erase(iter);
}

void BatchElemExplorer::update(double dt) {

	//try to handle deleted nodes without crashing
	try{
		volatile auto str = n->name.c_str();
	}
	catch(std::exception e){
		n = nullptr;
		opened = false;
		safeDestruction();
		return;
	}

	//continue anyway

	if (doFocus) {
		ImGuiWindow* window = ImGui::FindWindowByName(name.c_str());
		if (window && window->DockNode && window->DockNode->TabBar)
			window->DockNode->TabBar->NextSelectedTabId = window->ID;
		doFocus = false;
	}

	ImGui::SetNextWindowSize(ImVec2(350, 500), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(name.c_str(), &opened, ImGuiWindowFlags_MenuBar)) {
		ImGui::PushID(n);
		bool deleted = false;
		if (ImGui::BeginMenuBar()) {
			if (ImGui::MenuItem(ICON_MD_SEARCH " Inspect Parent"))
				r2::im::NodeExplorer::edit(n->batch);
			ImGui::Separator();
			if (ImGui::MenuItem(ICON_MD_DELETE " Delete")) {
				n->destroy();
				deleted = true;
				opened = false;
			}
			ImGui::EndMenuBar();
		}
		if (!deleted) {
			if (keepTargetHighlighted || ImGui::IsWindowFocused())
				r2::Im::bounds(n);
			if (nUid == n->uid) 
				n->im();
			else
				ImGui::Text("element was deleted?");
		}
		ImGui::PopID();
	}
	ImGui::End();
	if (!opened) safeDestruction();
}

BatchElemExplorer* BatchElemExplorer::edit(r2::BatchElem * _n) {
	for (BatchElemExplorer * ne : ALL)
		if (ne->n == _n) {
			ne->doFocus = true;
			ne->opened = true;
			return ne;
		}
	return new BatchElemExplorer(_n, true);
}

void BatchElemExplorer::onDeletion(r2::BatchElem * n) {
	for (auto ne = ALL.begin(); ne != ALL.end();) {
		if ((*ne)->n == n) {
			BatchElemExplorer* lne = *ne;
			ne = ALL.erase(ne);
			delete(lne);
		}
		else
			ne++;
	}
}

void BatchElem::im() {
	using namespace ImGui;
	if (destroyed) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
		ImGui::Text(ICON_MD_WARNING " This element is deleted");
		ImGui::PopStyleColor();
	}
	if (ImGui::CollapsingHeader(ICON_MD_WALLPAPER " Batch Element", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PushItemWidth(60); ImGui::Indent();

		ImGui::SetNextItemWidth(150);
		ImGui::InputText("name", name);
		ImGui::Checkbox("visible", &visible);
		ImGui::SetNextItemWidth(188);
		ImGui::DragFloat3("pos", &x); ImGui::SameLine();
		ImGui::Checkbox("useSimpleZ", &useSimpleZ);
		if (!useSimpleZ) {
			ImGui::SetNextItemWidth(124);
			ImGui::DragFloat2("zTop / zBotton", &zTopOffset, 0.1f);
		}
		ImGui::DragFloat("rotation", &rotation, 0.05f, -PASTA_PI * 2, PASTA_PI * 2);
		ImGui::SetNextItemWidth(124);
		if(tile)
			ImGui::Value("size", Vector2(width(),height()));
		ImGui::DragFloat2("scale", &scaleX);

		if (ImGui::TreeNode("Flags")) {
#define FLAG_CHECKBOX(bitset,name,flag) {\
					bool is ## name = (bitset & (flag)) == (flag); \
					if (ImGui::Checkbox( #name, &(is ## name) )) \
						rd::Bits::toggle(bitset, flag, is ## name); \
					}

				FLAG_CHECKBOX(beFlags, Utility, NodeFlags::NF_UTILITY);
				FLAG_CHECKBOX(beFlags, OrigFromPool, NodeFlags::NF_ORIGINATES_FROM_POOL);
				FLAG_CHECKBOX(beFlags, EditorFrozen, NodeFlags::NF_EDITOR_FROZEN);
				FLAG_CHECKBOX(beFlags, EditorProtected, NodeFlags::NF_EDITOR_PROTECT);
				FLAG_CHECKBOX(beFlags, EngineHelper, NodeFlags::NF_ENGINE_HELPER);
#undef FLAG_CHECKBOX
			ImGui::TreePop();
		}

		ImGui::PushItemWidth(120); 
		Str tags = vars.getTags();
		ImGui::LabelText("tags", tags.c_str());
		vars.im();
		ImGui::PopItemWidth();
		
		ImGui::Separator();

		if (ImGui::DragScalar("priority", ImGuiDataType_Double, &priority, 0.1f, &dmin, &dmax, "%0.3f"))
			setPriority(priority);

		ImGui::SetNextItemWidth(124);
		ImGui::ColorPicker4("color", color.ptr());
		ImGui::DragFloat("alpha", &alpha, 0.05f, 0, 2.2f);

		ImGui::SetNextItemWidth(150);
		if (ImGui::BeginCombo("blendmode", blendmode == TransparencyType::TT_INHERIT ? "Inherit" : blends[blendmode])) {
			{
				bool is_selected = blendmode == TransparencyType::TT_INHERIT;
				if (ImGui::Selectable("Inherit", is_selected))
					blendmode = TransparencyType::TT_INHERIT;
				if (is_selected) ImGui::SetItemDefaultFocus();
				ImGui::Separator();
			}
			for (int n = 0; n < IM_ARRAYSIZE(blends); n++) {
				bool is_selected = (n == blendmode);
				if (ImGui::Selectable(blends[n], is_selected))
					blendmode = (TransparencyType)n;
				if (is_selected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (ImGui::TreeNode(ICON_MD_GAMEPAD " Actions")) {
			if( Button("Flip X"))
				tile->flipX();
			SameLine();
			if (Button("Flip Y"))
				tile->flipY();
			TreePop();
		}

		if (ImGui::TreeNode(ICON_MD_IMAGE " Tile")) {
			if (tile) tile->im();
			if (ImGui::Button("Change Tile")) {
				if (!this->tile) 
					this->setTile(rd::Pools::tiles.alloc(),true);
					
				
				rd::Promise* p = r2::im::TilePicker::forTile(*tile);
			};
			ImGui::TreePop();
		}

		rd::ABatchElem* asBe = dynamic_cast<rd::ABatchElem*>(this);
		if (asBe) {
			if (TreeNode("Animated")) {
				asBe->player.im();
				TreePop();
			}
		}

		ImGui::Unindent();
		ImGui::PopItemWidth();
	}
}

void BatchElemExplorer::killAll() {
	auto copy = std::vector(BatchElemExplorer::ALL);
	ALL.clear();
	for (auto& n : copy) {
		delete n;
	}
}

#undef SUPER