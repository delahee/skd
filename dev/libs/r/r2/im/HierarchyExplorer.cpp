#include "stdafx.h"

#include "NodeExplorer.hpp"
#include "BatchElemExplorer.hpp"
#include "HierarchyExplorer.hpp"

#include "rd/JSerialize.hpp"
#include "rd/AudioEmitter2D.hpp"
#include "rs/Std.hpp"
#include "ri18n/RichText.hpp"

using namespace std;
using namespace r;
using namespace r2;
using namespace rd;
using namespace r2::im;

static string noName = "noname";

eastl::vector<r2::Scene*>		HierarchyExplorer::scs;
HierarchyExplorer*				HierarchyExplorer::me = nullptr;
HierarchyExplorer::GameIPC		HierarchyExplorer::gameIPC;

HierarchyExplorer::HierarchyExplorer(r2::Scene* _sc, rd::AgentList* al) : rd::Agent(al) {
	me = this;
	name = "Hierarchy Explorer";
	addScene(_sc);
}

void HierarchyExplorer::removeScene(r2::Scene* _sc) {
	if (!_sc) return;
	if (!rs::Std::exists(scs, _sc))
		return;

	Std::remove(scs, _sc);
}

void HierarchyExplorer::addScene(r2::Scene* _sc) {
	if (!_sc) return;
	if (rs::Std::exists(scs, _sc))
		return;

	scs.push_back(_sc);
	_sc->onDeletion.addOnce([_sc]() {
		Std::remove(scs, _sc);
	});
}

HierarchyExplorer::~HierarchyExplorer() {
	onDestruction();
	me = nullptr;
}

void HierarchyExplorer::hide() {
	if (me != nullptr)
		me->imOpened =false;
}

void HierarchyExplorer::toggle(r2::Scene* sc) {
	if (me == nullptr)
		new r2::im::HierarchyExplorer(sc, &rs::Sys::enterFrameProcesses);
	else {
		auto inst = me;
		if (sc == 0) 
			inst->imOpened = false;
		else {
			inst->addScene(sc);
			inst->imOpened = !inst->imOpened;
		}
		inst->onImChanged();
	}
}

r2::Node* HierarchyExplorer::popupType() {
	if (ImGui::BeginPopupContextItem("TypeSelect", 0)) {
		r2::Node* nu = nullptr;

		auto &ipc = HierarchyExplorer::gameIPC;

		ipc.onImAddNodeBegin();
		if (ipc.nu)
			nu = ipc.nu;

		if (ImGui::Selectable("Node")) nu = new Node();
		//not sure anybody want to make a regular "sprite" but can be useful as a filter cache with custom props
		if (ImGui::Selectable("Sprite")) nu = new Sprite();
		if (ImGui::Selectable("Bitmap")) {
			auto bmp = new Bitmap();
			nu = bmp;
			bmp->copyTile(r2::GpuObjects::whiteTile);
		}
		if (ImGui::Selectable("Anim. Bitmap")) nu = new ABitmap();
		if (ImGui::Selectable("Graphics")) nu = new Graphics();
		if (ImGui::Selectable("VertexArray")) nu = new VertexArray();
		if (ImGui::Selectable("Batch")) nu = new Batch();
		if (ImGui::Selectable("Text")) nu = new Text(nullptr, "this is a text!");
		if (ImGui::Selectable("Rich Text")) nu = new ri18n::RichText(nullptr, "this is a text!");
		if (ImGui::Selectable("Scissor"))
			nu = new Scissor(Rect(0,0,200,200), nullptr);
		if (ImGui::Selectable("Audio Emitter 2D"))
			nu = new rd::AudioEmitter2D(nullptr);
		if (ImGui::Selectable("Slice3")) {
			Slice3Param prm;
			nu = new Slice3(nullptr, prm);
		}
		if (ImGui::Selectable("Slice9")) {
			Slice9Param prm;
			nu = new Slice9(nullptr, prm);
		}
		if (ImGui::Selectable("Node3D")) nu = new r3::Node3D(nullptr);
		if (ImGui::Selectable("Button")) nu = new rui::Button("", nullptr, nullptr);

		ipc.onImAddNodeEnd();
		if (ipc.nu)
			nu = ipc.nu;

		ImGui::EndPopup();
		return nu;
	}
	return nullptr;
}

