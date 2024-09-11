#include "stdafx.h"

#include "SliceBatch.hpp"
#include "r2/im/TilePicker.hpp"

using namespace rd;

rd::Slice3::Slice3(Node * parent, const Slice3Param & _param) : r2::Batch(parent) {
	set(_param);
	setName("Slice3");
}

Slice3Param::Slice3Param(const r2::Tile* t) {
	if(t)
		targetWidth = t->width;
	setTile(t);
}

Slice3Param& Slice3Param::operator=(const Slice3Param& sp) {
	tile = sp.tile.clone();
	targetWidth = sp.targetWidth;
	marginLeft = sp.marginLeft;
	marginRight = sp.marginRight;
	return *this;
}

rd::Slice3Param::Slice3Param(const r2::Tile* t, int tw, int ml, int mr) {
	tile.setAnon(t->clone());
	tile.tile->setCenterRatio(0, 0);
	targetWidth = tw;
	marginLeft = ml;
	marginRight = mr;
};

rd::Slice3Param::~Slice3Param() {
	tile.dispose();
}

rd::Slice3Param::Slice3Param() {
	tile.setAnon(r2::GpuObjects::whiteTile->clone());
}

rd::Slice3Param::Slice3Param(const Slice3Param& sp) {
	tile = sp.tile.clone();
	targetWidth = sp.targetWidth;
	marginLeft = sp.marginLeft;
	marginRight = sp.marginRight;
}

void rd::Slice3Param::setTile(const r2::Tile* t) {
	tile.dispose();
	if(t)
		tile.setAnon(t->clone());
	if (!t)
		tile.tile = r2::Tile::fromWhite();
	tile.tile->setCenterRatio(0, 0);
}

void rd::Slice3::set(const Slice3Param& _param) {
	param = _param;

	int tWidth = param.targetWidth - param.marginLeft - param.marginRight;
	if (tWidth <= 0) tWidth = 0.0;

	if (!param.tile.tile)
		param.tile.setAnon( (r2::Tile*)r2::GpuObjects::whiteTile->clone() );

	auto tile = param.tile.tile;
	param.tile.tile->setCenterRatio(0, 0);
	
	if (bLeft) bLeft->destroy(); bLeft = 0;
	if (bCenter) bCenter->destroy(); bCenter = 0;
	if (bRight) bRight->destroy(); bRight = 0;

	if (!tileLeft) tileLeft = r2::Tile::fromWhite();
	if (!tileCenter) tileCenter = r2::Tile::fromWhite();
	if (!tileRight) tileRight = r2::Tile::fromWhite();

	tileLeft = param.tile.tile->clone();
	tileLeft->setSize(param.marginLeft, tile->height);

	tileCenter = param.tile.tile->clone();

	tileCenter->setPos(tile->x + param.marginLeft, tile->y);
	tileCenter->setSize(tile->width - param.marginLeft - param.marginRight, tile->height);

	tileRight = param.tile.tile->clone();
	tileRight->setPos(tile->x + tile->width - param.marginRight, tile->y);
	tileRight->setSize(param.marginRight, tile->height);

	bLeft = alloc(tileLeft);
	bCenter = alloc(tileCenter);
	bRight = alloc(tileRight);

	bCenter->x = bLeft->width();
	bCenter->setSize(tWidth, bCenter->height());
	
	bRight->x = bCenter->x + bCenter->width();
}

rd::Slice3::~Slice3() {
	if (tileLeft) tileLeft->destroy(); tileLeft = 0;
	if (tileCenter) tileCenter->destroy(); tileCenter = 0;
	if (tileRight) tileRight->destroy(); tileRight = 0;
}

void rd::Slice3::setTargetSize(int width) {
	param.targetWidth = width;
	int tWidth = PASTA_MAX(0, param.targetWidth - param.marginLeft - param.marginRight);

	bCenter->setSize(tWidth, bCenter->height());
	bRight->x = bCenter->x + bCenter->width();
}

