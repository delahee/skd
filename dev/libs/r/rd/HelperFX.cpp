#include "stdafx.h"

#include "HelperFX.hpp"

using namespace std;
using namespace r;
using namespace Pasta;

rd::ParamFx rd::HelperFX::param = rd::ParamFx();

void rd::HelperFX::saveFx(
	std::string filename,
	float& glitchIntensityX, float& glitchPercentBigX, float& glitchIntensityY, float& glitchPercentBigY,
	float& blurX, float& blurY, float& blurScale,
	r::Color& bloomMul, float& bloomPass, Vector3& lumVector,
	Vector2& redOff, Vector2& greOff, Vector2& bluOff,
	Matrix44& matColor) {
	FileMgr* fileMgr = FileMgr::getSingleton();
	auto file = fileMgr->createFile("shaderParams/" + filename + ".param");

	file->open(Pasta::File::FA_WRITE);
	file->write((float*) &glitchIntensityX, sizeof(float));
	file->write((float*) &blurX, sizeof(float));
	file->write((float*) &blurY, sizeof(float));
	file->write((float*) &blurScale, sizeof(float));
	file->write((r::Color*) &bloomMul, sizeof(r::Color));
	file->write((float*) &bloomPass, sizeof(float));
	file->write((Vector3*) &lumVector, sizeof(Vector3));
	file->write((float*) &param.colMult, sizeof(float));
	file->write((Vector2*) &redOff, sizeof(Vector2));
	file->write((Vector2*) &greOff, sizeof(Vector2));
	file->write((Vector2*) &bluOff, sizeof(Vector2));
	file->write((Matrix44*) &matColor, sizeof(matColor));
	file->write((float*) &param.hue, sizeof(float));
	file->write((float*) &param.sat, sizeof(float));
	file->write((float*) &param.val, sizeof(float));
	file->write((float*) &param.ratioNew, sizeof(float));
	file->write((float*) &param.ratioOld, sizeof(float));
	file->write((float*) &glitchPercentBigX, sizeof(float));
	file->write((float*) &glitchIntensityY, sizeof(float));
	file->write((float*) &glitchPercentBigY, sizeof(float));
	//AJOUTER NOUVEAU PARAM A LA FIN POUR RETROCOMPAT
	file->close();
}

void rd::HelperFX::loadFx(
	std::string filename,
	float& glitchIntensityX, float& glitchPercentBigX, float& glitchIntensityY, float& glitchPercentBigY,
	float& blurX, float& blurY, float& blurScale,
	r::Color& bloomMul, float& bloomPass, Vector3& lumVector,
	Vector2& redOff, Vector2& greOff, Vector2& bluOff,
	Matrix44& matColor) {
	FileMgr* fileMgr = FileMgr::getSingleton();

	if ((filename.size() <= 6) || filename.substr(filename.size() - 6, 6).compare(".param")) {
		filename.append(".param");
	}

	auto file = fileMgr->createFile("shaderParams/" + filename);
	if (!file->exists())
		return;
	resetFx(glitchIntensityX, glitchPercentBigX, glitchIntensityY, glitchPercentBigY, blurX, blurY, blurScale, bloomMul, bloomPass, lumVector, redOff, greOff, bluOff, matColor);
	file->open(Pasta::File::FA_READ);
	file->read((float*) &glitchIntensityX, sizeof(float));
	file->read((float*) &blurX, sizeof(float));
	file->read((float*) &blurY, sizeof(float));
	file->read((float*) &blurScale, sizeof(float));
	file->read((r::Color*) &bloomMul, sizeof(Pasta::Color));
	file->read((float*) &bloomPass, sizeof(float));
	file->read((Vector3*) &lumVector, sizeof(Vector3));
	file->read((float*) &param.colMult, sizeof(float));
	file->read((Vector2*) &redOff, sizeof(Vector2));
	file->read((Vector2*) &greOff, sizeof(Vector2));
	file->read((Vector2*) &bluOff, sizeof(Vector2));
	file->read((Matrix44*) &matColor, sizeof(matColor));
	file->read((float*) &param.hue, sizeof(float));
	file->read((float*) &param.sat, sizeof(float));
	file->read((float*) &param.val, sizeof(float));
	file->read((float*) &param.ratioNew, sizeof(float));
	file->read((float*) &param.ratioOld, sizeof(float));
	file->read((float*) &glitchPercentBigX, sizeof(float));
	file->read((float*) &glitchIntensityY, sizeof(float));
	file->read((float*) &glitchPercentBigY, sizeof(float));
	//AJOUTER NOUVEAU PARAM A LA FIN POUR RETROCOMPAT
	file->close();
}