// maybe find a way to combine this two functions
r2::Node* HierarchyExplorer::menuType() {
	r2::Node* nu = nullptr;
	if (ImGui::MenuItem("Node")) nu = new Node();
	//not sure anybody want to make a regular "sprite" but can be useful as a filter cache with custom props
	if (ImGui::MenuItem("Sprite")) nu = new Sprite();
	if (ImGui::MenuItem("Bitmap")) {
		auto bmp = new Bitmap();
		nu = bmp;
		bmp->copyTile(r2::GpuObjects::whiteTile);
	}
	if (ImGui::MenuItem("Anim. Bitmap")) nu = new ABitmap();
	if (ImGui::MenuItem("Graphics")) nu = new Graphics();
	if (ImGui::MenuItem("Batch")) nu = new Batch();
	if (ImGui::MenuItem("Text")) nu = new Text(nullptr, "this is a text!");
	if (ImGui::MenuItem("Rich Text")) nu = new ri18n::RichText(nullptr, "this is a text!");
	if (ImGui::MenuItem("Scissor")) nu = new Scissor(Rect(0, 0, 100, 100));
	if (ImGui::MenuItem("Slice3")) {
		Slice3Param prm;
		nu = new Slice3(nullptr,prm);
	}
	if (ImGui::MenuItem("Slice9")) {
		Slice9Param prm;
		nu = new Slice9(nullptr, prm);
	}
	if (ImGui::MenuItem("Audio Emitter 2D"))
		nu = new rd::AudioEmitter2D(nullptr);
	return nu;
}

bool HierarchyExplorer::popupAction(r2::Node* n, bool showEdit, int mouseButton) {
	bool skipAfter = false;
	bool edProtected = n->nodeFlags & NF_EDITOR_PROTECT;
	bool isUtil = n->nodeFlags & NF_UTILITY;
	if (ImGui::BeginPopupContextItem("Actions", mouseButton)) {
		if (showEdit && ImGui::MenuItem("Edit")) 
			NodeExplorer::edit(n);
		if (ImGui::MenuItem("Save")) 
			save(n, nullptr);
		if (ImGui::MenuItem("Load")) 
			load(n, nullptr);
		if (ImGui::MenuItem("Clone")) {
			auto cl = n->clone();
			n->parent->addChild(cl);
			r2::im::NodeExplorer::edit(cl);
		}
		if (n->parent && !edProtected && !isUtil) {
			if (ImGui::BeginMenu("Change type")) {
				r2::Node* nu = menuType();
				if (nu) {
					skipAfter = true;
					r2::Lib::changeType(nu, n);
					if (n) n->destroy();
					NodeExplorer::edit(nu);
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Delete")) {
				if(n) n->destroy();
				skipAfter = true;
			}
		}
		ImGui::EndPopup();
	}
	return skipAfter;
}

void HierarchyExplorer::imToolbar(r2::Node* n, bool& skipAfter) {
	using namespace ImGui;
	bool edProtected = n->nodeFlags & NF_EDITOR_PROTECT;
	bool isUtil = n->nodeFlags & NF_UTILITY;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 4));

	if (ImGui::Button(n->visible ? ICON_MD_VISIBILITY : ICON_MD_VISIBILITY_OFF))
		n->visible = !n->visible;

	SameLine();
	if (ImGui::Button(ICON_MD_EDIT))
		NodeExplorer::edit(n);

	ImGui::SameLine();
	ImGui::Button(ICON_MD_MORE_HORIZ);
	skipAfter = popupAction(n);

	if (!isUtil) {
		ImGui::PushID("add");
		SameLine();
		ImGui::Button(ICON_MD_ADD);

		auto& ipc = gameIPC;
		ipc.cur = n;
		r2::Node* nu = popupType();
		if (nu) {
			n->addChildAt(nu, 0);
			NodeExplorer::edit(nu);
		}
		ipc.nu = ipc.cur = 0;
		ImGui::PopID();

		if (!edProtected) {
			if (n->parent) {
				ImGui::SameLine();
				if (ImGui::Button(ICON_MD_ARROW_UPWARD)) {
					int idx = n->parent->getChildIndex(n);
					n->parent->setChildIndex(n, idx - 1);
				}
				ImGui::SameLine();
				if (ImGui::Button(ICON_MD_ARROW_DOWNWARD)) {
					int idx = n->parent->getChildIndex(n);
					n->parent->setChildIndex(n, idx + 1);
				}
			}
		}
	}

	ImGui::PopStyleVar(1);
}

