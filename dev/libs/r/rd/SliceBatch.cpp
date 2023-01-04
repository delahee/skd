#include "stdafx.h"

#include "SliceBatch.hpp"
#include "r2/im/TilePicker.hpp"

rd::Slice3::Slice3(Node * parent, const Slice3Param & _param) : r2::Batch(parent) {
	set(_param);
}


rd::Slice3Param::Slice3Param() {
	tile = r2::GpuObjects::whiteTile->clone();
	ownTile = true;
}

rd::Slice3Param::Slice3Param(const Slice3Param& sp) {
	tile = sp.tile->clone();
	ownTile = true;
}

void rd::Slice3Param::setTile(const r2::Tile* t) {
	if (tile) {
		if (ownTile)
			tile->destroy();
		tile = 0;
		ownTile = false;
	}
	tile = t->clone();
	ownTile = true;
}

void rd::Slice3::set(const Slice3Param& _param) {
	param = _param;

	int tWidth = param.targetWidth - param.marginLeft - param.marginRight;
	if (tWidth <= 0) tWidth = 0.0;

	if (!param.tile){
		param.tile = (r2::Tile*)r2::GpuObjects::whiteTile->clone();
		param.ownTile = true;
	}

	if (tileLeft) rd::Pools::tiles.free(tileLeft);
	if (tileCenter) rd::Pools::tiles.free(tileCenter);
	if (tileRight) rd::Pools::tiles.free(tileRight);
	if (bLeft) rd::Pools::free(bLeft);
	if (bCenter) rd::Pools::free(bCenter);
	if (bRight) rd::Pools::free(bRight);

	tileLeft = param.tile->clone();
	tileLeft->setSize(param.marginLeft, param.tile->height);

	tileCenter = param.tile->clone();
	tileCenter->setPos(param.tile->x + param.marginLeft, param.tile->y);
	tileCenter->setSize(param.tile->width - param.marginLeft - param.marginRight, param.tile->height);

	tileRight = param.tile->clone();
	tileRight->setPos(param.tile->x + param.tile->width - param.marginRight, param.tile->y);
	tileRight->setSize(param.marginRight, param.tile->height);

	bLeft = alloc(tileLeft);
	bCenter = alloc(tileCenter);
	bCenter->x = bLeft->width();
	bCenter->setSize(tWidth, bCenter->height());
	bRight = alloc(tileRight);
	bRight->x = bCenter->x + bCenter->width();
}

rd::Slice3::~Slice3() {
	if(tileLeft)tileLeft->toPool();
	if (tileCenter)tileCenter->toPool();
	if (tileRight)tileRight->toPool();
}

void rd::Slice3::setTargetSize(int width) {
	param.targetWidth = width;
	int tWidth = PASTA_MAX(0, param.targetWidth - param.marginLeft - param.marginRight);

	bCenter->setSize(tWidth, bCenter->height());
	bRight->x = bCenter->x + bCenter->width();
}

void rd::Slice3::updateMargins(int marginLeft, int marginRight) {
	if (marginLeft != -1 && marginLeft != param.marginLeft) {
		param.marginLeft = marginLeft;
		if (param.marginLeft > param.tile->width - param.marginRight) {
			param.marginRight = param.tile->width - param.marginLeft;
			if(marginRight > param.marginRight) marginRight = param.marginRight;
		}
	}
	if (marginRight != -1 && marginRight != param.marginRight) {
		param.marginRight = marginRight;
		if (param.tile->width - param.marginRight < param.marginLeft) param.marginLeft = param.tile->width - param.marginRight;
	}
	int tWidth = PASTA_MAX(0, param.targetWidth - param.marginLeft - param.marginRight);

	tileLeft->setSize(param.marginLeft, param.tile->height);
	tileCenter->setPos(param.tile->x + param.marginLeft, param.tile->y);
	tileCenter->setSize(param.tile->width - param.marginLeft - param.marginRight, param.tile->height);
	tileRight->setPos(param.tile->x + param.tile->width - param.marginRight, param.tile->y);
	tileRight->setSize(param.marginRight, param.tile->height);
	bCenter->x = bLeft->width();
	bCenter->setSize(tWidth, bCenter->height());
	bRight->x = bCenter->x + bCenter->width();
}


