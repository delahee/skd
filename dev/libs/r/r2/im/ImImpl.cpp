#include "stdafx.h"
#include "4-menus/imgui_internal.h"
#include "r2/filter/all.hpp"
#include "r2/svc/all.hpp"
#include "r2/Bitmap.hpp"

#include "rd/ABitmap.hpp"
#include "rd/Anon.hpp"
#include "r2/Im.hpp"
#include "r2/NodeAgent.hpp"
#include "r2/im/BatchElemExplorer.hpp"
#include "r2/im/NodeExplorer.hpp"
#include "r2/im/TilePicker.hpp"

#include "rd/JSerialize.hpp"
#include "HierarchyExplorer.hpp"
#include "rs/Checksum.hpp"

using namespace std;
using namespace r2;
using namespace rd;
using namespace r2::im;

static double dtmin = -4096;
static double dtmax = 4096;

const char* r2::Im::blends[r::TransparencyType::TRANSPARENCY_TYPE_COUNT] = { 
	"Opaque", "Clip Alpha", "Alpha", "Add", "Screen", "Multiply", "Alpha Multiply", "Erase" };
const char* r2::Im::filters[(u32)r2::FilterType::FT_COUNT] = { //FilterType
	"None", 
	"Base",
	"Layer", 
	"Blur", 
	"Glow", 
	"Bloom",
	"ColorMatrix",
	"Copy",
};
const char* r2::Im::modeMatrix[(int)r2::ColorMatrixMode::Count] = { "HSV", "Colorize", "Matrix" };
const char* r2::Im::texFilters[r2::TexFilter::TF_Count] = { 
	"Nearest neighbor", "Linear", "Anisotropic", "Inherit" };

static float homogeneousMinF = -2;
static float homogeneousMaxF = 2;