void rd::Slice3::updateMargins(int marginLeft, int marginRight) {
	auto tile = param.tile.tile;
	if (marginLeft != -1 && marginLeft != param.marginLeft) {
		param.marginLeft = marginLeft;
		if (param.marginLeft > tile->width - param.marginRight) {
			param.marginRight = tile->width - param.marginLeft;
			if(marginRight > param.marginRight) marginRight = param.marginRight;
		}
	}
	if (marginRight != -1 && marginRight != param.marginRight) {
		param.marginRight = marginRight;
		if (tile->width - param.marginRight < param.marginLeft) param.marginLeft = tile->width - param.marginRight;
	}
	int tWidth = PASTA_MAX(0, param.targetWidth - param.marginLeft - param.marginRight);

	tileLeft->setSize(param.marginLeft, tile->height);
	tileCenter->setPos(tile->x + param.marginLeft, tile->y);
	tileCenter->setSize(tile->width - param.marginLeft - param.marginRight, tile->height);
	tileRight->setPos(tile->x + tile->width - param.marginRight, tile->y);
	tileRight->setSize(param.marginRight, tile->height);

	bCenter->x = bLeft->width();
	bCenter->setSize(tWidth, bCenter->height());
	bRight->x = bCenter->x + bCenter->width();
}


void rd::Slice3::im() {
	using namespace ImGui;
	r2::Batch::im();
	auto tile = param.tile.tile;
	if (ImGui::CollapsingHeader(ICON_MD_VIEW_ARRAY " 3 Slice Batch")) {
		ImGui::Indent();
		
		if (Button("Change tile")) {
			auto p = r2::im::TilePicker::forPackage(param.tile);
			p->autoReleased();
			p->then([this](auto a, std::any data) {
				r2::Tile* t = std::any_cast<r2::Tile*>(data);
				if (!t) return data;
				param.targetWidth = t->width;
				set(param);
				return data;
			});
		}

		ImGui::Text("Margin left: " ICON_MD_ARROW_FORWARD "\n%d px", param.marginLeft);
		ImGui::SameLine();
		ImVec2 pStart = ImGui::GetCursorScreenPos();
		float finalHeight = 100 / (tile->width / tile->height);
		ImGui::Image(tile, ImVec2(100, finalHeight));
		ImVec2 pEnd = ImGui::GetCursorScreenPos();

		float marginLeftU = param.marginLeft / tile->width;
		float marginRightU = param.marginRight / tile->width;

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
		if (ImGui::SliderInt2("Margin left/right", value, 0, tile->width))
			updateMargins(value[0], value[1]);
		value[0] = param.targetWidth;
		ImGui::SetNextItemWidth(60);
		if (ImGui::DragInt("Target width", &value[0], 0.5f, 0, INT_MAX))
			setTargetSize(value[0]);

		ImGui::Unindent();
	}
}


void rd::Slice3::serialize(Pasta::JReflect& f, const char* name){
	//Super::serialize(f, name);
	//f.visit(param.tile, "tile");
	param.tile.serialize(&f, "tile");
	f.visit(param.targetWidth, "tw");
	f.visit(param.marginLeft, "ml");
	f.visit(param.marginRight, "mr");
}

rd::Slice9::Slice9(Node * parent, const Slice9Param & _param) : r2::Batch(parent) {
	set(_param);
	setName("Slice9");
}

void rd::Slice9::set(const Slice9Param& _param) {
	param = _param;

	if (!param.tile.isReady())
		param.tile.empty();

	if (bTopLeft)bTopLeft->destroy();
	if (bTopCenter)bTopCenter->destroy();
	if (bTopRight)bTopRight->destroy();
	if (bLeft)bLeft->destroy();
	if (bCenter)bCenter->destroy();
	if (bRight)bRight->destroy();
	if (bBotLeft)bBotLeft->destroy();
	if (bBotCenter)bBotCenter->destroy();
	if (bBotRight)bBotRight->destroy();

	auto inst = [&]() {
		return param.tile.tile->clone();
	};
	tileTopLeft = inst();
	tileTopCenter = inst();
	tileTopRight = inst();
	tileLeft = inst();
	tileCenter = inst();
	tileRight = inst();
	tileBotLeft = inst();
	tileBotCenter = inst();
	tileBotRight = inst();

	bTopLeft = alloc(tileTopLeft); bTopLeft->name=("tl");
	bTopCenter = alloc(tileTopCenter); bTopCenter->name = ("tc");
	bTopRight = alloc(tileTopRight); bTopRight->name = ("tr");
	bLeft = alloc(tileLeft); bLeft->name = ("l");
	bCenter = alloc(tileCenter); bCenter->name = ("c");
	bRight = alloc(tileRight); bRight->name = ("r");
	bBotLeft = alloc(tileBotLeft); bBotLeft->name = ("bl");
	bBotCenter = alloc(tileBotCenter); bBotCenter->name = ("bc");
	bBotRight = alloc(tileBotRight); bBotRight->name = ("br");

	updateMargins();
}