void rd::HelperFX::resetFx(
	float& glitchIntensityX, float& glitchPercentBigX, float& glitchIntensityY, float& glitchPercentBigY,
	float& blurX, float& blurY, float& blurScale,
	r::Color& bloomMul, float& bloomPass, Vector3& lumVector,
	Vector2& redOff, Vector2& greOff, Vector2& bluOff,
	Matrix44& matColor) {
	glitchIntensityX = 0.0;
	glitchIntensityY = 0.0;
	glitchPercentBigX = 0.0;
	glitchPercentBigY = 0.0;
	blurX = 8.0;
	blurY = 8.0;
	blurScale = 1.0;
	bloomMul = r::Color::Black;
	bloomPass = 0.150f;
	lumVector = Vector3(0.299f, 0.587f, 0.114f);
	param.colMult = 1.0f;
	redOff = Vector2(0, 0);
	greOff = Vector2(0, 0);
	bluOff = Vector2(0, 0);
	matColor = Matrix44::identity;
	param.hue = 0.0f;
	param.sat = 1.0f;
	param.val = 1.0f;
	param.ratioNew = 0.0f;
	param.ratioOld = 1.0f;
}


void rd::HelperFX::showPostFxTune(
	float& glitchIntensityX, float& glitchPercentBigX, float& glitchIntensityY, float& glitchPercentBigY,
	float& blurX, float& blurY, float& blurScale,
	r::Color& bloomMul, float& bloomPass, Vector3& lumVector,
	Vector2& redOff, Vector2& greOff, Vector2& bluOff,
	Matrix44& matColor) {
	ImGui::Begin("Debug Shader", NULL, ImGuiWindowFlags_None | ImGuiWindowFlags_MenuBar);

	ImGui::PushItemWidth(-100);
	bool openModal = false;
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Menu")) {
			if (ImGui::MenuItem("Reset")) {
				resetFx(glitchIntensityX, glitchPercentBigX, glitchIntensityY, glitchPercentBigY, blurX, blurY, blurScale, bloomMul, bloomPass, lumVector, redOff, greOff, bluOff, matColor);
			}
			if (ImGui::BeginMenu("Open")) {
				FileMgr* fileMgr = FileMgr::getSingleton();
				if (!param.fileSearch) {
					auto path = fileMgr->convertResourcePath("shaderParams");
					auto dirData = fileMgr->openDir(path.c_str());
					bool hasFile;
					param.files.clear();
					do {
						hasFile = false;
						auto entr = fileMgr->nextDirEntry(dirData, Pasta::FileMgr::LIST_FILES);
						if (entr) {
							param.files.push_back(entr->name);
							hasFile = true;
						}
					} while (hasFile);
					param.fileSearch = true;
					fileMgr->closeDir(dirData);
				}

				for (std::string f : param.files) {
					if (ImGui::MenuItem(f.c_str())) {
						loadFx(f, glitchIntensityX, glitchPercentBigX, glitchIntensityY, glitchPercentBigY, blurX, blurY, blurScale, bloomMul, bloomPass, lumVector, redOff, greOff, bluOff, matColor);
					}
				}
				ImGui::EndMenu();
			}
			else {
				param.fileSearch = false;
			}

			if (ImGui::MenuItem("Save As..."))
				openModal = true;


			if (ImGui::MenuItem("Random")) {
				glitchIntensityX = Rand::get().diceF(-10, 10);
				glitchIntensityY = Rand::get().diceF(-10, 10);
				glitchPercentBigX = Rand::get().diceF(0, 0.2f);
				glitchPercentBigY = Rand::get().diceF(0, 0.2f);
				blurX = Rand::get().diceF(0, 16);
				blurY = Rand::get().diceF(0, 16);
				bloomMul = r::Color(Rand::get().diceF(0, 1), Rand::get().diceF(0, 1), Rand::get().diceF(0, 1));
				param.colMult = Rand::get().diceF(0, 3);
				bloomPass = Rand::get().diceF(0, 1.0f);
				lumVector = Vector3(0.299f, 0.587f, 0.114f);
				redOff = Vector2(Rand::get().diceF(-10, 10), Rand::get().diceF(-10, 10));
				greOff = Vector2(Rand::get().diceF(-10, 10), Rand::get().diceF(-10, 10));
				bluOff = Vector2(Rand::get().diceF(-10, 10), Rand::get().diceF(-10, 10));
				param.hue = Rand::get().diceF(-180, 180);
				param.sat = Rand::get().diceF(0, 2.0f);
				param.val = Rand::get().diceF(0, 2.0f);
				ColorLib::colorHSV(matColor, param.hue, param.sat, param.val);
				matColor = matColor.transpose();
			}

			ImGui::EndMenu();
		}
		else {

		}
		ImGui::EndMenuBar();
	}

	if (openModal)
		ImGui::OpenPopup("Save ##1");
	if (ImGui::BeginPopupModal("Save ##1", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::PushItemWidth(-100);
		ImGui::Text("Filename : ");
		ImGui::InputText(".param", (char*) &param.buf, 32);
		ImGui::Separator();

		ImGui::Spacing();
		ImGui::SameLine(0, 40);
		if (ImGui::Button("Save", ImVec2(100, 0))) {
			saveFx(std::string(param.buf), glitchIntensityX, glitchPercentBigX, glitchIntensityY, glitchPercentBigY, blurX, blurY, blurScale, bloomMul, bloomPass, lumVector, redOff, greOff, bluOff, matColor);

			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(100, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
		ImGui::PopItemWidth();
	}

	ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoOptions;
	flags |= ImGuiColorEditFlags_NoAlpha;
	flags |= ImGuiColorEditFlags_NoSidePreview;
	flags |= ImGuiColorEditFlags_NoLabel;
	flags |= ImGuiColorEditFlags_NoInputs;

	if (ImGui::CollapsingHeader("Glitch")) {
		ImGui::DragFloat("Intensity X", (float*) &glitchIntensityX, 0.5f);
		ImGui::DragFloat("Intensity Y", (float*) &glitchIntensityY, 0.5f);
		glitchPercentBigX *= 100;
		glitchPercentBigY *= 100;
		ImGui::DragFloat("Big Glitch X", (float*) &glitchPercentBigX, 0.1f, 0.0f, 100.0f, "%.1f %%");
		ImGui::DragFloat("Big Glitch Y", (float*) &glitchPercentBigY, 0.1f, 0.0f, 100.0f, "%.1f %%");
		glitchPercentBigX *= 0.01f;
		glitchPercentBigY *= 0.01f;
	}

	if (ImGui::CollapsingHeader("RGB Offset")) {
		ImGui::DragFloat2("Offset Red", (float*) &redOff, 0.5f);
		ImGui::DragFloat2("Offset Green", (float*) &greOff, 0.5f);
		ImGui::DragFloat2("Offset Blue", (float*) &bluOff, 0.5f);
	}

	if (ImGui::CollapsingHeader("Blur - Bloom")) {
		ImGui::Text("Blur");
		ImGui::DragFloat("Blur X", (float*) &blurX, 0.5f, 0.0f, 16.0f * 4);
		ImGui::DragFloat("Blur Y", (float*) &blurY, 0.5f, 0.0f, 16.0f * 4);
		if (blurX > 80)
			blurX = 80; 
		if (blurY > 80)
			blurY = 80;
		ImGui::DragFloat("Blur Scale", (float*) &blurScale, 0.5f);

		ImGui::Separator();
		ImGui::Text("Bloom");

		ImGui::Text("Color");
		bloomMul = bloomMul / param.colMult;
		ImGui::PushItemWidth(175);
		ImGui::ColorPicker4("Bloom Mult", (float*) &bloomMul, flags, NULL);
		ImGui::PopItemWidth();
		ImGui::DragFloat("Multiplier", (float*) &param.colMult, 0.01f, 0.0f, 20.0f);
		bloomMul = bloomMul * param.colMult;
		ImGui::DragFloat("Bloom Pass", (float*) &bloomPass, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat3("Luminance", (float*) &lumVector, 0.001f, 0.0f, 1.0f);
	}

	if (ImGui::CollapsingHeader("Color Matrix")) {
		//ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);
		static int mode = 0;
		ImGui::Combo("Mode", &mode, "HSV\0Colorize\0Matrix\0\0");

		switch (mode) {
			case 0:
				matColor = Matrix44::identity;
				ImGui::DragFloat("Hue", (float*) &param.hue, 1.0f, -180, 180);
				ImGui::DragFloat("Satur.", (float*) &param.sat, 0.01f, 0, 2);
				ImGui::DragFloat("Value", (float*) &param.val, 0.01f, 0, 2);
				ColorLib::colorHSV(matColor, param.hue, param.sat, param.val);
				matColor = matColor.transpose();
				break;
			case 1: {
				matColor = Matrix44::identity;
				ImGui::Text("Tint:");
				ImGui::PushItemWidth(175);
				ImGui::ColorPicker4("Tint", (float*)&param.colorize, flags, NULL);
				ImGui::PopItemWidth();
				ImGui::DragFloat("Ratio New", (float*)&param.ratioNew, 0.01f, 0, 2);
				ImGui::DragFloat("Ratio Old", (float*)&param.ratioOld, 0.01f, 0, 2);

				auto c = param.colorize;
				r::Color cc(c.x, c.y, c.z, c.w);
				ColorLib::colorColorize(matColor, cc, param.ratioNew, param.ratioOld);
				matColor = matColor.transpose();
				break;
			}
			case 2:
				ImGui::PushItemWidth(-50);
				ImVec4 colorR = matColor.getRow(0);
				ImVec4 colorG = matColor.getRow(1);
				ImVec4 colorB = matColor.getRow(2);
				ImVec4 colorA = matColor.getRow(3);
				ImGui::DragFloat4("##1", (float*) &colorR, 0.01f, -1, 2);
				ImGui::DragFloat4("##2", (float*) &colorG, 0.01f, -1, 2);
				ImGui::DragFloat4("##3", (float*) &colorB, 0.01f, -1, 2);
				ImGui::DragFloat4("##4", (float*) &colorA, 0.01f, -1, 2);
				matColor.setRow(0, colorR);
				matColor.setRow(1, colorG);
				matColor.setRow(2, colorB);
				matColor.setRow(3, colorA);
				ImGui::PopItemWidth();

				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.4f);
				colorR = matColor.getCol(0);
				colorG = matColor.getCol(1);
				colorB = matColor.getCol(2);
				colorA = matColor.getCol(3);
				ImGui::BeginGroup();
				ImGui::Text("Color R:");
				ImGui::ColorPicker4("ColorR", (float*) &colorR, flags, NULL);
				ImGui::EndGroup();
				ImGui::SameLine();
				ImGui::BeginGroup();
				ImGui::Text("Color G:");
				ImGui::ColorPicker4("ColorG", (float*) &colorG, flags, NULL);
				ImGui::EndGroup();
				ImGui::BeginGroup();
				ImGui::Text("Color B:");
				ImGui::ColorPicker4("ColorB", (float*) &colorB, flags, NULL);
				ImGui::EndGroup();
				ImGui::SameLine();
				ImGui::BeginGroup();
				ImGui::Text("Color A:");
				ImGui::ColorPicker4("ColorA", (float*) &colorA, flags, NULL);
				ImGui::EndGroup();
				matColor.setCol(0, colorR);
				matColor.setCol(1, colorG);
				matColor.setCol(2, colorB);
				matColor.setCol(3, colorA);
				ImGui::PopItemWidth();

				break;
		}
		if (ImGui::Button("Reset")) {
			matColor = Matrix44::identity;

			param.hue = 0.0f;
			param.sat = 1.0f;
			param.val = 1.0f;

			param.ratioNew = 0.0f;
			param.ratioOld = 1.0f;
		}
	}

	ImGui::End();
}