void r2::Scene::im() {
	using namespace ImGui;
	//don't call node;
	if (ImGui::CollapsingHeader(ICON_MD_NATURE_PEOPLE " Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PushItemWidth(124); ImGui::Indent();

		ImGui::SetNextItemWidth(150);
		ImGui::InputText("name", name);

		ImGui::SameLine();
		if (ImGui::Button("Save")) {
			if (!jSerialize(*this, r::Conf::EDITOR_PREFAB_FOLDER, name + ".meta.json"))
				std::cout << "scene save failed" << std::endl;
		}

		ImGui::SameLine();
		if (ImGui::Button("Load")) {
			if (!jDeserialize(*this, r::Conf::EDITOR_PREFAB_FOLDER, name.cpp_str() + ".meta.json"))
				std::cout << "scene load failed" << std::endl;
		}

		ImGui::DragFloat2("Pan", cameraPos.ptr(), 0.25f, 0, 0, "%0.2lf");
		ImGui::SetNextItemWidth(60);
		ImGui::DragFloat("Z", &cameraPos.z, 0.25f, 0, 0, "%0.2lf");

		float smin = 0.5;
		float smax = 16.0;
		ImGui::DragFloat2("Zoom", &cameraScale.x, 0.25f, smin, smax, "%0.2lf");

		ImGui::SetNextItemWidth(60);
		int rgz = -round(1 / cameraScale.z);
		if (ImGui::DragInt("Depth Range [-x, x]", &rgz, 1, 1024)) {
			cameraScale.z = -1.0 / rgz;
		}

		if (TreeNode("Internal Dimensions")) {
			Value("fixedSize", this->fixedSize);
			Value("areDimensionsOverriden", areDimensionsOverriden);
			DragFloat("Scene width", &sceneWidth);
			DragFloat("Scene height", &sceneHeight);
			TreePop();
		}

		if (TreeNode("Matrices")) {
			Value("view", viewMatrix);
			Value("proj", projMatrix);
			Value("trs", mat);
			TreePop();
		}

		if (TreeNode("Inputs")) {
			if (currentFocus) 
				Im::imNodeListEntry("Focused", currentFocus);
			else
				ImGui::Text("No focuses interact");
			TreePop();
		}


		ImGui::Checkbox("[DEBUG] Print events", &DEBUG_EVENT);
		ImGui::Checkbox("Has BG", &doClear);

		ImGui::ColorPicker4("BG Color", clearColor.ptr(), NULL);

		if (TreeNode("input dependencies")) {
			for (auto& dep : deps) {
				PushID(&dep);
				ImGui::Value("uid",dep.sc->uid); SameLine();
				ImGui::Text(dep.sc->name);
				if (Button(ICON_MD_SEARCH "inspect"))
					r2::im::NodeExplorer::edit(dep.sc);
				PopID();
			}
			TreePop();
		}
		ImGui::Unindent();
		ImGui::PopItemWidth();
	}

#ifdef _DEBUG
	Checkbox("DEBUG EVENT", &DEBUG_EVENT);
	if (TreeNode("Event Targets")) {
		for (auto& et : interacts) 
			r2::Im::nodeButton(et);
		TreePop();
	}
#endif
	if (!rd::Bits::is(nodeFlags, NF_EDITOR_HIDE_METADATA)) {
		vars.im();
	}

	Super::im();
}


void r2::EarlyDepthScene::im() {
	using namespace ImGui;
	r2::Scene::im();
	if (ImGui::CollapsingHeader(ICON_MD_NATURE " EarlyDepthScene")) {
		ImGui::PushItemWidth(60); ImGui::Indent();
		Checkbox("Skip Clip Pass (ed+opaq+cutout)", &skipClip);
		Checkbox("Skip Translucent Pass (others)", &skipBasic);
		ImGui::Unindent(); ImGui::PopItemWidth();
	}
}

void r2::Node::im() {
	using namespace ImGui;
	ImGui::PushID(this);

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Node")) {
			if (ImGui::BeginMenu("Add child")) {
				auto nc = r2::im::HierarchyExplorer::menuType();
				if (nc) addChild(nc);
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Clone")) {
				auto cl = clone();
				parent->addChild(cl);
				r2::im::NodeExplorer::edit(cl);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if(agl) {
		if (ImGui::TreeNodeEx(ICON_MD_HEALING " Components##header", ImGuiTreeNodeFlags_DefaultOpen)) {
			agl->im();
			NodeAgent* na = 0;
			if (NodeAgentFactory::imCreate("+##compsHead", na)) 
				addComponent(na);
			ImGui::TreePop();
			Separator();
		}
	}
	

	if (ImGui::CollapsingHeader(ICON_MD_ACCOUNT_TREE " Node", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PushItemWidth(60); ImGui::Indent();
		
		double zmin = rs::GfxContext::MTX_DEFAULT_ZMIN() * 400;
		double zmax = rs::GfxContext::MTX_DEFAULT_ZMAX() * 400;

		ImGui::SetNextItemWidth(200);
		ImGui::InputText("name", name);
		ImGui::SameLine();
		ImGui::Text("uid: %lu", uid); SameLine(); if (Button(ICON_FA_DICE)) uid = rs::Sys::getUID();
		ImGui::Checkbox("visible", &visible);
		SameLine();
		if (Button(ICON_MD_DELETE)) 
			rd::Garbage::trash(this);

		ImGui::SetNextItemWidth(188);
		ImGui::DragScalarN("pos", ImGuiDataType_Double, &x, 2, 1.f, nullptr, nullptr, "%0.2lf"); ImGui::SameLine();
		if (ImGui::Button(ICON_MD_UNDO)) { setPos(0.f, 0.f); z = 0.0f; };

		ImGui::DragDouble("z",&z, 0.01f, dtmin, dtmax, "%0.2lf"); 


		ImGui::PushID("rot");
		double rotMin = -PASTA_PI * 2.0;
		double rotMax = PASTA_PI * 2.0;
		ImGui::DragScalar("rotation", ImGuiDataType_Double, &rotation, 0.05f, &rotMin, &rotMax, "%0.3lf"); ImGui::SameLine();
		if (ImGui::Button(ICON_MD_UNDO)) rotation = 0.0;
		ImGui::PopID();

		ImGui::Value("size", Vector2(width(), height()));
		ImGui::PushID("scale");
		double smin = 0.0;
		double smax = 1000;
		ImGui::SetNextItemWidth(124);
		ImGui::DragScalarN("scale", ImGuiDataType_Double, &scaleX, 2, 0.25f, &smin, &smax, "%0.2lf"); ImGui::SameLine();
		if (ImGui::Button(ICON_MD_UNDO)) { scaleX = scaleY = 1.0;  }
		ImGui::SameLine();
		if (ImGui::Button("Sync To X")) scaleY = scaleX;
		ImGui::DragDouble("Alpha", &alpha, 1.0 / 255.0, 0.0, 2.0);

		if (ImGui::TreeNode("Flags")) {
#define FLAG_CHECKBOX(bitset,name,flag) {\
				bool is ## name = (bitset & (flag)) == (flag); \
				if (ImGui::Checkbox( #name, &(is ## name) )) \
					rd::Bits::toggle(bitset, flag, is ## name); \
				}

			FLAG_CHECKBOX(nodeFlags, DrawChildrenFirst, NodeFlags::NF_DRAW_CHILDREN_FIRST);
			FLAG_CHECKBOX(nodeFlags, ManualMatrix, NodeFlags::NF_MANUAL_MATRIX);
			FLAG_CHECKBOX(nodeFlags, Utility, NodeFlags::NF_UTILITY);
			FLAG_CHECKBOX(nodeFlags, OrigFromPool, NodeFlags::NF_ORIGINATES_FROM_POOL);
			FLAG_CHECKBOX(nodeFlags, SkipChildSerial, NodeFlags::NF_SKIP_CHILD_SERIALIZATION);
			FLAG_CHECKBOX(nodeFlags, CustomPooling, NodeFlags::NF_CUSTOM_POOLING);
			FLAG_CHECKBOX(nodeFlags, InPool, NodeFlags::NF_IN_POOL);
			FLAG_CHECKBOX(nodeFlags, SkipDraw, NodeFlags::NF_SKIP_DRAW);
			FLAG_CHECKBOX(nodeFlags, Culled, NodeFlags::NF_CULLED);
			FLAG_CHECKBOX(nodeFlags, SkipSerialization, NodeFlags::NF_SKIP_SERIALIZATION);
			FLAG_CHECKBOX(nodeFlags, SkipDestruction, NF_SKIP_DESTRUCTION); 
			FLAG_CHECKBOX(nodeFlags, SkipMeasuresForBounds,NF_SKIP_MEASURES_FOR_BOUNDS);
			FLAG_CHECKBOX(nodeFlags, AphaUnder0SkipsDraw, NF_ALPHA_UNDER_ZERO_SKIPS_DRAW);

			Separator();
			
			FLAG_CHECKBOX(nodeFlags, EditorProtected, NodeFlags::NF_EDITOR_PROTECT);
			FLAG_CHECKBOX(nodeFlags, EditorFrozen, NodeFlags::NF_EDITOR_FROZEN);
			FLAG_CHECKBOX(nodeFlags, EditorHideMetadata, NodeFlags::NF_EDITOR_HIDE_METADATA);
			FLAG_CHECKBOX(nodeFlags, EditorRendering, NodeFlags::NF_EDITOR_HIDE_RENDERING);

			Separator();
			FLAG_CHECKBOX(nodeFlags, User0, NodeFlags::NF_USER_FLAGS);
			FLAG_CHECKBOX(nodeFlags, User1, NodeFlags::NF_USER_FLAGS<<1);
			FLAG_CHECKBOX(nodeFlags, User2, NodeFlags::NF_USER_FLAGS<<2);
			FLAG_CHECKBOX(nodeFlags, User3, NodeFlags::NF_USER_FLAGS<<3);
#undef FLAG_CHECKBOX
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Calcs.")) {
			Scene* sc = getScene();
			Value("abs mouse", Vector2(rs::Sys::mouseX, rs::Sys::mouseY));

			{
				r2::Bounds bnd = getMyLocalBounds();
				ImGui::Text("Local Bounds");
				ImGui::Text("%0.2f,%0.2f", bnd.left(), bnd.top());
				ImGui::Text("+------------+");
				ImGui::Text("|            |");
				ImGui::Text("+------------+ %0.2f,%0.2f", bnd.right(), bnd.bottom());
				ImGui::Text("Size : %0.2f x %0.2f", bnd.width(), bnd.height());
			}
			ImGui::NewLine();
			{
				r2::Bounds bnd = getBounds(sc);
				ImGui::Text("Scene Bounds");
				ImGui::Text("%0.2f,%0.2f", bnd.left(), bnd.top());
				ImGui::Text("+------------+");
				ImGui::Text("|            |");
				ImGui::Text("+------------+ %0.2f,%0.2f", bnd.right(), bnd.bottom());
				ImGui::Text("Size : %0.2f x %0.2f", bnd.width(), bnd.height());

			}
			ImGui::NewLine();
			{
				r2::Bounds bnd = getBounds(sc);
				bnd.transform(sc->viewMatrix);
				ImGui::Text("Screen Bounds");
				ImGui::Text("%0.2f,%0.2f", bnd.left(), bnd.top());
				ImGui::Text("+------------+");
				ImGui::Text("|            |");
				ImGui::Text("+------------+ %0.2f,%0.2f", bnd.right(), bnd.bottom());
				ImGui::Text("Size : %0.2f x %0.2f", bnd.width(), bnd.height());

			}
			ImGui::TreePop();
		}

		if (TreeNode("Hidden values")) {
			DragDouble("_x", &_x);
			DragDouble("_y", &_y);
			DragDouble("_z", &_z);

			DragDouble("_scaleX", &_scaleX);
			DragDouble("_scaleY", &_scaleY);

			DragDouble("_rotation", &_rotation);

			//DragFloat("dummy0", &dummy0);
			//DragFloat("dummy1", &dummy1);
			TreePop();
		}

		static bool openPickParent = false;

		if (parent) {
			ImGui::Text("Parent : %s ", parent->name.c_str());
			SameLine();
			if (Button(to_string(parent->uid))) {
				r2::im::NodeExplorer::killAll();
				r2::im::NodeExplorer::edit(parent);
			}
			if(IsItemHovered())
				r2::Im::bounds(parent);
		}

		if (Button(ICON_MD_HEALING " Select New Parent")) {
			openPickParent = true;

			ImGui::OpenPopup("Parent Picker");
			ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
		}

		if (ImGui::TreeNode("Children")) {
			for (auto& c : children) {
				PushID(c->uid);
				ImGui::SetNextItemWidth(200);
				LabelText("name", "%s", c->name.c_str());
				SameLine();
				if (Button(to_string(c->uid))) {
					r2::im::NodeExplorer::killAll();
					r2::im::NodeExplorer::edit(c);
				}
				if( IsItemHovered()){
					r2::Im::bounds(c);
				}
				PopID();
			}
			TreePop();
		}
		if (ImGui::TreeNode("Components##embeded")) {
			NodeAgent* res = 0;
			if (NodeAgentFactory::imCreate("+##compsEmbed", res))
				addComponent(res);
			TreePop();
		}

		if (openPickParent) {
			if (ImGui::BeginPopupModal("Parent Picker", &openPickParent, 0)) {
				
				std::vector<r2::Node*> all;
				auto sc = getScene();

				std::function<void(r2::Node* _n)> f = [this, &all](r2::Node* _n) {
					all.push_back(_n);
				};
				if(sc) sc->traverse(f);

				for (auto n : all)
					if (ImGui::Selectable(n->name.empty() ? "##" : n->name.c_str())) {
						if(n == this )
							continue;

						//hmm we should probably use this to take scaling diff into account
						Vector2 z = Vector2(x, y);
						Matrix44 diff = parent->getRelativeMatrix(n);
						z = diff * z;
						Vector3 dsc = diff.getScale();

						syncAllMatrix();
						r2::Bounds abs = getBounds(sc);
						detach();
						trsDirty = true;
						n->addChild(this);
						syncAllMatrix();
						r2::Bounds nabs = getBounds(sc);
						float dx = nabs.left() - abs.left();
						float dy = nabs.top() - abs.top();
						x = z.x;
						y = z.y;
						scaleX *= dsc.x;
						scaleY *= dsc.y;
						trsDirty = true;
					}

				ImGui::EndPopup();
			}
		}
		
		ImGui::PopID();
		ImGui::Unindent(); ImGui::PopItemWidth(); 
	}

	if (!rd::Bits::is(nodeFlags, NF_EDITOR_HIDE_RENDERING) && CollapsingHeader(ICON_MD_STYLE " Filter")) {
		ImGui::Indent();
		FilterType item_current = (!filter) ? FilterType::FT_NONE : filter->type;
		ImGui::SetNextItemWidth(150);
		bool changed = ImGui::Combo("Filter##combo", (int*)(&item_current), r2::Im::filters, (int)FilterType::FT_COUNT);
		if (changed) {
			if (filter) {
				delete filter;
				filter = nullptr;
			}
			switch (item_current) {
				case FilterType::FT_NONE:			break;
				case FilterType::FT_BASE:			filter = new r2::Filter(); break;
				case FilterType::FT_COPY:			filter = new r2::filter::Layer(); break;
				case FilterType::FT_LAYER:			filter = new r2::filter::Layer(); break;
				case FilterType::FT_BLUR:			filter = new r2::filter::Blur(); break;
				case FilterType::FT_COLORMATRIX:	filter = new r2::filter::ColorMatrix(); break;
				case FilterType::FT_GLOW:			filter = new r2::filter::Glow(); break;
				case FilterType::FT_BLOOM:			filter = new r2::filter::Bloom(); break;
			}
		}

		ImGui::PushItemWidth(60);
		if (filter) filter->im();
		ImGui::PopItemWidth();
		ImGui::Unindent();
	}

	if (!rd::Bits::is(nodeFlags, NF_EDITOR_HIDE_METADATA)) {
		vars.im();
	}
	
	ImGui::PopID();
}


void r2::Filter::im() {
	ImGui::PushID("FilterBaseSettings");
	if (ImGui::Checkbox("enabled", &enabled)) {
		if (!enabled) {
			if (forFlattening) 
				delete forFlattening;
			forFlattening = 0;
			if (forFiltering)
				delete forFiltering;
			forFiltering = 0;
			ImGui::PopID();
			return;
		}
	}
	ImGui::Checkbox("Render to backbuffer", &doRenderToBackbuffer);
	ImGui::Checkbox("Take scale into account", &includeScale);
	ImGui::SetNextItemWidth(124);
	ImGui::Combo("Texture filtering##filter", (int*)(&texFilter), r2::Im::texFilters, (int)r2::TexFilter::TF_Count);
    if(ImGui::Checkbox("isSingleBuffer", &isSingleBuffer))
		invalidate();
	ImGui::SliderInt("Mode", (int*)&mode, (int)FilterMode::FM_Frozen, (int)FilterMode::FM_Dynamic, (mode == FilterMode::FM_Dynamic) ? "Dynamic" : "Static");

	ImGui::SameLine(0, 10); ImGui::Checkbox("debugFilter", &debugFilter);
	if (debugFilter) {
		ImGui::Checkbox("debugUseClearColor", &debugUseClearColor);
		if (debugUseClearColor) {
			ImGui::SetNextItemWidth(188);
			ImGui::ColorEdit4("debugClearColor", debugClearColor.ptr());
		}

		ImGui::Value("Capture View Matrix", this->cdebugView);
		ImGui::Value("Capture Proj Matrix", this->cdebugProj);
		ImGui::Value("Capture Model Matrix", this->cdebugModel);

		if (!debugCaptureData)	debugCaptureData = new Pasta::ShadedTexture();
		if (!debugFilterData)	debugFilterData = new Pasta::ShadedTexture();
		if (forFlattening) {
			ImGui::Separator();
			ImGui::Text("For flattening");
			forFlattening->im();
		}
		if (forFiltering) {
			ImGui::Separator();
			ImGui::Text("For filtering");
			forFiltering->im();
		}
	}
	ImGui::PopID();
}



void r2::filter::Blur::im() {
	using namespace ImGui;
	Super::im();
	ImGui::Separator();
	ImGui::PushID(this);
	if (enabled) {
		bool changed = false;
		PushItemWidth(124);
		if (changed|=ImGui::DragFloat2("size", size.ptr(), 0.25f, 0, 75, "%0.2f"))
			updateSize();
		if (changed |= ImGui::DragFloat("offsetScale", &offsetScale, 0.25f, 0, 16, "%0.2f"))//very large offset scales will likely cause texture size degeneration
			updateSize();
		if (changed |= ImGui::DragFloat("resolutionDivider", &resolutionDivider, 0.025f, 0.25f, 16, "%0.2f"))
			updateSize();
		PopItemWidth();
		if (ImGui::TreeNode("Blur infos")) {
			ImGui::SetNextItemWidth(124);
			ImGui::Text("Flatten Padding : %d px", flattenPadding);
			if (kH) {
				ImGui::Separator();
				ImGui::Text("Kernel Horizontal");
				kH->im();
			}
			if (kV) {
				ImGui::Separator();
				ImGui::Text("Kernel Vertical"); 
				kV->im();
			}
			ImGui::TreePop();
		}
		if (changed)
			invalidate();
	}
	ImGui::PopID();
}

void r2::Im::imElemEntry(const char* label, r2::BatchElem* be){
	if (!be) return;
	using namespace ImGui;

	PushID(be);
	if (label) {
		ImGui::Text("%s", label); SameLine();
	}
	ImGui::Text("#%lu", be->uid); SameLine();
	if (be->name.length()) {
		ImGui::Text(be->name);
		SameLine();
	}
	if (Button(ICON_MD_ZOOM_IN))
		r2::im::BatchElemExplorer::edit(be);
	PopID();
}

void r2::Im::imNodeListEntry(const char * label,Node* e) {
	if (!e) return;
	using namespace ImGui;

	PushID(e);
	BeginGroup();
	if (Button(ICON_MD_ZOOM_IN)) 
		r2::im::NodeExplorer::edit(e);
	SameLine();
	if (label) 
		ImGui::Text("%s", label); SameLine();
	ImGui::Text("#%lu", e->uid); SameLine();
	if (e->name.length()) 
		ImGui::Text(e->name);
	EndGroup();
	PopID();
}

static std::unordered_map<r::uid, ColorMatrixControl> colorPrms;

bool r2::Im::imColorMatrix(ColorMatrixControl& prms) {
	using namespace ImGui;
    auto c = ImGui::GetCurrentContext();
    bool changed = false;
	ImGui::PushID("Color Matrix Settings");

	ImGui::SetNextItemWidth(150);
	changed |= ImGui::Combo("Mode", (int*)&prms.mode, r2::Im::modeMatrix, (int)ColorMatrixMode::Count);

	ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoOptions;
	flags |= ImGuiColorEditFlags_NoAlpha;
	flags |= ImGuiColorEditFlags_NoSidePreview;
	flags |= ImGuiColorEditFlags_NoLabel;
	flags |= ImGuiColorEditFlags_NoInputs;

	Matrix44& mat = prms.mat;
	Matrix44 matSettings = Matrix44(mat);
	switch (prms.mode) {
	case ColorMatrixMode::HSV:
		ImGui::PushItemWidth(80);
		changed |= ImGui::SliderFloat("hue", &prms.hue, 0.0, 360.0, "%0.2f"); ImGui::SameLine();
		changed |= ImGui::SliderFloat("sat", &prms.sat, -2, 2.0, "%0.2f"); ImGui::SameLine();
		changed |= ImGui::SliderFloat("val", &prms.val, -2, 2.0, "%0.2f");
		ImGui::PopItemWidth();

		matSettings = Matrix44::identity;
		rd::ColorLib::colorHSV(matSettings, prms.hue, prms.sat, prms.val);
		mat = (matSettings.transpose());
		break;
	case ColorMatrixMode::Colorize:
		ImGui::Text("Tint:");
		ImGui::SetNextItemWidth(188);
		//changed |= ImGui::ColorPicker4("Tint", prms.tint.ptr(), flags, NULL);
		changed |= ImGui::DragFloat("Ratio New", &prms.ratioNew, 0.01f, 0, 2);
		changed |= ImGui::DragFloat("Ratio Old", &prms.ratioOld, 0.01f, 0, 2);
		changed |= ImGui::ColorEdit4("color", prms.tint.ptr());
		matSettings = Matrix44::identity;
		ColorLib::colorColorize(matSettings, prms.tint, prms.ratioNew, prms.ratioOld);
		mat = (matSettings.transpose());
		break;
	case ColorMatrixMode::Matrix:
		ImGui::PushItemWidth(252);
		Vector4 colorR = matSettings.getRow(0);
		Vector4 colorG = matSettings.getRow(1);
		Vector4 colorB = matSettings.getRow(2);
		Vector4 colorA = matSettings.getRow(3);
		changed |= ImGui::DragFloat4("##1", (float*)&colorR, 0.01f, -1, 2);
		changed |= ImGui::DragFloat4("##2", (float*)&colorG, 0.01f, -1, 2);
		changed |= ImGui::DragFloat4("##3", (float*)&colorB, 0.01f, -1, 2);
		changed |= ImGui::DragFloat4("##4", (float*)&colorA, 0.01f, -1, 2);
		matSettings.setRow(0, colorR);
		matSettings.setRow(1, colorG);
		matSettings.setRow(2, colorB);
		matSettings.setRow(3, colorA);
		ImGui::PopItemWidth();

		ImGui::PushItemWidth(122);
		colorR = matSettings.getCol(0);
		colorG = matSettings.getCol(1);
		colorB = matSettings.getCol(2);
		colorA = matSettings.getCol(3);
		ImGui::BeginGroup();
		ImGui::Text("Color R:");
		changed |= ImGui::ColorPicker4("ColorR", (float*)&colorR, flags, NULL);
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::BeginGroup();
		ImGui::Text("Color G:");
		changed |= ImGui::ColorPicker4("ColorG", (float*)&colorG, flags, NULL);
		ImGui::EndGroup();
		ImGui::BeginGroup();
		ImGui::Text("Color B:");
		changed |= ImGui::ColorPicker4("ColorB", (float*)&colorB, flags, NULL);
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::BeginGroup();
		ImGui::Text("Color A:");
		changed |= ImGui::ColorPicker4("ColorA", (float*)&colorA, flags, NULL);
		ImGui::EndGroup();
		matSettings.setCol(0, colorR);
		matSettings.setCol(1, colorG);
		matSettings.setCol(2, colorB);
		matSettings.setCol(3, colorA);
		ImGui::PopItemWidth();
		mat = (matSettings);
		break;
	}
	
	if (ImGui::Button("Reset")) {
		mat = (Matrix44::identity);
		prms.hue = 0.0f;
		prms.sat = 1.0f;
		prms.val = 1.0f;
		prms.ratioNew = 0.8f;
		prms.ratioOld = 0.2f;
		changed = true;
	}

	SameLine();
	if (ImGui::Button(ICON_FA_CLIPBOARD)) {
		string cc;
		if (prms.mode == ColorMatrixMode::HSV) {
			cc += "r2::ColorMatrixControl ctrl;\n";
			cc += "ctrl.mode = r2::ColorMatrixMode::HSV;\n";
			cc += "ctrl.hue = "s + std::to_string( prms.hue ) + "f;\n";
			cc += "ctrl.sat = "s + std::to_string(prms.sat)  + "f;\n";
			cc += "ctrl.val = "s + std::to_string(prms.val)  +"f;\n";
			cc += "ctrl.ratioNew = "s + std::to_string(prms.ratioNew) + "f;\n";
			cc += "ctrl.ratioOld = "s + std::to_string(prms.ratioOld) + "f;\n";
			cc += "ctrl.tint = r::Color::fromUIntRGBA( 0x"s + prms.tint.toHexString() + " );\n";
			cc += "ctrl.sync({obj});\n";
		}
		ImGui::SetClipboardText(cc.c_str());
	}
	ImGui::PopID();
	return changed;
}

bool r2::Im::imColorMatrix(r::uid id, Matrix44& mat) {
	bool exists = false;
	if (colorPrms.find(id) != colorPrms.end())
		exists = true;
	ColorMatrixControl& prms = colorPrms[id];
	if (!exists) {
		// since we can't retrieve HSV param from matrix we default to Matrix mode for now
		prms.mode = ColorMatrixMode::Matrix;
		prms.mat = mat;
	}	
	bool changed = imColorMatrix(prms);
	mat = prms.mat;
	if (prms.fresh) {
		prms.fresh = false;
		changed = true;
	}
	return changed;
}



void r2::filter::ColorMatrix::im() {
	r2::Filter::im();
	ImGui::Separator();
	
	Matrix44 & res = ctrl.mat;
	if (enabled) {
		r2::Im::imColorMatrix(ctrl);
	}
}

static float s_hue = 0;
static float s_sat = 0;
static float s_val = 0;
void r2::filter::Glow::im() {
	r2::filter::Blur::im();
	ImGui::Separator();
	ImGui::PushID(this);
	if (enabled) {
		bool changed = false;
		changed|= ImGui::Checkbox("knockout", &knockout);
		ImGui::SetNextItemWidth(124);
		changed |= ImGui::Combo("Compositing filtering##filter", (int*)(&compositingFilter), r2::Im::texFilters, (int)r2::TexFilter::TF_Count);
		changed |= ImGui::SliderInt("mode", (int*)&matrixMode, 0, 1, matrixMode ? "matrix" : "color");

		ImGui::SetNextItemWidth(200);
		changed |= ImGui::ColorPicker4("blit color", glowColorBlit.ptr());

		if (!matrixMode) {
			ImGui::SetNextItemWidth(124);
			changed |= ImGui::ColorPicker4("color", glowColor.ptr());
			changed |= ImGui::DragFloat("Intensity", &glowIntensity, 0.0f, 0.001f, 100);
		} else {
			Matrix44 mat;
			if(curNode)
				changed |= r2::Im::imColorMatrix(curNode->uid, mat);
			colorMatrix = mat;
		}
		if (changed) 
			invalidate();
	}
	ImGui::PopID();
}

void r2::filter::Bloom::im() {
	Super::im();
	ImGui::Separator();
	ImGui::PushID(this);
	ctrl.im();
	ImGui::PopID();
}

bool r2::Im::imBlendmode(r::TransparencyType& t) {
	return ImGui::Combo("blendmode", (int*)(&t), r2::Im::blends, r::TransparencyType::TRANSPARENCY_TYPE_COUNT);
}

void r2::Sprite::imBlend() {
	ImGui::Combo("blendmode", (int*)(&blendmode), r2::Im::blends, r::TransparencyType::TRANSPARENCY_TYPE_COUNT);
}


void r2::Sprite::im(){
	using namespace ImGui;
	r2::Node::im();
	if (!rd::Bits::is(nodeFlags, NF_EDITOR_HIDE_RENDERING) &&  ImGui::CollapsingHeader(ICON_MD_PALETTE " Sprite")) {
		ImGui::Indent(); ImGui::PushItemWidth(124);
			ImGui::Combo("Tex Filtering##spr", (int*)(&texFiltering), r2::Im::texFilters, (int)r2::TexFilter::TF_Count);

			ImGui::Checkbox("forceBasicPass", &forceBasicPass);
			ImGui::Checkbox("depthRead", &depthRead);
			ImGui::Checkbox("depthWrite", &depthWrite);
			ImGui::Checkbox("depthGreater", &depthGreater);
			ImGui::Checkbox("killAlpha", &killAlpha);
			if (overrideShader) {
				ImGui::SameLine();
				ImGui::Warning("Be careful, killAlpha doesn't work with overridden shaders");
			}

			ImGui::Checkbox("useSimpleZ", &useSimpleZ);
			if (!useSimpleZ) {
				ImGui::DragFloat("Z Top Offset",&zTopOffset,1,-100,100);
				ImGui::DragFloat("Z Bottom Offset",&zBottomOffset, 1, -100, 100);
			}

			ImGui::ColorPicker4("Color", color.ptr(), NULL);
			imBlend();
		ImGui::PopItemWidth(); ImGui::Unindent();
	}

	if (!rd::Bits::is(nodeFlags, NF_EDITOR_HIDE_RENDERING) && ImGui::CollapsingHeader(ICON_MD_AUTO_FIX_HIGH " Shader")) {
		ImGui::Indent();

		if (overrideShader) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
			ImGui::TextWrapped(ICON_MD_WARNING " This Sprite uses a custom shader");
			
			if (ImGui::Button(ICON_MD_WARNING " Remove")) overrideShader = nullptr;
			ImGui::PopStyleColor();
		}
		else {
			ImGui::PushItemWidth(124);
			static const char* shaderNames[] = { "Basic","Uber","MSDF" };
			Combo("Shader", (int*)&shader, shaderNames, IM_ARRAYSIZE(shaderNames));

			if (IsItemHovered()) {
				SetTooltip(ICON_MD_MESSAGE "Performances are much better than filters.\nMSDF are for texts usually");
			}

			if (shader == Shader::SH_Uber) {
				bool shaderColorMatrix = shaderFlags & USF_ColorMatrix;
				if (ImGui::Checkbox("Color matrix", &shaderColorMatrix))
					rd::Bits::toggle(shaderFlags, USF_ColorMatrix, shaderColorMatrix);

				bool shaderColorAdd = shaderFlags & USF_ColorAdd;
				if (ImGui::Checkbox("Color Add", &shaderColorAdd))
					rd::Bits::toggle(shaderFlags, USF_ColorAdd, shaderColorAdd);

				if(shaderColorAdd){
					auto an = getShaderParam("uColorAdd");
					r::Color c = !an ? r::Color(0,0,0,0) : an->asColor();
					if( ImGui::ColorPicker4("color", c.ptr()) ){
						updateShaderParam("uColorAdd", c.ptr(), 16);
					}
				}
			
				if (shaderColorMatrix) {
					Matrix44 res = Matrix44::identity;
					if (r2::Im::imColorMatrix(uid, res)) //todo replace this by anon storage or retrieve matrix as it is
						setShaderParam("uColorMatrix", res.ptr(), 4 * 4);
				}

				{
					bool shaderGlitch = shaderFlags & USF_Glitch;
					if (ImGui::Checkbox("Glitch", &shaderGlitch))
						rd::Bits::toggle(shaderFlags, USF_Glitch, shaderGlitch);
					if (shaderGlitch) {
						GlitchControl gctrl;
						gctrl.readFromShader(this);
						if (gctrl.im())
							gctrl.upload(this);
					}
				}

				{
					bool shaderVignette = shaderFlags & USF_Vignette;
					if (ImGui::Checkbox("Vignette", &shaderVignette)) {
						if (shaderVignette) {
							VignetteControl gctrl;
							gctrl.setup(this);
							gctrl.upload(this);
						}
					}
					if (shaderVignette) {
						VignetteControl gctrl;
						gctrl.readFromShader(this);
						Indent();
						if (gctrl.im()) 
							gctrl.upload(this);
						Unindent();
					}
				}

				bool shaderRGBOffset = shaderFlags & USF_RGBOffset;
				if (ImGui::Checkbox("RGB Offset", &shaderRGBOffset))
					rd::Bits::toggle(shaderFlags, USF_RGBOffset, shaderRGBOffset);

				if (shaderRGBOffset) {
					float rgbs[3 * 2] = {};
					if (hasShaderParam("uRGBOffset")) memcpy(rgbs, getShaderParam("uRGBOffset")->asFloatBuffer(), 3*2*4);
					bool touched = false;
					touched |= ImGui::DragFloat2("Red offsets", rgbs,0.001f );
					touched |= ImGui::DragFloat2("Green offsets", rgbs+2, 0.001f);
					touched |= ImGui::DragFloat2("Blue offsets", rgbs+4, 0.001f);
					if (touched) setShaderParam("uRGBOffset", rgbs, 6);
				}

				bool shaderFXAA = shaderFlags & USF_FXAA;
				if (ImGui::Checkbox("Anti-aliasing ( FXAA )", &shaderFXAA))
					rd::Bits::toggle(shaderFlags, USF_FXAA, shaderFXAA);

				bool shaderDither = shaderFlags & USF_Dither;
				const float ditherPattern[64] = {
					00.0, 32.0, 08.0, 40.0, 02.0, 34.0, 10.0, 42.0, /* 8x8 Bayer ordered dithering */
					48.0, 16.0, 56.0, 24.0, 50.0, 18.0, 58.0, 26.0, /* pattern. Each input pixel */
					12.0, 44.0, 04.0, 36.0, 14.0, 46.0, 06.0, 38.0, /* is scaled to the 0..63 range */
					60.0, 28.0, 52.0, 20.0, 62.0, 30.0, 54.0, 22.0, /* before looking in this table */
					03.0, 35.0, 11.0, 43.0, 01.0, 33.0, 09.0, 41.0, /* to determine the action. */
					51.0, 19.0, 59.0, 27.0, 49.0, 17.0, 57.0, 25.0,
					15.0, 47.0, 07.0, 39.0, 13.0, 45.0, 05.0, 37.0,
					63.0, 31.0, 55.0, 23.0, 61.0, 29.0, 53.0, 21.0 };
				if (ImGui::Checkbox("Dithering", &shaderDither)){
					rd::Bits::toggle(shaderFlags, USF_Dither, shaderDither);
					updateShaderParam("uDitherPattern", ditherPattern, 64);
				}

				bool shaderDissolve = shaderFlags & USF_Dissolve;
				float dissAmt = 0.2f;
				float dissZoom = 30.0f;
				bool useTex = shaderFlags & USF_DissolveUseTexture;
				bool renderNoise = shaderFlags & USF_ShowNoise;
				if (ImGui::Checkbox("Dissolve", &shaderDissolve)) {
					rd::Bits::toggle(shaderFlags, USF_Dissolve, shaderDissolve);
					updateShaderParam("uDissolveProgress", dissAmt);
					updateShaderParam("uDissolveZoom", dissZoom);
				}
				if (shaderDissolve) {
					if (hasShaderParam("uDissolveProgress"))
						dissAmt = getShaderParam("uDissolveProgress")->asFloat();
					if (ImGui::DragFloat("Amount", &dissAmt, 0.005f, 0.0f, 1.0f))
						updateShaderParam("uDissolveProgress", dissAmt);

					if (hasShaderParam("uDissolveZoom"))
						dissZoom = getShaderParam("uDissolveZoom")->asFloat();
					if (ImGui::DragFloat("Scale", &dissZoom, 0.1f, 0.0f, 100.0f))
						updateShaderParam("uDissolveZoom", dissZoom);

					if (ImGui::Checkbox("Show Noise", &renderNoise))
						rd::Bits::toggle(shaderFlags, USF_ShowNoise, renderNoise);
					
					if (ImGui::Checkbox("Use Texture", &useTex))
						rd::Bits::toggle(shaderFlags, USF_DissolveUseTexture, useTex);

					if (useTex) {
						if (ImGui::Button("Select Dissolve Texture")) {

							string path = "sample.png";
							vector<pair<string, string>> filters;
							filters.push_back(pair("PNG File Format (.png)", "*.png"));
							filters.push_back(pair("JPG File Format (.jpg)", "*.jpg"));
						
							if (rs::Sys::filePickForOpen(filters, path))
							{
								int i = path.end() - path.begin();
								while (path.substr(i, 4) != "res\\")
									i--;
								path = path.substr(i+4);

								Tile* tDisp = r2::Tile::fromImageFile(path.c_str());
								additionnalTexture = tDisp->getTexture();
							}
						}
					}
				}
			}
			ImGui::PopItemWidth();
		}

		if (ImGui::TreeNode("Shader values")) {
			if (!shaderValues.head){
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
				ImGui::TextWrapped(ICON_MD_WARNING " No shader values");
				ImGui::PopStyleColor();
				if (Button("Create"))
					shaderValues.head = rd::Anon::fromPool();
			}
			shaderValues.im();
			ImGui::TreePop();
		}
		ImGui::Unindent();
	}
}

void r2::Bitmap::im() {
	using namespace ImGui;
	r2::Sprite::im();
	if (!rd::Bits::is(nodeFlags, NF_EDITOR_HIDE_RENDERING) &&  ImGui::CollapsingHeader(ICON_MD_IMAGE " Bitmap")) {
		ImGui::Indent(); ImGui::PushItemWidth(124);

		if (tile&&ImGui::TreeNodeEx("Pivot", ImGuiTreeNodeFlags_DefaultOpen)) {
			Vector2 ratio2(tile->getCenterRatioX(), tile->getCenterRatioY());
			if (ImGui::DragFloat2("ratio", ratio2.ptr(), 0.01f, homogeneousMinF, homogeneousMaxF, "%0.2f")) {
				tile->dx = -ratio2.x * tile->width;
				tile->dy = -ratio2.y * tile->height;
			}

			ImGui::DragScalarN("offset in pixel", ImGuiDataType_Double, &tile->dx, 2, 1.0, &dtmin, &dtmax, "%0.2f");
			ImGui::SameLine();
			if (ImGui::Button(ICON_MD_UNDO))
				tile->dx = tile->dy = 0;

			ImGui::TreePop();
		}

		if (tile && ImGui::TreeNode(ICON_MD_IMAGE " Tile")) {
			if (ImGui::Button("Pick tile"))
				r2::im::TilePicker::forBitmap(this);

			if (tile) tile->im(true,false);

			Str p;
			if (r2::Im::filePicker("Pick any file",p)) {
				auto resPath = rd::String::skip(p.c_str(), "res/");
				auto d = RscLib::getTextureData( resPath );
				if (d) {
					setTile(r2::Tile::fromTextureData(d),true);
					vars.set("r2::bitmap::path", resPath);
				}
			}

			ImGui::TreePop();
		}
		
		ImGui::PopItemWidth(); 
		ImGui::Unindent();
	}
}

void rd::ABitmap::im() {
	using namespace ImGui;
	static bool aBitmapLoop = true;
	r2::Sprite::im();
	if (ImGui::CollapsingHeader(ICON_MD_DIRECTIONS_RUN " Animated Bitmap")) {
		ImGui::PushID("ABitmapSettings");
		ImGui::Indent(); ImGui::PushItemWidth(124);
		
			auto anm = player.getCurrentAnim();
			if (anm) 
				aBitmapLoop = anm->plays == -1;

			if (aBitmapLoop) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
			} else {
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Button));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			}

			if (ImGui::Button(ICON_MD_LOOP)) {
				aBitmapLoop = !aBitmapLoop;
				if (anm) anm->plays = aBitmapLoop ? -1 : 1;
			}

			ImGui::PopStyleColor(2);
			ImGui::SameLine();

			if (player.isPlaying) {
				if(ImGui::Button(ICON_MD_PAUSE))
					player.pause();
			} else {
				if (ImGui::Button(ICON_MD_PLAY_ARROW)) {
					if (anm)
						player.resume();
					else
						replay();
					if (aBitmapLoop)
						player.loop();
					else if(anm) anm->plays = 1;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(ICON_MD_STOP)) 
				stop();

			if (anm) {
				ImGui::SameLine();
				if (ImGui::Button(ICON_MD_SKIP_PREVIOUS)) {
					player.pause();
					anm->cursor--;
					player.syncCursorToSpr();
				}
				ImGui::SameLine();
				if (ImGui::Button(ICON_MD_SKIP_NEXT)) {
					player.pause();
					anm->cursor++;
					player.syncCursorToSpr();
				}
			}

			if (ImGui::DragFloat2("Pivot - ratio", (float*)&pivotX, 0.1f, homogeneousMinF, homogeneousMaxF, "%0.2f")) 
				setCenterRatio(pivotX, pivotY);
			ImGui::SameLine();
			if (ImGui::Button(ICON_MD_UNDO))
				setCenterRatio(0, 0);
		
			double frMin = 1;
			double frMax = 60;
			ImGui::SetNextItemWidth(60);
			ImGui::DragScalar("Frame Rate", ImGuiDataType_Double, &player.frameRate, 0.25, &frMin, &frMax, "%6.3lf");

			if (lib)
				ImGui::Text((std::string("Library: ") + lib->name).c_str());

			Value("group", groupName);

			if(!groupName.empty()&&!anm){
				ImGui::Warning("No anim named "s + groupName);
			}

			if (anm) {
				if (anm->groupName.length())
					ImGui::Text( Str64f("Tile Group: %s", anm->groupName) );
				else if(anm->groupData)
					ImGui::Text( Str64f("Tile Group: %s", anm->groupData->id ));
				double spMin = -1;
				double spMax = 4;
				ImGui::SetNextItemWidth(60);
				ImGui::DragScalar("Play Speed", ImGuiDataType_Double, &player.speed, 0.25, &spMin, &spMax, "%6.3lf");
				ImGui::Text("Current frame number %i", player.getFrameNumber());
				if(TreeNode("internal player")){
					player.im();
					TreePop();
				}
			}

			if (ImGui::Checkbox("flip X", &flippedX)) {
				replay();
				if (aBitmapLoop) player.loop();
			}
			ImGui::SameLine(); 
			if (ImGui::Checkbox("flip Y", &flippedY)) {
				replay();
				if (aBitmapLoop) player.loop();
			}

			if (ImGui::TreeNode(ICON_MD_IMAGE " Tile")) {
				if (ImGui::Button("Change Tile")) {
					if (!this->tile) {
						this->setTile(rd::Pools::tiles.alloc());
						this->ownsTile = true;
					}
					Promise* p = r2::im::TilePicker::forPicker();
					p->then([this](auto p, auto data) {
						int brk = 0;
						auto tp = std::any_cast<r2::im::TilePicker*>(data);
						if (tp) {
							set(tp->pickedLib);
							playAndLoop(tp->pickedGroup->id);
						}
						return data;
					});
				};
				if (tile) tile->im();
				ImGui::TreePop();
			}
		ImGui::PopItemWidth(); ImGui::Unindent();
		ImGui::PopID();
	}
}


void r2::Scissor::im() {

	if (ImGui::CollapsingHeader(ICON_FA_CUT " Scissors")) {
		ImGui::Indent();
		ImGui::DragDouble("Width", &rect.width, 1, 0, 2000);
		ImGui::DragDouble("Height", &rect.height, 1, 0, 2000);
		ImGui::DragDouble("X", &rect.x, 1, 0, 2000);
		ImGui::DragDouble("Y", &rect.y, 1, 0, 2000);
		if (ImGui::Button("make Rect")) {
			Pasta::Graphic* gfx = Pasta::Graphic::getMainGraphic();
			rs::GfxContext g(gfx);
			drawRec(&g);
		}
			

	}
	r2::Node::im();

}

void r2::Batch::im() {
	using namespace ImGui;
	r2::Sprite::im();

	if (CollapsingHeader(ICON_MD_COLLECTIONS " Batch")) {
		Indent();
		ImGui::Text("Number of elements: %d", nbElems);
		ImGui::Text("Number of submit: %d",	nbSubmit);
		Indent();
		int i = 0;
		for (BufferCache cache : bufCache) {
			ImGui::Text("Submit " + to_string(i++) + " fbuf size : " + to_string(cache.fbuf.size()));
		}
		Unindent();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 4));
		if (ImGui::Button(ICON_MD_ADD)) {
			alloc();
		} ImGui::SameLine();
		if (ImGui::Button(ICON_MD_DELETE_SWEEP)) {
			destroyAllElements();
		}
		ImGui::PopStyleVar(1);

		if (ImGui::TreeNode("BatchElem list")) {
			r2::BatchElem * cur = head;
			while(cur){
				PushID(cur);
				BeginGroup();
				BulletText("x:%6.2f  y:%6.2f z:%6.2f", cur->x, cur->y, cur->z);
				SameLine();
				SetCursorPosY(ImGui::GetCursorPosY() - 2);
				PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 4));
				if (ImGui::Button(cur->visible ? ICON_MD_VISIBILITY : ICON_MD_VISIBILITY_OFF))
					cur->visible = !cur->visible;
				SameLine();

				if (ImGui::Button(ICON_MD_IMAGE_SEARCH))
					r2::im::BatchElemExplorer::edit(cur);

				SameLine();
				if (ImGui::Button(ICON_MD_ARROW_DROP_UP))
					cur->setPriority(cur->priority + 1);
				SameLine();
				if (ImGui::Button(ICON_MD_ARROW_DROP_DOWN))
					cur->setPriority(cur->priority - 1);
				
				ImGui::PopStyleVar(1);
				EndGroup();
				if (ImGui::IsItemHovered()) {
					r2::Im::bounds(cur);
					std::string label;
					label = "prio:" + to_string(cur->priority);
					auto asAbe = dynamic_cast<rd::ABatchElem*>(cur);
					if (asAbe) {
						label += " group:" + asAbe->groupName.cpp_str();
						if (asAbe->getLib()) label += " lib:" + asAbe->getLib()->name;
					}
					SetTooltip(label);
				}
				ImGui::PopID();
				cur = cur->next;
			}
			ImGui::TreePop();
		}
		ImGui::Unindent();
	}
}