void rd::Slice3::im() {
	using namespace ImGui;
	r2::Batch::im();

	if (ImGui::CollapsingHeader(ICON_MD_VIEW_ARRAY " 3 Slice Batch")) {
		ImGui::Indent();
		
		if (Button("Change tile")) {
			auto p = r2::im::TilePicker::forTile(*param.tile);
			p->autoReleased();
		}

		ImGui::Text("Margin left: " ICON_MD_ARROW_FORWARD "\n%d px", param.marginLeft);
		ImGui::SameLine();
		ImVec2 pStart = ImGui::GetCursorScreenPos();
		float finalHeight = 100 / (param.tile->width / param.tile->height);
		ImGui::Image(param.tile, ImVec2(100, finalHeight));
		ImVec2 pEnd = ImGui::GetCursorScreenPos();

		float marginLeftU = param.marginLeft / param.tile->width;
		float marginRightU = param.marginRight / param.tile->width;

		ImU32 col = IM_COL32(255, 0, 0, 180);
		int leftX = 100 * marginLeftU;
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(pStart.x + leftX, pStart.y),
			ImVec2(pStart.x + leftX, pEnd.y),
			col);
		int rightX = 100 - 100 * marginRightU;
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(pStart.x + rightX, pStart.y),
			ImVec2(pStart.x + rightX, pEnd.y),
			col);
		ImGui::SameLine();
		ImGui::Text(ICON_MD_ARROW_BACK);
		ImGui::SameLine();
		ImGui::Text("Margin right:\n%d px", param.marginRight);

		ImGui::Dummy(ImVec2(50, 1)); ImGui::SameLine();
		ImGui::Text("Middle width " ICON_MD_CODE ": %d px", PASTA_MAX(0, param.targetWidth - param.marginLeft - param.marginRight));

		static int value[2] = { 0, 0 };
		value[0] = param.marginLeft;
		value[1] = param.marginRight;
		ImGui::SetNextItemWidth(124);
		if (ImGui::SliderInt2("Margin left/right", value, 0, param.tile->width))
			updateMargins(value[0], value[1]);
		value[0] = param.targetWidth;
		ImGui::SetNextItemWidth(60);
		if (ImGui::DragInt("Target width", &value[0], 0.5f, 0, INT_MAX))
			setTargetSize(value[0]);

		ImGui::Unindent();
	}
}

rd::Slice9::Slice9(Node * parent, const Slice9Param & _param) : r2::Batch(parent) {
	set(_param);
}

void rd::Slice9::set(const Slice9Param& _param) {
	param = _param;

	if (!param.tile)
		param.tile = (r2::Tile*)r2::GpuObjects::whiteTile;

	for (auto& t : { tileTopLeft }) {

	}

	tileTopLeft = param.tile->clone();
	tileTopCenter = param.tile->clone();
	tileTopRight = param.tile->clone();
	tileLeft = param.tile->clone();
	tileCenter = param.tile->clone();
	tileRight = param.tile->clone();
	tileBotLeft = param.tile->clone();
	tileBotCenter = param.tile->clone();
	tileBotRight = param.tile->clone();

	bTopLeft = alloc(tileTopLeft);
	bTopCenter = alloc(tileTopCenter);
	bTopRight = alloc(tileTopRight);
	bLeft = alloc(tileLeft);
	bCenter = alloc(tileCenter);
	bRight = alloc(tileRight);
	bBotLeft = alloc(tileBotLeft);
	bBotCenter = alloc(tileBotCenter);
	bBotRight = alloc(tileBotRight);

	updateMargins();
}

rd::Slice9::~Slice9() {

}