void HierarchyExplorer::im(r2::Node* n, int depth) {
	using namespace ImGui;

	if (!imOpened)
		return;
	bool isUtil = n->nodeFlags & NF_UTILITY;
	bool edProtected = n->nodeFlags & NF_EDITOR_PROTECT;

	if (strlen(n->name.c_str()) == 0) 
		n->name = noName + "#" + to_string(n->uid);

	string id = n->name.cpp_str() + "#" + to_string(n->uid) + "##heToolbar";
	ImGui::PushID(id.c_str());

	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_None;
	if (isUtil) node_flags |= ImGuiTreeNodeFlags_Leaf;
	if (depth == 0) node_flags |= ImGuiTreeNodeFlags_DefaultOpen;

	bool hidden = false;
	if (searchFilter != "") {
		if (rd::String::stristr(searchFilter.c_str(), "beuid:")) {
			hidden = false;
		}
		else 
		if (rd::String::stristr(searchFilter.c_str(), "uid:")) {
			u64 searchUid = atoll(searchFilter.c_str() + strlen("uid:"));
			if (n->uid != searchUid) {
				hidden = true;
			}
		}
		else {
			if (rd::String::stristr(n->name.c_str(), searchFilter.c_str()) == nullptr) {
				hidden = true;
			}
		}
	}
	bool node_open = false;
	bool skipAfter = false;
	if (!hidden) { // Show the node + toolbar
		node_open = ImGui::TreeNodeEx(n->name.c_str(), node_flags);
		//if (IsItemHovered()) {
		//SetTooltip(std::string().c_str());
		//}

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip | ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
		{
			ImGui::SetDragDropPayload("REORDER_NODE", &n, sizeof(r2::Node*));
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("REORDER_NODE", ImGuiDragDropFlags_AcceptNoDrawDefaultRect | ImGuiDragDropFlags_AcceptBeforeDelivery))
			{
				IM_ASSERT(payload->DataSize == sizeof(r2::Node*));
				r2::Node* nodeToInsert = *(r2::Node**)payload->Data;

				if (n->parent && n->parent == nodeToInsert->parent) {
					int nodeIdx = n->parent->getChildIndex(n);
					n->parent->setChildIndex(nodeToInsert, nodeIdx);
				}
				else if (n->parent && !nodeToInsert->hasChildRec(n)) {  // very WIP
					// do another AcceptDragDropPayload here without any flag to force a mouse release
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("REORDER_NODE", ImGuiDragDropFlags_None))
					{
						n->parent->addChild(nodeToInsert);
						nodeToInsert->putBehind(n);
						skipAfter = true;
					}
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemHovered())
			r2::Im::bounds(n);
		popupAction(n, true, 1);
		ImGui::NextColumn();
		ImGui::PushID("toolbar");
		imToolbar(n, skipAfter);
		ImGui::PopID();
		ImGui::NextColumn();
	}

	if (skipAfter) {
		if (node_open) ImGui::TreePop();
		ImGui::PopID();
		return;
	}

	if (node_open && !isUtil) { // utils are leaf in the tree, the user can't open them
		r2::Batch* asB = dynamic_cast<r2::Batch*>(n);
		if (asB) {
			r2::BatchElem* be = asB->head;
			bool list_open = false;

			string label = "BatchElem list";
			PushID(asB->uid);
			list_open = ImGui::TreeNodeEx(label.c_str());
			if (IsItemHovered())
				SetTooltip(to_string(asB->getNbElements()));
			ImGui::NextColumn();
			//Add toolbar batchelem here	
			ImGui::NextColumn();

			bool hasBeFilter = false;
			r::uid  beSearchUid = 0;
			if (rd::String::stristr(searchFilter.c_str(), "beuid:")) {
				hasBeFilter = true;
				beSearchUid = atoll(searchFilter.c_str() + strlen("beuid:"));
			}
			while (list_open && be) {
				if(hasBeFilter && (be->uid != beSearchUid)){
					be = be->next;
					continue;
				}
				ImGui::PushID(be->uid);
				ImGui::BulletText("x:%6.2f  y:%6.2f z:%6.2f", be->x, be->y, be->z);
				ImGui::NextColumn();

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 4));
				if (ImGui::Button(be->visible ? ICON_MD_VISIBILITY : ICON_MD_VISIBILITY_OFF))
					be->visible = !be->visible;
				if (ImGui::IsItemHovered()) {
					r2::Im::bounds(be);
					SetTooltip( Str256f( "%s uid:%ld", be->name.c_str(),be->uid));
				}
				SameLine();
				Str256f btUID(ICON_MD_IMAGE_SEARCH  "##icbe%ld", be->uid);
				if (ImGui::Button(btUID.c_str()))
					BatchElemExplorer::edit(be);
				if (ImGui::IsItemHovered()) {
					r2::Im::bounds(be);
					SetTooltip(Str256f("%s uid:%ld", be->name.c_str(), be->uid));
				}
				be = be->next;
				ImGui::PopStyleVar(1);
				ImGui::NextColumn();
				ImGui::PopID();
			}
			PopID();
			if (list_open) ImGui::TreePop();
		}
	}

	if (hidden || node_open) { // if hidden skip & search inside, to improve
		for (int i = 0; i < n->children.size(); ++i) {
			auto c = n->children[i];
			if (!showUtilities && (c->nodeFlags & NF_UTILITY))
				continue;
			im(c, depth + 1);
		}
	}

	if (node_open) {
		if (!isUtil) {
			const ImGuiPayload* payload = ImGui::GetDragDropPayload();
			if (payload != NULL && payload->IsDataType("REORDER_NODE")) {
				IM_ASSERT(payload->DataSize == sizeof(r2::Node*));
				r2::Node* nodeToInsert = *(r2::Node**)payload->Data;

				if (n != nodeToInsert->parent && !nodeToInsert->hasChildRec(n)) {  // very WIP
					ImGui::Text("Insert here");
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("REORDER_NODE", ImGuiDragDropFlags_SourceAllowNullID))
						{
							n->addChild(nodeToInsert);
							skipAfter = true;
						}
					}
					ImGui::NextColumn();
					ImGui::NextColumn();
				}
			}

			ImGui::Button(ICON_MD_ADD);
			r2::Node* nu = popupType();
			if (nu) {
				n->addChild(nu);
				NodeExplorer::edit(nu);
			}
			ImGui::NextColumn();
			ImGui::NextColumn();
		}

		ImGui::TreePop();
	}

	ImGui::PopID();
}