rd::Slice9::~Slice9() {
	if(tileTopLeft) tileTopLeft->destroy(); tileTopLeft = 0;
	if(tileTopCenter) tileTopCenter->destroy(); tileTopCenter = 0;
	if(tileTopRight) tileTopRight->destroy(); tileTopRight = 0;

	if(tileLeft) tileLeft->destroy(); tileLeft = 0;
	if(tileCenter) tileCenter->destroy(); tileCenter = 0;
	if(tileRight) tileRight->destroy(); tileRight = 0;

	if(tileBotLeft) tileBotLeft->destroy(); tileBotLeft = 0;
	if(tileBotCenter) tileBotCenter->destroy(); tileBotCenter = 0;
	if(tileBotRight) tileBotRight->destroy(); tileBotRight = 0;
	
}

void rd::Slice9::updateMargins(int marginLeft, int marginRight, int marginTop, int marginBottom) {
	if (!param.tile.isReady())
		return;

	auto tile = param.tile.tile;
	if (marginLeft != -1 && marginLeft != param.marginLeft) {
		param.marginLeft = marginLeft;
		if (param.marginLeft > tile->width - param.marginRight) {
			param.marginRight = tile->width - param.marginLeft;
			if (marginRight > param.marginRight) marginRight = param.marginRight;
		}
	}
	else if (marginRight != -1 && marginRight != param.marginRight) {
		param.marginRight = marginRight;
		if (tile->width - param.marginRight < param.marginLeft) param.marginLeft = tile->width - param.marginRight;
	}
	if (marginTop != -1 && marginTop != param.marginTop) {
		param.marginTop = marginTop;
		if (param.marginTop > tile->height - param.marginBottom) {
			param.marginBottom = tile->height - param.marginTop;
			if (marginBottom > param.marginBottom) marginBottom = param.marginBottom;
		}
	}
	if (marginBottom != -1 && marginBottom != param.marginBottom) {
		param.marginBottom = marginBottom;
		if (tile->height - param.marginBottom < param.marginTop) param.marginTop = tile->height - param.marginBottom;
	}
	int tWidth = PASTA_MAX(0, param.targetWidth - param.marginLeft - param.marginRight);
	int tHeight = PASTA_MAX(0, param.targetHeight - param.marginTop - param.marginBottom);

	tileTopLeft->setSize(param.marginLeft, param.marginTop);
	tileTopCenter->setPos(tile->x + param.marginLeft, tile->y);
	tileTopCenter->setSize(tile->width - param.marginLeft - param.marginRight, param.marginTop);
	tileTopRight->setPos(tile->x + tile->width - param.marginRight, tile->y);
	tileTopRight->setSize(param.marginRight, param.marginTop);

	tileLeft->setPos(tile->x, tile->y + param.marginTop);
	tileLeft->setSize(param.marginLeft, tile->height - param.marginTop - param.marginBottom);
	tileCenter->setPos(tile->x + param.marginLeft, tile->y + param.marginTop);
	tileCenter->setSize(tile->width - param.marginLeft - param.marginRight, tile->height - param.marginTop - param.marginBottom);
	tileRight->setPos(tile->x + tile->width - param.marginRight, tile->y + param.marginTop);
	tileRight->setSize(param.marginRight, tile->height - param.marginTop - param.marginBottom);
	
	tileBotLeft->setPos(tile->x, tile->y + tile->height - param.marginBottom);
	tileBotLeft->setSize(param.marginLeft, param.marginBottom);
	tileBotCenter->setPos(tile->x + param.marginLeft, tile->y + tile->height - param.marginBottom);
	tileBotCenter->setSize(tile->width - param.marginLeft - param.marginRight, param.marginBottom);
	tileBotRight->setPos(tile->x + tile->width - param.marginRight, tile->y + tile->height - param.marginBottom);
	tileBotRight->setSize(param.marginRight, param.marginBottom);

	setTargetSize(param.targetWidth, param.targetHeight);
}