void rd::Slice9::updateMargins(int marginLeft, int marginRight, int marginTop, int marginBottom) {
	if (marginLeft != -1 && marginLeft != param.marginLeft) {
		param.marginLeft = marginLeft;
		if (param.marginLeft > param.tile->width - param.marginRight) {
			param.marginRight = param.tile->width - param.marginLeft;
			if (marginRight > param.marginRight) marginRight = param.marginRight;
		}
	}
	else if (marginRight != -1 && marginRight != param.marginRight) {
		param.marginRight = marginRight;
		if (param.tile->width - param.marginRight < param.marginLeft) param.marginLeft = param.tile->width - param.marginRight;
	}
	if (marginTop != -1 && marginTop != param.marginTop) {
		param.marginTop = marginTop;
		if (param.marginTop > param.tile->height - param.marginBottom) {
			param.marginBottom = param.tile->height - param.marginTop;
			if (marginBottom > param.marginBottom) marginBottom = param.marginBottom;
		}
	}
	if (marginBottom != -1 && marginBottom != param.marginBottom) {
		param.marginBottom = marginBottom;
		if (param.tile->height - param.marginBottom < param.marginTop) param.marginTop = param.tile->height - param.marginBottom;
	}
	int tWidth = PASTA_MAX(0, param.targetWidth - param.marginLeft - param.marginRight);
	int tHeight = PASTA_MAX(0, param.targetHeight - param.marginTop - param.marginBottom);

	tileTopLeft->setSize(param.marginLeft, param.marginTop);
	tileTopCenter->setPos(param.tile->x + param.marginLeft, param.tile->y);
	tileTopCenter->setSize(param.tile->width - param.marginLeft - param.marginRight, param.marginTop);
	tileTopRight->setPos(param.tile->x + param.tile->width - param.marginRight, param.tile->y);
	tileTopRight->setSize(param.marginRight, param.marginTop);

	tileLeft->setPos(param.tile->x, param.tile->y + param.marginTop);
	tileLeft->setSize(param.marginLeft, param.tile->height - param.marginTop - param.marginBottom);
	tileCenter->setPos(param.tile->x + param.marginLeft, param.tile->y + param.marginTop);
	tileCenter->setSize(param.tile->width - param.marginLeft - param.marginRight, param.tile->height - param.marginTop - param.marginBottom);
	tileRight->setPos(param.tile->x + param.tile->width - param.marginRight, param.tile->y + param.marginTop);
	tileRight->setSize(param.marginRight, param.tile->height - param.marginTop - param.marginBottom);
	
	tileBotLeft->setPos(param.tile->x, param.tile->y + param.tile->height - param.marginBottom);
	tileBotLeft->setSize(param.marginLeft, param.marginBottom);
	tileBotCenter->setPos(param.tile->x + param.marginLeft, param.tile->y + param.tile->height - param.marginBottom);
	tileBotCenter->setSize(param.tile->width - param.marginLeft - param.marginRight, param.marginBottom);
	tileBotRight->setPos(param.tile->x + param.tile->width - param.marginRight, param.tile->y + param.tile->height - param.marginBottom);
	tileBotRight->setSize(param.marginRight, param.marginBottom);

	setTargetSize(param.targetWidth, param.targetHeight);
}

void rd::Slice9::setTargetSize(int width, int height) {
	param.targetWidth = width;
	param.targetHeight = height;
	int tWidth = PASTA_MAX(0, param.targetWidth - param.marginLeft - param.marginRight);
	int tHeight = PASTA_MAX(0, param.targetHeight - param.marginTop - param.marginBottom);

	bTopCenter->x = bTopLeft->x + bTopLeft->width();
	bTopCenter->setSize(tWidth, bTopCenter->height());
	bTopRight->x = bTopCenter->x + bTopCenter->width();

	bLeft->y = bTopLeft->y + bTopLeft->height();
	bLeft->setSize(bLeft->width(), tHeight);
	bCenter->x = bLeft->x + bLeft->width();
	bCenter->y = bTopCenter->y + bTopCenter->height();
	bCenter->setSize(tWidth, tHeight);
	bRight->x = bCenter->x + bCenter->width();
	bRight->y = bTopRight->y + bTopRight->height();
	bRight->setSize(bRight->width(), tHeight);

	bBotLeft->y = bLeft->y + bLeft->height();
	bBotCenter->x = bBotLeft->x + bBotLeft->width();
	bBotCenter->y = bCenter->y + bCenter->height();
	bBotCenter->setSize(tWidth, bBotCenter->height());
	bBotRight->x = bBotCenter->x + bBotCenter->width();
	bBotRight->y = bRight->y + bRight->height();
}

