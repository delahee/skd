#include "stdafx.h"
#include "Interp.hpp"
#include "rd/JSerialize.hpp"
#include "4-menus/imgui_internal.h"

using namespace std;

float rd::ext::CurveC1::get(int idx) {
	return data[std::clamp<int>(idx, 0, data.size() - 1)];
}

float rd::ext::CurveC1::plot(float t) {
	if (!data.size())
		return 0.0;

	int l = (data.size() - 1);
	float fidx = (t * l);
	int idx = (int)(t * l);
	float f = fidx - idx;
	float pm1 = get(idx - 1);
	float p0 = get(idx);
	float p1 = get(idx + 1);
	float p2 = get(idx + 2);
	return rd::ext::Interp::catmull( pm1, p0, p1, p2, f );
}

void rd::ext::CurveC1::load(const char* folder, const char* file) {
	rs::jDeserialize(data, folder, file, "data");
}

void rd::ext::CurveC1::save(const char* folder, const char* file) {
	rs::jSerialize(data, folder, file, "data");
}

bool rd::ext::CurveC1::im() {
	using namespace ImGui;
	bool changed = false;
	Text("Curve Editor");
	if (!data.size()) {
		Text("No Vertices");
	}

	int idx = 0;
	for (auto& f : data) {
		Text("%d ", idx);
		PushID(idx);
		SameLine();
		changed|=DragFloat("val", &f, 0.01f);
		SameLine();

		if (Button(ICON_MD_DELETE)) {
			data.erase(data.begin() + idx);
			PopID();
			break;
		}
		PopID();
		idx++;
	}

	if (Button(ICON_MD_ADD)) {
		float toInsert;
		if (data.size() == 0) {
			toInsert = 0;
			data.push_back(toInsert);
		}
		else if (data.size() == 1) {
			toInsert = 1;
			data.push_back(toInsert);
		}
		else {
			int idxMin = 0;
			int idxMax = data.size() - 1;

			int idxMid = (idxMin + idxMax) >> 1;
			int idxMidN = idxMid + 1;
			if (idxMidN >= idxMax) idxMidN = idxMax;
			toInsert = 0.5f * (data[idxMid] + data[idxMidN]);
			data.insert(data.begin() + idxMidN, toInsert);
		}
		changed=true;
	}

	//see https://github.com/ocornut/imgui/issues/786
	if (data.empty()) return changed;

	const char* label = "myCurveView";
	const ImGuiStyle& Style = GetStyle();
	const ImGuiIO& IO = GetIO();
	ImDrawList* DrawList = GetWindowDrawList();
	ImGuiWindow* Window = GetCurrentWindow();

	const float avail = GetContentRegionAvailWidth();
	const float dim = ImMin(avail, 128.f);
	ImVec2 canvas(dim, dim);
	ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + canvas);

	ItemSize(bb);
	if (!ItemAdd(bb, NULL))
		return changed;

	RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg, 1), true, Style.FrameRounding);

	// background grid
	for (int i = 0; i <= canvas.x; i += (canvas.x / 4)) {
		DrawList->AddLine(
			ImVec2(bb.Min.x + i, bb.Min.y),
			ImVec2(bb.Min.x + i, bb.Max.y),
			GetColorU32(ImGuiCol_TextDisabled));
	}
	for (int i = 0; i <= canvas.y; i += (canvas.y / 4)) {
		DrawList->AddLine(
			ImVec2(bb.Min.x, bb.Min.y + i),
			ImVec2(bb.Max.x, bb.Min.y + i),
			GetColorU32(ImGuiCol_TextDisabled));
	}
	DrawList->AddText(ImVec2(bb.Min.x, bb.Max.y), GetColorU32(ImGuiCol_TextDisabled), "0");
	DrawList->AddText(ImVec2(bb.Max.x, bb.Max.y), GetColorU32(ImGuiCol_TextDisabled), "1");

	cout << "**********************" << endl;
	int nb = 128;
	for (int i = 0; i < nb; ++i) {
		float ratioP = (float)i / nb;
		float ratioN = (float)(i + 1) / nb;
		float v0 = plot(ratioP);
		float v1 = plot(ratioN);
		ImVec2 a(bb.Min.x + (bb.Max.x - bb.Min.x) * ratioP, bb.Min.y + v0 * (bb.Max.y - bb.Min.y));
		ImVec2 b(bb.Min.x + (bb.Max.x - bb.Min.x) * ratioN, bb.Min.y + v1 * (bb.Max.y - bb.Min.y));
		DrawList->AddLine(
			a,
			b,
			0xFF0000ff);
	}
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 12);
	return changed;
}

