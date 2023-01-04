#pragma once

#include "4-menus/imgui.h"

namespace rd {

	struct ParamFx {
		float hue = 0, sat = 1, val = 1;
		float colMult = 1;
		ImVec4 colorize = ImColor(255, 0, 0, 0);
		float ratioNew = 0, ratioOld = 1;
		char buf[32];
		std::vector<std::string> files;
		bool fileSearch = false;
	};

	class HelperFX {
	private:
		static ParamFx param;
	public:
		static void showPostFxTune(
			float& glitchIntensityX, float& glitchPercentBigX, float& glitchIntensityY, float& glitchPercentBigY,
			float& blurX, float& blurY, float& blurScale,
			r::Color& bloomMul, float& bloomPass, r::Vector3& lumVector,
			r::Vector2& redOff, r::Vector2& greOff, r::Vector2& bluOff,
			Matrix44& matColor);

		static void resetFx(
			float& glitchIntensityX, float& glitchPercentBigX, float& glitchIntensityY, float& glitchPercentBigY,
			float& blurX, float& blurY, float& blurScale,
			r::Color& bloomMul, float& bloomPass, r::Vector3& lumVector,
			r::Vector2& redOff, r::Vector2& greOff, r::Vector2& bluOff,
			Matrix44& matColor);

		static void loadFx(
			std::string filename, 
			float& glitchIntensityX, float& glitchPercentBigX, float& glitchIntensityY, float& glitchPercentBigY,
			float& blurX, float& blurY, float& blurScale,
			r::Color& bloomMul, float& bloomPass, r::Vector3& lumVector,
			r::Vector2& redOff, r::Vector2& greOff, r::Vector2& bluOff,
			Matrix44& matColor);

		static void saveFx(
			std::string filename,
			float& glitchIntensityX, float& glitchPercentBigX, float& glitchIntensityY, float& glitchPercentBigY,
			float& blurX, float& blurY, float& blurScale,
			r::Color& bloomMul, float& bloomPass, r::Vector3& lumVector,
			r::Vector2& redOff, r::Vector2& greOff, r::Vector2& bluOff,
			Matrix44& matColor);
	};
}