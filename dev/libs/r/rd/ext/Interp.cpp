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
	
	return false;
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
	return false;
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

	return changed;
}