const Vector3 & rd::ext::CurveC3::get(int idx) {
	return data[std::clamp<int>(idx,0, data.size() - 1)];
}

Vector3 rd::ext::CurveC3::plot(float t){
	int l = (data.size()-1);
	float fidx = (t * l);
	int idx = (int) (t * l);
	float f = fidx - idx;

	const auto& pm1 = get(idx - 1);
	const auto& p0 = get(idx);
	const auto& p1 = get(idx + 1);
	const auto& p2 = get(idx + 2);

	return rd::ext::Interp::c3(pm1,p0,p1,p2,f);
}

void rd::ext::CurveC3::load(const char* folder, const char* file){
	rs::jDeserialize(data, folder, file,"data");
}

void rd::ext::CurveC3::save(const char* folder, const char* file) {
	rs::jSerialize(data, folder, file,"data");
}

bool rd::ext::CurveC3::im(){
	using namespace ImGui;
	bool changed = false;

	Text("Curve Editor");
	if (!data.size()) {
		Text("No Vertices");
	}

	int idx = 0;
	for (auto& f : data) {
		Text("%d ",idx);
		PushID(idx);
		SameLine();
		changed |= DragFloat3("val", f.ptr(),0.01f);
		SameLine();

		if (Button(ICON_MD_DELETE)) {
			data.erase(data.begin() + idx);
			PopID();
			changed = true;
			break;
		}
		PopID();
		idx++;
		
	}
	
	if (Button(ICON_MD_ADD)) {
		Vector3 toInsert;
		if (data.size() == 0) {
			toInsert = Vector3(0, 0, 0);
			data.push_back(toInsert);
		}
		else if (data.size() == 1) {
			toInsert = Vector3(1, 1, 1);
			data.push_back(toInsert);
		}
		else {
			int idxMin = 0;
			int idxMax = data.size() - 1;

			int idxMid = (idxMin + idxMax) >> 1;
			int idxMidN = idxMid + 1;
			if (idxMidN >= idxMax) idxMidN = idxMax;
			toInsert = 0.5f*(data[idxMid] + data[idxMidN]);
			data.insert( data.begin()+idxMidN, toInsert);
		}
		changed = true; 
	}

	//see https://github.com/ocornut/imgui/issues/786
	if (data.empty()) return changed;

	const char* label = "myCurveView";
	const ImGuiStyle& Style = GetStyle();
	const ImGuiIO& IO = GetIO();
	ImDrawList* DrawList = GetWindowDrawList();
	ImGuiWindow* Window = GetCurrentWindow();

	const float avail = GetContentRegionAvailWidth();
	const float dim = ImMin(avail, 128.f);
	ImVec2 canvas(dim, dim);
	ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + canvas);

	ItemSize(bb);
	if (!ItemAdd(bb, NULL))
		return changed;

	RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg, 1), true, Style.FrameRounding);

	// background grid
	for (int i = 0; i <= canvas.x; i += (canvas.x / 4)) {
		DrawList->AddLine(
			ImVec2(bb.Min.x + i, bb.Min.y),
			ImVec2(bb.Min.x + i, bb.Max.y),
			GetColorU32(ImGuiCol_TextDisabled));
	}
	for (int i = 0; i <= canvas.y; i += (canvas.y / 4)) {
		DrawList->AddLine(
			ImVec2(bb.Min.x, bb.Min.y + i),
			ImVec2(bb.Max.x, bb.Min.y + i),
			GetColorU32(ImGuiCol_TextDisabled));
	}
	DrawList->AddText(ImVec2(bb.Min.x, bb.Max.y), GetColorU32(ImGuiCol_TextDisabled), "0");
	DrawList->AddText(ImVec2(bb.Max.x, bb.Max.y), GetColorU32(ImGuiCol_TextDisabled), "1");

	cout << "**********************" << endl;
	int nb = 128;
	for (int i = 0; i < nb; ++i) {
		float ratioP = (float)i / nb;
		float ratioN = (float)(i+1) / nb;
		Vector3 v0 = plot(ratioP);
		Vector3 v1 = plot(ratioN);
		ImVec2 a(bb.Min.x + (bb.Max.x - bb.Min.x) * ratioP, bb.Min.y + v0.x * (bb.Max.y - bb.Min.y));
		ImVec2 b(bb.Min.x + (bb.Max.x - bb.Min.x) * ratioN, bb.Min.y + v1.x * (bb.Max.y - bb.Min.y));
		DrawList->AddLine(
			a,
			b,
			0xFF0000ff);
	}

	for (int i = 0; i < nb; ++i) {
		float ratioP = (float)i / nb;
		float ratioN = (float)(i + 1) / nb;
		Vector3 v0 = plot(ratioP);
		Vector3 v1 = plot(ratioN);
		ImVec2 a(bb.Min.x + (bb.Max.x - bb.Min.x) * ratioP, bb.Min.y + v0.y * (bb.Max.y - bb.Min.y));
		ImVec2 b(bb.Min.x + (bb.Max.x - bb.Min.x) * ratioN, bb.Min.y + v1.y * (bb.Max.y - bb.Min.y));
		DrawList->AddLine(
			a,
			b,
			0xff00ff00);
	}

	for (int i = 0; i < nb; ++i) {
		float ratioP = (float)i / nb;
		float ratioN = (float)(i + 1) / nb;
		Vector3 v0 = plot(ratioP);
		Vector3 v1 = plot(ratioN);
		ImVec2 a(bb.Min.x + (bb.Max.x - bb.Min.x) * ratioP, bb.Min.y + v0.z * (bb.Max.y - bb.Min.y));
		ImVec2 b(bb.Min.x + (bb.Max.x - bb.Min.x) * ratioN, bb.Min.y + v1.z * (bb.Max.y - bb.Min.y));
		DrawList->AddLine(
			a,
			b,
			0xffff0000);
	}

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 12);
	return changed;
}