void PushStyle(bool cond) {
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1,4));
	if (cond) {
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Button));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
	}
}
void PopStyle() {
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar();
}

void r2::Text::im() {
	using namespace ImGui;
	r2::Batch::im();
	
	if (!fnt)return;

	if (ImGui::CollapsingHeader(ICON_MD_TEXT_FIELDS " Text")) {
		ImGui::Indent();

		textIm();
		
		ImGui::Unindent();

		
	}
}

void r2::Text::textIm(){
	using namespace ImGui;
	ImGui::SetNextItemWidth(150);
	if (ImGui::BeginCombo("Font", fnt->getResourceName().c_str())) {
		rd::FontManager& fntMgr = rd::FontManager::get();
		std::vector<pair<Str, rd::Font*>>fonts(fntMgr.map.begin(), fntMgr.map.end());
		sort(fonts.begin(), fonts.end(), [](auto& p0, auto& p1) {
			return p0.first < p1.first;
		});
		for (auto fntPair : fonts) {
			if (ImGui::Selectable(fntPair.second->getResourceName().c_str()))
				setFont(fntPair.second);
			if (fntPair.second == fnt) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::Text("Text:");
	if( ImGui::InputText("Translation Key", translationKey) )
		tryTranslate();
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	if (ImGui::InputTextMultiline("##textEdit", text, ImVec2(0, ImGui::GetTextLineHeight() * 5))) {
		//for rich texts we have to use setText and not other fancy stuff
		setText(text);
	}

	ImGui::Text("Alignement:");
	static int mode = 0;
	PushStyle((blockAlign & (ALIGN_HCENTER | ALIGN_RIGHT)) == ALIGN_LEFT);
	if (ImGui::Button(ICON_MD_ALIGN_HORIZONTAL_LEFT)) { blockAlign &= ~(ALIGN_RIGHT | ALIGN_HCENTER); cache(); } ImGui::SameLine(); PopStyle();
	PushStyle((blockAlign & ALIGN_HCENTER) == ALIGN_HCENTER);
	if (ImGui::Button(ICON_MD_ALIGN_HORIZONTAL_CENTER)) { blockAlign |= ALIGN_HCENTER; blockAlign &= ~ALIGN_RIGHT; cache(); } ImGui::SameLine(); PopStyle();
	PushStyle((blockAlign & ALIGN_RIGHT) == ALIGN_RIGHT);
	if (ImGui::Button(ICON_MD_ALIGN_HORIZONTAL_RIGHT)) { blockAlign |= ALIGN_RIGHT; blockAlign &= ~ALIGN_HCENTER; cache(); } ImGui::SameLine(); PopStyle();
	ImGui::Dummy(ImVec2(20, 0)); ImGui::SameLine();
	PushStyle((blockAlign & (ALIGN_VCENTER | ALIGN_BOTTOM)) == ALIGN_TOP);
	if (ImGui::Button(ICON_MD_ALIGN_VERTICAL_TOP)) { blockAlign &= ~(ALIGN_BOTTOM | ALIGN_VCENTER); cache(); } ImGui::SameLine(); PopStyle();
	PushStyle((blockAlign & ALIGN_VCENTER) == ALIGN_VCENTER);
	if (ImGui::Button(ICON_MD_ALIGN_VERTICAL_CENTER)) { blockAlign |= ALIGN_VCENTER; blockAlign &= ~ALIGN_BOTTOM; cache(); } ImGui::SameLine(); PopStyle();
	PushStyle((blockAlign & ALIGN_BOTTOM) == ALIGN_BOTTOM);
	if (ImGui::Button(ICON_MD_ALIGN_VERTICAL_BOTTOM)) { blockAlign |= ALIGN_BOTTOM; blockAlign &= ~ALIGN_VCENTER; cache(); } PopStyle();

	ImGui::PushItemWidth(60);
	ImGui::PushID("colors");

	ImGui::Text("Base Color:");
	ImGui::PushID(&colors[0]);
	bool changed = false;
	ImGui::SetNextItemWidth(209);
	changed |= ImGui::ColorEdit3("color", colors[0].col.ptr());
	changed |= ImGui::DragFloat("alpha", &colors[0].col.a, 0.05f, 0, 2.0f);
	if (changed)
		cache();
	ImGui::PopID();

	float fontSize = getFontSize();
	bool autoSize = autoSizeTarget != nullopt;
	if (DragFloat("Font Size", &fontSize, 1.0, 0.0, 200)) {
		setFontSize(fontSize);
	}

	bool autoSizeChanged = Checkbox("autoSize", &autoSize);
	if (autoSize) {
		Indent();
		if (!autoSizeTarget)
			autoSizeTarget = fontSize;
		bool changed = DragInt("autoSize Target", &(*autoSizeTarget), 8, 256);
		if (!autoSizeWidth)
			autoSizeWidth = width();
		changed |= DragInt("autoSize Max Width", &(*autoSizeWidth), 8, 1024);
		if (changed)
			autosize(*autoSizeWidth, *autoSizeTarget);
		Unindent();
	}
	else {
		autoSizeTarget = nullopt;
		autoSizeWidth = nullopt;
	}

	auto ib = italicBend;
	if (DragInt("italic", ib)) {
		setItalicBend(italicBend = ib);
	}

	if (DragInt("Max Line Width (px at 100% scale) ", &originalMaxLineWidth, 1, -1, 1024)) {
		setMaxLineWidth(originalMaxLineWidth);
		cache();
	}
	Value("Max Effective line width", maxLineWidth);

	if (ImGui::TreeNodeEx("Color Ranges")) {

		for (int i = 1; i < colors.size(); i++) {
			ImGui::PushID(&colors[i]);
			ImGui::Spacing();
			ImGui::Text("Range %d", i);
			ImGui::SameLine();
			if (ImGui::Button(ICON_MD_DELETE)) {
				colors.erase(colors.begin() + i);
				ImGui::PopID();
				break;
			}
			bool changed = false;
			changed |= ImGui::SliderInt("start", &colors[i].start, 0, text.length()); ImGui::SameLine();
			changed |= ImGui::SliderInt("end", &colors[i].end, -1, text.length());
			ImGui::SetNextItemWidth(209);
			changed |= ImGui::ColorEdit3("color", colors[i].col.ptr());
			changed |= ImGui::SliderFloat("alpha", &colors[i].col.a, 0, 2.0); ImGui::SameLine();
			changed |= ImGui::Checkbox("multiply", &colors[i].multiply);
			if (changed)
				cache();
			ImGui::PopID();
			ImGui::Separator();
		}

		if (ImGui::Button(ICON_MD_ADD)) {
			addTextColor(r::Color(), 0, -1);
		}
		ImGui::TreePop();
	}
	ImGui::PopID();

	ImGui::PushID("ds");
	if (dropShadow) {
		if (ImGui::TreeNodeEx("Drop Shadow", ImGuiTreeNodeFlags_DefaultOpen)) {
			bool changed = false;
			ImGui::SetNextItemWidth(124);
			changed |= ImGui::SliderFloat2("dx/dy", &dropShadow->dx, 0, 16);
			ImGui::SetNextItemWidth(209);
			changed |= ImGui::ColorEdit3("color", dropShadow->col.ptr());
			changed |= ImGui::SliderFloat("alpha", &dropShadow->col.a, 0, 2.0f);
			if (ImGui::Button("Remove drop shadow")) {
				dropShadow = nullopt;
				changed = true;
			}
			if (changed)
				cache();
			ImGui::TreePop();
		}
	}
	else {
		if (ImGui::Button("Add drop shadow")) {
			addDropShadow(1, 1, Color(0, 0, 0, 1));
		}
	}
	ImGui::PopID();

	ImGui::PushID("ol");
	if (outline) {
		if (ImGui::TreeNodeEx("Outline", ImGuiTreeNodeFlags_DefaultOpen)) {
			bool changed = false;
			ImGui::SetNextItemWidth(209);
			changed |= ImGui::ColorEdit3("color", outline->col.ptr());
			changed |= ImGui::SliderFloat("alpha", &outline->col.a, 0, 2.0f);
			changed |= ImGui::DragFloat("delta", &outline->delta, 0.1f, 0, 2.5f);
			if (ImGui::Button("Remove outline")) {
				outline = nullopt;
				changed = true;
			}
			if (changed)
				cache();
			ImGui::TreePop();
		}
	}
	else {
		if (ImGui::Button("Add outline"))
			addOutline(Color(0, 0, 0, 1));
	}
	if (ImGui::Button("[DBG]cache"))
		cache();

	if (TreeNode("elInfos")) {
		for (auto& e : elInfos) {
			if (TreeNode(std::to_string(e.first))) {
				Value("uid", e.first);
				Value("charcode", e.second.charcode);
				Value("lineIdx", e.second.lineIdx);
				TreePop();
			}
			if (IsItemHovered()) {
				auto el = getElementByUID(e.first);
				r2::Im::bounds(el, 2);
			}
		}
		TreePop();
	}

	if (TreeNode("lineInfos")) {
		int idx = 0;
		for (auto& l : lineInfos) {
			Value("idx", idx);
			Value("width", l.width);
			Value("bottom", l.bottom);
			idx++;
		}
		TreePop();
	}
	ImGui::PopID();
	ImGui::PopItemWidth();
}

static int StrResizeCallback(ImGuiInputTextCallbackData* data) {
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
		rd::Anon * my_str = (rd::Anon *)data->UserData;
		int neededSize = (data->BufSize <= 0) ? 0 : data->BufSize;
		if (my_str->getByteSizeCapacity() < neededSize)
			my_str->reserve(neededSize + 1); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
		data->Buf = (char*)my_str->data;
	}
	return 0;
};

bool rd::Anon::im(rd::Anon *& selfref) {
	bool changed = false;
	ImGui::PushID(this);

	ImGui::SetNextItemWidth(-1);
	if (ImGui::InputText("##name", name)) {
		//holy shit we are not ordered any more...
		changed = true;
	}
	ImGui::NextColumn();

	ImGui::SetNextItemWidth(-1);
	switch(type) {
		case AType::AVoid:
			break;
		case AType::AFloat:
			changed |= ImGui::DragFloat("##val", (float*)&data, 1,-1000, 1000);
			break;
		case AType::AInt:
			changed |= ImGui::DragInt("##val", (int*)&data, 1,-1000, 1000);
			break;

		case AType::AInt64: {
			s64 smin = -1000LL;
			s64 smax = 1000LL;
			changed |= ImGui::DragScalarN("##val", ImGuiDataType_S64, &reinterpret_cast<r::s64&>(data), 1,1.0, &smin,&smax);
			break;
		}

		case AType::AUInt64: {
			u64 umin = 0ULL;
			u64 umax = 1000ULL;
			changed |= ImGui::DragScalarN("##val", ImGuiDataType_U64, &reinterpret_cast<r::u64&>(data), 1, 1.0, &umin, &umax);
			break;
		}

		case AType::AFloatBuffer: {
			switch(typeEx) {
				case ATypeEx::AVec2:
					changed |= ImGui::DragFloat2("##val", (float*)data, 1, -1000, 1000);
					break;
				case ATypeEx::AVec3:
					changed |= ImGui::DragFloat3("##val", (float*)data, 1, -1000, 1000);
					break;
				case ATypeEx::AVec4:
					changed |= ImGui::DragFloat4("##val", (float*)data, 1, -1000, 1000);
					break;
				case ATypeEx::AColor:
					if (flags&AFL_IMGUI_CHANGING_COLOR) {
						changed |= ImGui::ColorPicker4("##val", (float*)data);
						if (ImGui::Button(ICON_MD_CLOSE)) 
							flags &= ~AFL_IMGUI_CHANGING_COLOR;
					}
					else {
						r::Color c = *(r::Color*)data;
						if (ImGui::ColorButton("##val", (ImVec4)c))//wild thing i love you
							flags |= AFL_IMGUI_CHANGING_COLOR;
					}
					break;
				default:
					switch (getSize())
					{
						case 1: changed |= ImGui::DragFloat("##val", (float*)data,		1, -1000, 1000); break;
						case 2:	changed |= ImGui::DragFloat2("##val", (float*)data,	1, -1000, 1000); break;
						case 3:	changed |= ImGui::DragFloat3("##val", (float*)data,	1, -1000, 1000); break;
						case 4: changed |= ImGui::DragFloat4("##val", (float*)data,	1, -1000, 1000); break;
						default: {
							float mn = -1000;
							float mx = -1000;
							changed |= ImGui::DragScalarN("##val", ImGuiDataType_Float, (float*)data, getSize(), 1, &mn, &mx, "%0.1f", 0);
							break;
						}
					};
					break;
			}
			break;
		}

		case AType::AByteBuffer: {
			auto crc = Checksum::CRC32(data, byteSize);
			ImGui::Text("CRC32:%s", to_string(crc).c_str());
			break;
		}
		case AType::AString: {
			changed |= ImGui::InputText("##val", (char*)data, byteSize, ImGuiInputTextFlags_CallbackResize, StrResizeCallback, this);
			break;
		}
		
		default:
			break;
	}
	ImGui::NextColumn();

	switch(type){
		case AType::AVoid: ImGui::Text("void"); break;
		case AType::AFloat: ImGui::Text("float"); break;
		case AType::AInt: ImGui::Text("int"); break;
		case AType::AInt64: ImGui::Text("int64"); break;
		case AType::AUInt64: ImGui::Text("uint64"); break;
		case AType::AString: ImGui::Text("string"); break;

		case AType::AFloatBuffer: {
			switch( typeEx ) {
				case ATypeEx::AVec2: ImGui::Text("vec2"); break;
				case ATypeEx::AVec3: ImGui::Text("vec3"); break;
				case ATypeEx::AVec4: ImGui::Text("vec4"); break;
				case ATypeEx::AColor: ImGui::Text("color"); break;
				default:
					ImGui::Text("float arr.");
					break;
			}
			break;
		}
		default: ImGui::Text("unsupported"); break;

		//default:
		//	ImGui::Text("[Todo]Cannod display value");
		//	break;
	}

	//remvoe because in cas of large indent values was causing hiding
	//ImGui::SameLine(60);
	ImGui::SameLine();
	//change type
	ImGui::Button(ICON_MD_SETTINGS);
	r2::Im::anonContextMenu(this);

	ImGui::SameLine();
	auto sib = sibling;
	if (ImGui::Button(ICON_MD_DELETE)) {
		selfref = destroy();
		ImGui::NextColumn();
		ImGui::PopID();
		changed = true;
		return changed;
	}

	ImGui::NextColumn();
	ImGui::PopID();

	if (sib) {
		changed |= sib->im(sib);
		sibling = sib;
	}
	else {
		ImGui::Button(ICON_MD_ADD);
		rd::Anon* anon = r2::Im::anonContextMenu(nullptr);
		if (anon) {
			sib = anon;
			sibling = sib;
		}
	}
	return changed;
}

void r2::Tile::smallPreview() {
	using namespace ImGui;
	int w = width;
	int h = height;
	if (w > ImGui::GetWindowWidth()) {
		w = ImGui::GetWindowWidth() * 0.25;
		h = height * (w / width);
	}
	ImGui::Image(this, ImVec2(w, h));
	if (debugName&& IsItemHovered()) {
		SetTooltip(debugName);
	}
}

bool r2::Tile::im(bool preview, bool pick){
	using namespace ImGui;

	bool changed = false;
	ImGui::PushID(this);
	ImGui::Text("Tile info:");
	Indent();

	auto op = ImGuiTreeNodeFlags_DefaultOpen;
	if (!preview)
		op = {};
	if (TreeNodeEx("Preview",op)) {
		if (debugName)
			Value("debug name",StrRef(debugName));
		int w = width;
		int h = height;
		if( w > ImGui::GetWindowWidth()){
			w = ImGui::GetWindowWidth() * 0.25;
			h = height * ( w / width);
		}
		ImGui::Image(this, ImVec2(w, h));
		TreePop();
	}
	if (tex) {
		try {
			if(tex->getPath())
				ImGui::Text("tex  name   : %s", tex->getPath());
			if (tex->getDebugName())
				ImGui::Text("tex  dname  : %s", tex->getDebugName());
			ImGui::Text("tex  id     : 0x%x", tex->getResourceId());
			ImGui::Value("tex  size", Vector2i(tex->getWidth(), tex->getHeight()));
			ImGui::Value("tile size", Vector2i(width, height));
		}
		catch(...){
			ImGui::Error("INVALID Texture");
		}
	}
	else {
		ImGui::Error("No Texture !!!");
	}

	ImGui::Value("uid", uid);
	ImGui::PushItemWidth(124);

	ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal); // Prevent ImGuiSeparatorFlags_SpanAllColumns
	if (ImGui::Button("Flip X")) {	flipX(); changed = true;} ImGui::SameLine();
	if (ImGui::Button("Flip Y")) { flipY(); changed = true; }
	if (pick && ImGui::Button("Pick tile")) {
		r2::im::TilePicker::forTile(*this);
		changed = true;//not a good solution as tile fetching might be deferred but anw...
	}
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal); // Prevent ImGuiSeparatorFlags_SpanAllColumns

	r::Vector2 pivot = r::Vector2(getCenterRatioX(), getCenterRatioY());
	if( ImGui::DragFloat2("pivot - ratio ()", pivot.ptr(),0.1f,-4.0f,4.0f ))
		setCenterRatio(pivot.x, pivot.y);
	changed |= ImGui::DragDouble2("pivot - pixel", &dx, 0.1, -100, 100, "%0.3lf");
	changed |= ImGui::DragDouble2("xy", &x, 0.1, -100, 100, "%0.3lf");
	changed |= ImGui::DragDouble2("size", &width, 0.1, -100, 100, "%0.3lf");
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

	ImGui::PopItemWidth();
	ImGui::SetNextItemWidth(252);
	changed |= ImGui::DragDouble4("uv", &u1, 0.1, -100, 100, "%.3lf");
	ImGui::PopID();
	Unindent();
	return changed;
}