void rd::Slice9::setTargetSize(int width, int height) {
	param.targetWidth = width;
	param.targetHeight = height;

	param.tile;

	int tWidth = PASTA_MAX(0, param.targetWidth - param.marginLeft - param.marginRight);
	int tHeight = PASTA_MAX(0, param.targetHeight - param.marginTop - param.marginBottom);

	bTopCenter->x = bTopLeft->x + bTopLeft->width();
	bTopCenter->setSize(tWidth, bTopLeft->height());
	bTopRight->x = bTopCenter->x + bTopCenter->width();

	bLeft->x = bTopLeft->x;
	bLeft->y = bTopLeft->y + bTopLeft->height();
	bLeft->setSize(bTopLeft->width(), tHeight);
	bCenter->x = bLeft->x + bLeft->width();
	bCenter->y = bTopCenter->y + bTopCenter->height();
	bCenter->setSize(tWidth, tHeight);
	bRight->x = bCenter->x + bCenter->width();
	bRight->y = bTopRight->y + bTopRight->height();
	bRight->setSize(bTopRight->width(), tHeight);

	bBotLeft->x = bTopLeft->x;
	bBotLeft->y = bLeft->y + bLeft->height();
	bBotLeft->setSize(bTopLeft->width(), param.marginBottom);
	bBotCenter->x = bBotLeft->x + bBotLeft->width();
	bBotCenter->y = bCenter->y + bCenter->height();
	bBotCenter->setSize(tWidth, bBotLeft->height());
	bBotRight->x = bBotCenter->x + bBotCenter->width();
	bBotRight->y = bRight->y + bRight->height();
}

static int value[2] = { 0, 0 };
void rd::Slice9::im() {
	using namespace ImGui;
	r2::Batch::im();

	if (ImGui::CollapsingHeader(ICON_MD_APPS " 9 Slice Batch")) {
		ImGui::Indent();
		if (!param.tile.isReady())
			param.tile.empty();
		auto tile = param.tile.tile;

		if (Button("Change tile")) {
			auto p = r2::im::TilePicker::forPackage(param.tile);
			p->autoReleased();
			p->then([this](auto a, std::any data) {
				r2::Tile* t = std::any_cast<r2::Tile*>(data);
				if (!t) return data;
				auto nt = param.tile.tile;
				param.tile.tile->setCenterRatio(0, 0);
				setTargetSize(nt->width, nt->height);
				set(param);
				return data;
			});
		}

		ImGui::Text("\t\t   Margin top: " ICON_MD_ARROW_DOWNWARD " %d px", param.marginTop);
		ImGui::Text("\n\nMargin left: " ICON_MD_ARROW_FORWARD "\n%d px", param.marginLeft);
		ImGui::SameLine();
		ImVec2 pStart = ImGui::GetCursorScreenPos();
		float finalHeight = 100 / (tile->width / tile->height);
		ImGui::Image(tile, ImVec2(100, finalHeight));
		ImVec2 pEndY = ImGui::GetCursorScreenPos();
		ImGui::SameLine();
		ImVec2 pEndX = ImGui::GetCursorScreenPos();

		auto marginLeftU = param.marginLeft / tile->width;
		auto marginRightU = param.marginRight / tile->width;
		auto marginTopV = param.marginTop / tile->height;
		auto marginBottomV = param.marginBottom / tile->height;

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
		value[0] = param.marginLeft;
		value[1] = param.marginRight;
		if (ImGui::SliderInt2("Margin left/right", value, 0, tile->width))
			updateMargins(value[0], value[1]);
		value[0] = param.marginTop;
		value[1] = param.marginBottom;
		if (ImGui::SliderInt2("Margin top/bottom", value, 0, tile->height))
			updateMargins(-1, -1, value[0], value[1]);
		value[0] = param.targetWidth;
		value[1] = param.targetHeight;
		if (ImGui::DragInt2("Target size", value, 0.5f, 0, INT_MAX))
			setTargetSize(value[0], value[1]);
		SameLine();
		if( Button(ICON_MD_LOOP)){
			setTargetSize(tile->width, tile->height);
		}
		ImGui::PopItemWidth();

		if( TreeNode("Debug")){
			if( Button("Rebuild")){
				setTargetSize(param.targetWidth, param.targetHeight);
			}
			for (r2::Tile* t : { tileTopLeft,tileTopCenter,tileTopRight,tileLeft,tileCenter,tileRight, tileBotLeft,tileBotCenter,tileBotRight }) {
				Text(">");
				ImGui::Image(t, ImVec2(32, 32));
			}
			TreePop();
		}
		ImGui::Unindent();
	}
}

void rd::Slice9::serialize(Pasta::JReflect& f, const char* name){
	Node::serialize(f, name);

	//f.visit(param.tile,"s9tile");
	param.tile.serialize(&f, "tile");

	f.visit(param.marginBottom,"mb");
	f.visit(param.marginTop,"mt");
	f.visit(param.marginLeft,"ml");
	f.visit(param.marginRight,"mr");

	f.visit(param.targetWidth,"tw");
	f.visit(param.targetHeight,"th");

	if (f.isReadMode())
		set(param);
}


rd::Slice9Param::Slice9Param() {
	tile.empty();
}