const Vector2& rd::ext::CurveC2::get(int idx) {
	return data[std::clamp<int>(idx, 0, data.size() - 1)];
}

Vector2 rd::ext::CurveC2::plot(float t) {
	int l = (data.size() - 1);
	float fidx = (t * l);
	int idx = (int)(t * l);
	float f = fidx - idx;

	const auto& pm1 = get(idx - 1);
	const auto& p0 = get(idx);
	const auto& p1 = get(idx + 1);
	const auto& p2 = get(idx + 2);

	return rd::ext::Interp::c2(pm1, p0, p1, p2, f);
}

void rd::ext::CurveC2::load(const char* folder, const char* file) {
	rs::jDeserialize(data, folder, file, "data");
}

void rd::ext::CurveC2::save(const char* folder, const char* file) {
	rs::jSerialize(data, folder, file, "data");
}

bool rd::ext::CurveC2::im() {
	using namespace ImGui;
	bool changed = false;

	Text("Curve Editor");
	if (!data.size()) {
		Text("No Vertices");
	}

	int idx = 0;
	for (auto& f : data) {
		Text("%d ", idx);
		PushID(idx);
		SameLine();
		changed|=DragFloat2("val", f.ptr(), 0.01f);
		SameLine();

		if (Button(ICON_MD_DELETE)) {
			changed = true;
			data.erase(data.begin() + idx);
			PopID();
			break;
		}
		PopID();
		idx++;
	}

	if (Button(ICON_MD_ADD)) {
		Vector2 toInsert;
		if (data.size() == 0) {
			toInsert = Vector2( 0, 0);
			data.push_back(toInsert);
		}
		else if (data.size() == 1) {
			toInsert = Vector2( 1, 1);
			data.push_back(toInsert);
		}
		else {
			int idxMin = 0;
			int idxMax = data.size() - 1;
			int idxMid = (idxMin + idxMax) >> 1;
			int idxMidN = idxMid + 1;
			if (idxMidN >= idxMax) idxMidN = idxMax;
			toInsert = 0.5f * (data[idxMid] + data[idxMidN]);
			data.insert(data.begin() + idxMidN, toInsert);
		}
		changed = true;
	}

	//see https://github.com/ocornut/imgui/issues/786
	if (data.empty()) return changed;

	const char* label = "myCurveView";
	const ImGuiStyle& Style = GetStyle();
	const ImGuiIO& IO = GetIO();
	ImDrawList* DrawList = GetWindowDrawList();
	ImGuiWindow* Window = GetCurrentWindow();

	const float avail = GetContentRegionAvailWidth();
	const float dim = ImMin(avail, 128.f);
	ImVec2 canvas(dim, dim);
	ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + canvas);

	ItemSize(bb);
	if (!ItemAdd(bb, NULL))
		return changed;

	RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg, 1), true, Style.FrameRounding);

	// background grid
	for (int i = 0; i <= canvas.x; i += (canvas.x / 4)) {
		DrawList->AddLine(
			ImVec2(bb.Min.x + i, bb.Min.y),
			ImVec2(bb.Min.x + i, bb.Max.y),
			GetColorU32(ImGuiCol_TextDisabled));
	}
	for (int i = 0; i <= canvas.y; i += (canvas.y / 4)) {
		DrawList->AddLine(
			ImVec2(bb.Min.x, bb.Min.y + i),
			ImVec2(bb.Max.x, bb.Min.y + i),
			GetColorU32(ImGuiCol_TextDisabled));
	}
	DrawList->AddText(ImVec2(bb.Min.x, bb.Max.y), GetColorU32(ImGuiCol_TextDisabled), "0");
	DrawList->AddText(ImVec2(bb.Max.x, bb.Max.y), GetColorU32(ImGuiCol_TextDisabled), "1");

	cout << "**********************" << endl;
	int nb = 128;
	for (int i = 0; i < nb; ++i) {
		float ratioP = (float)i / nb;
		float ratioN = (float)(i + 1) / nb;
		Vector2 v0 = plot(ratioP);
		Vector2 v1 = plot(ratioN);
		ImVec2 a(bb.Min.x + (bb.Max.x - bb.Min.x) * ratioP, bb.Min.y + v0.x * (bb.Max.y - bb.Min.y));
		ImVec2 b(bb.Min.x + (bb.Max.x - bb.Min.x) * ratioN, bb.Min.y + v1.x * (bb.Max.y - bb.Min.y));
		DrawList->AddLine(
			a,
			b,
			0xFF0000ff);
	}

	for (int i = 0; i < nb; ++i) {
		float ratioP = (float)i / nb;
		float ratioN = (float)(i + 1) / nb;
		Vector2 v0 = plot(ratioP);
		Vector2 v1 = plot(ratioN);
		ImVec2 a(bb.Min.x + (bb.Max.x - bb.Min.x) * ratioP, bb.Min.y + v0.y * (bb.Max.y - bb.Min.y));
		ImVec2 b(bb.Min.x + (bb.Max.x - bb.Min.x) * ratioN, bb.Min.y + v1.y * (bb.Max.y - bb.Min.y));
		DrawList->AddLine(
			a,
			b,
			0xff00ff00);
	}

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 12);
	return changed;
}