void HierarchyExplorer::update(double dt) {
	using namespace ImGui;
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);

	bool opened = imOpened;
	if (ImGui::Begin("Hierarchy Explorer", &opened)) {
		ImGui::Checkbox("show utilities", &showUtilities);
		if (strlen(searchFilter.c_str()) == 0)
			searchFilter = "";
		ImGui::InputText("filter", searchFilter);
		if (IsItemHovered())
			SetTooltip("search string in the hierarchy\nCan use 'uid:' to filter by uids");

		ImGui::Separator();

		ImGui::BeginChild("##ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		for( auto sc : scs)
		if (sc) {
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() - 160);
			ImGui::SetColumnWidth(1, 160);
			im(sc);
			ImGui::Columns(1);
		}
		else ImGui::Text("Error: no scene to inspect");
		ImGui::EndChild();
	}

	ImGui::End();

	if (!opened) safeDestruction();
}

#ifdef PASTA_WIN
#include <filesystem>
static string norm(string path) {
	std::filesystem::path np(path);
	return np.generic_string();
}
#endif

void HierarchyExplorer::save(r2::Node* n, const char* filename) {
	string fname;
	if (!filename)	fname = n->name.cpp_str() + ".meta.json";
	else			fname = filename;
#ifdef PASTA_WIN
	fname = rd::String::replace(fname,":", "_");
	fname = norm(fname);
#endif

	if (!jSerialize(*n, r::Conf::EDITOR_PREFAB_FOLDER, fname.c_str()))
		std::cout << "scene save failed\n";
	else
		trace("saved "s + fname);
}

r2::Node* HierarchyExplorer::load(r2::Node* n, const char* filename) {
	Str fname;
	if (!filename) {
		if (n) {
			fname = n->name;
			fname = rd::String::replace(fname, ":", "_");
			fname = fname + ".meta.json";
		}
	}
	else
		fname = filename;
	if (n == nullptr)
		n = new Node();
	if (!jDeserialize(*n, r::Conf::EDITOR_PREFAB_FOLDER, fname.c_str()))
		std::cout << "scene load failed \n";
	else
		trace("loaded "s + fname.c_str());
	return n;
}

