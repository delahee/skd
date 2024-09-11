#include "stdafx.h"

#include "4-menus/imgui.h"
#include "r3/Node3D.hpp"
#include "r3/Scene3D.hpp"
#include "r3/r3Im.hpp"

using namespace r3;

void Scene3D::im() {
	using namespace ImGui;
	if (ImGui::CollapsingHeader(ICON_MD_3D_ROTATION " Scene3D", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Indent();

		ImGui::SetNextItemWidth(150);
		ImGui::InputText("name", name);

		ImGui::DragFloat("perspective target width", &persW);
		ImGui::DragFloat("perspective target height", &persH);

		ImGui::PushItemWidth(188);
		ImGui::DragFloat3("Pos", cameraPos.ptr(), 0.25f, 0, 0, "%0.2lf");
		ImGui::DragFloat3("LookAt", cameraLookAt.ptr(), 0.25f, 0, 0, "%0.2lf");
		ImGui::DragFloat3("Offset", offset.ptr(), 0.25f, 0, 0, "%0.2lf");
		ImGui::DragFloat("Frustum angle", &angleDeg, 0.1f);
		ImGui::PopItemWidth();

		float smin = 0.5;
		float smax = 16.0;

		ImGui::PushItemWidth(124);
		ImGui::DragFloat2("Zoom", &cameraScale.x, 0.25f, smin, smax, "%0.2lf");

		ImGui::SetNextItemWidth(60);
		ImGui::DragFloat("DepthScale", &cameraScale.z, 0.001f, -1, 1, "%0.4lf");

		int rgz = round(1 / cameraScale.z);
		ImGui::Text("Depth Range [%d, %d] ", -rgz, rgz);

		ImGui::Checkbox("Has BG", &doClear);
		ImGui::ColorPicker4("BG Color", clearColor.ptr(), NULL);
		ImGui::PopItemWidth();

		ImGui::Unindent();
	}

	vars.im();
	Super::im();
}

void Node3D::im() {
	using namespace ImGui;

	ImGui::PushID(this);
	if (ImGui::CollapsingHeader(ICON_MD_VIEW_IN_AR " Node 3D", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PushItemWidth(60); ImGui::Indent();

		double zmin = rs::GfxContext::MTX_DEFAULT_ZMIN() * 400;
		double zmax = rs::GfxContext::MTX_DEFAULT_ZMAX() * 400;

		ImGui::PushID("pos3D");
		ImGui::SetNextItemWidth(188);
		ImGui::DragScalarN("pos", ImGuiDataType_Double, &x, 3, 1.f, 0, 0, "%0.2lf"); ImGui::SameLine();
		if (ImGui::Button(ICON_MD_UNDO)) { setPos(0.f, 0.f); z = 0.0f; };
		ImGui::PopID();

		ImGui::PushID("rot3D");
		double rotMin = -PASTA_PI * 2.0;
		double rotMax = PASTA_PI * 2.0;
		static double rot[] = {0, 0, 0};
		rot[0] = rotationX;	rot[1] = rotationY;	rot[2] = rotation;
		ImGui::SetNextItemWidth(188);
		ImGui::DragScalarN("rotation", ImGuiDataType_Double, &rot, 3, 0.05f, &rotMin, &rotMax, "%0.3lf"); ImGui::SameLine();
		rotationX = rot[0];	rotationY = rot[1]; rotation = rot[2];
		if (ImGui::Button(ICON_MD_UNDO)) { rotationX = 0.0; rotationY = 0.0; rotation = 0.0; }
		ImGui::PopID();

		ImGui::PushID("scale3D");
		double smin = 0.0;
		double smax = 1000;
		static double scale[] = { 0, 0, 0 };
		scale[0] = scaleX; scale[1] = scaleY; scale[2] = scaleZ;
		ImGui::SetNextItemWidth(188);
		ImGui::DragScalarN("scale", ImGuiDataType_Double, &scale, 3, 0.25f, &smin, &smax, "%0.2lf"); ImGui::SameLine();
		scaleX = scale[0]; scaleY = scale[1]; scaleZ = scale[2];
		if (ImGui::Button(ICON_MD_UNDO)) { scaleX = scaleY = scaleZ = 1.0; }
		ImGui::SameLine();
		if (ImGui::Button("Sync To X")) { scaleZ = scaleY = scaleX; }
		ImGui::PopID();

		ImGui::Unindent(); ImGui::PopItemWidth();
	}
	ImGui::PopID();

	r2::Node::im();
}