void rd::Slice9::im() {
	r2::Batch::im();

	if (ImGui::CollapsingHeader(ICON_MD_APPS " 9 Slice Batch")) {
		ImGui::Indent();

		ImGui::Text("\t\t   Margin top: " ICON_MD_ARROW_DOWNWARD " %d px", param.marginTop);
		ImGui::Text("\n\nMargin left: " ICON_MD_ARROW_FORWARD "\n%d px", param.marginLeft);
		ImGui::SameLine();
		ImVec2 pStart = ImGui::GetCursorScreenPos();
		float finalHeight = 100 / (param.tile->width / param.tile->height);
		ImGui::Image(param.tile, ImVec2(100, finalHeight));
		ImVec2 pEndY = ImGui::GetCursorScreenPos();
		ImGui::SameLine();
		ImVec2 pEndX = ImGui::GetCursorScreenPos();

		float marginLeftU = param.marginLeft / param.tile->width;
		float marginRightU = param.marginRight / param.tile->width;
		float marginTopV = param.marginTop / param.tile->height;
		float marginBottomV = param.marginBottom / param.tile->height;

		ImU32 col = IM_COL32(255, 0, 0, 180);
		int leftX = 100 * marginLeftU;
		int rightX = 100 - 100 * marginRightU;
		int topY = 100 * marginTopV;
		int bottomY = 100 - 100 * marginBottomV;
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(pStart.x + leftX, pStart.y),
			ImVec2(pStart.x + leftX, pEndY.y),
			col);
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(pStart.x + rightX, pStart.y),
			ImVec2(pStart.x + rightX, pEndY.y),
			col);
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(pStart.x, pStart.y + topY),
			ImVec2(pEndX.x, pStart.y + topY),
			col);
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(pStart.x, pStart.y + bottomY),
			ImVec2(pEndX.x, pStart.y + bottomY),
			col);
		ImGui::Text("\n\n" ICON_MD_ARROW_BACK);
		ImGui::SameLine();
		ImGui::Text("\n\nMargin right:\n%d px", param.marginRight);

		ImGui::Text("\t\tMargin bottom: " ICON_MD_ARROW_UPWARD " %d px", param.marginBottom);

		ImGui::Text("Middle width " ICON_MD_CODE ": %d px", PASTA_MAX(0, param.targetWidth - param.marginLeft - param.marginRight));
		ImGui::Text("Middle height " ICON_MD_HEIGHT ": %d px", PASTA_MAX(0, param.targetHeight - param.marginTop - param.marginBottom));

		ImGui::PushItemWidth(124);
		static int value[2] = { 0, 0 };
		value[0] = param.marginLeft;
		value[1] = param.marginRight;
		if (ImGui::SliderInt2("Margin left/right", value, 0, param.tile->width))
			updateMargins(value[0], value[1]);
		value[0] = param.marginTop;
		value[1] = param.marginBottom;
		if (ImGui::SliderInt2("Margin top/bottom", value, 0, param.tile->height))
			updateMargins(-1, -1, value[0], value[1]);
		value[0] = param.targetWidth;
		value[1] = param.targetHeight;
		if (ImGui::DragInt2("Target size", value, 0.5f, 0, INT_MAX))
			setTargetSize(value[0], value[1]);
		ImGui::PopItemWidth();

		ImGui::Unindent();
	}
}


rd::Slice9Param::Slice9Param() {
	tile = r2::GpuObjects::whiteTile->clone();
	ownTile = true;
}