void r2::BloomCtrl::im() {
	using namespace ImGui;
	PushID(this);
	Checkbox("doPad", &doPad);

	Combo("Blur Tex Filtering##bloom_blur", (int*)(&blurFilter), r2::Im::texFilters, (int)r2::TexFilter::TF_Count);
	SameLine();
	Combo("Render Tex Filtering##bloom_render", (int*)(&renderFilter), r2::Im::texFilters, (int)r2::TexFilter::TF_Count);

	Checkbox("skipBloomPass", &skipBloomPass); ImGui::SameLine();
	Checkbox("skipColorPass", &skipColorPass);

	PushItemWidth(124);
	DragFloat2("size", &dx, 0.1f, 0.0f, 80.0f);
	DragFloat("offsetScale", &offsetScale, 0.1f, 0.0f, 100.0f);
	DragFloat("pass", &pass, 0.01f, 0.0f, 2.0f);
	PopItemWidth();

	SetNextItemWidth(276);

	ColorEdit4("mul", mul.ptr());
	DragFloat("intensity", &intensity, 0.01f, 0.0, 256.0f);
	SetNextItemWidth(188);
	DragFloat3("luminanceVector ", luminanceVector.ptr(), 0.05f, -2.2f, 2.2f);
	if (Button("neutral"))	neutral();
	if (Button("fast"))		fast();
	if (Button("abused"))	abused();
	ImGui::PopID();
}

void r2::svc::GaussianBlurCtrl::im() {
	using namespace ImGui;
	PushID(this);
	if (CollapsingHeader("GaussianBlur Ctrl")) {
		Indent();
		Combo("Tex Filtering##ctrl_gauss", (int*)(&filter), r2::Im::texFilters, (int)r2::TexFilter::TF_Count);
		Checkbox("doPad", &doPad);
		SetNextItemWidth(124);
		DragFloat2("size", this->size.ptr(), 0.1f, 0.0f, 80.0f);
		DragFloat("offsetScale", offsetScale.ptr(), 0.1f, 0.0f, 100.0f);
		Unindent();
	}
	PopID();
}
