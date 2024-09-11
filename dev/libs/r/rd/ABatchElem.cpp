#include "stdafx.h"
#include "../r2/Bitmap.hpp"
#include "../r2/Sprite.hpp"

#include "ABatchElem.hpp"

#include "1-graphics/geo_vectors.h"
#include "1-graphics/Graphic.h"
#include "r2/im/TilePicker.hpp"

using namespace std;
using namespace r2;
using namespace rd;

#define SUPER BatchElem

ABatchElem::ABatchElem(const char * _groupName, TileLib * _lib, r2::Batch * _master) : SUPER(nullptr, _master) {
	if (_lib) 
		lib = _lib;
	if (_groupName)
		groupName = _groupName;

	player.spr = this;
	
	set(lib, groupName.c_str(), 0);
	player.playAndLoop(groupName.c_str());
}

ABatchElem::~ABatchElem() {
	if (beFlags & NF_ORIGINATES_FROM_POOL) {
		int breakOnWrongUse = 0;
	}
	dispose();
}

TileAnimPlayer* rd::ABatchElem::play(const string & g, bool loop){
	return play(g.c_str(),loop);
}

TileAnimPlayer* rd::ABatchElem::play(const Str& g, bool loop) {
	return play(g.c_str(), loop);
}

TileAnimPlayer* rd::ABatchElem::play(const char* g, int nbPlay, bool loop) {
	if (g) groupName = g;
	auto pl = player.play(g,nbPlay);
	if (loop) player.loop();
	return pl;
}

TileAnimPlayer* rd::ABatchElem::play(const char * g, bool loop){
	if (g) groupName = g;
	auto pl = player.play(g);
	if (loop) player.loop();
	return pl;
}

TileAnimPlayer* ABatchElem::playAndLoop(const char * g) {
	if (g) groupName = g;
	auto pl = player.playAndLoop(g);
	return pl;
}

void ABatchElem::replay(int nb){
	player.play(groupName.c_str(), nb);
}

void ABatchElem::replayAndLoop() {
    player.playAndLoop(groupName.c_str());
}

void rd::ABatchElem::stop(){
	player.stop();
}

bool ABatchElem::isReady() {
	return !destroyed && lib != nullptr && groupName.length() && tile;
}

void ABatchElem::update(double dt){
	player.update(dt);
}

void ABatchElem::setWidth(float _w) {
	if (!tile)return;
	scaleX = _w / tile->width;
}

void ABatchElem::setHeight(float _h) {
	if (!tile)return;
	scaleY = _h / tile->height;
}

void ABatchElem::setFrame(int _frame){
	frame = _frame;
	if (isReady()) {
		frameData = lib->getFrameData(groupName.c_str(), frame);
		syncTile();
	}
}

Vector2 ABatchElem::getCenterRatio() {
	if (usePivot)
		return Vector2(pivotX, pivotY);
	else
		return tile->getCenterRatio();
}

void ABatchElem::setCenterRatio(double px, double py){
	pivotX = px;
	pivotY = py;
	usePivot = true;
	syncTile();
}


void rd::ABatchElem::setSpeed(float val){
	player.speed = val;
}

void rd::ABatchElem::mulSpeed(float val){
	player.speed *= val;
}

void rd::ABatchElem::updatePool(double dt){
	int i = rd::Pools::aelems.active.size() - 1;
	for (; i >= 0; i--) 
		rd::Pools::aelems.active[i]->update(dt);
}

BeType rd::ABatchElem::getType() const{
	return BeType::BET_ABATCH_ELEM;
}

ABatchElem * rd::ABatchElem::fromPool(const char * _groupName, TileLib * _lib, r2::Batch * b) {
	auto elem = rd::Pools::aelems.alloc();
	if (_lib)
		elem->lib = _lib;
	if (_groupName)
		elem->groupName = _groupName;

	elem->player.spr = elem;

	elem->set(elem->lib, elem->groupName.c_str(), 0);
	elem->player.playAndLoop(elem->groupName.c_str());
	if (b)
		b->add(elem);
	return elem;
}

void rd::ABatchElem::toPool(){
	dispose();
	rd::Pools::aelems.release(this);
}



void rd::ABatchElem::setLib(TileLib* _lib){
	this->lib = _lib;
	player.frameRate = lib->defaultFrameRate;
}

void ABatchElem::set(TileLib * l, const std::string & g, int frame, bool stopAllAnims) {
	set(l, g.c_str(), frame, stopAllAnims);
}

void ABatchElem::set(TileLib * l, const char * g, int frame, bool stopAllAnims){

	//prevents some issues when tile is fetched and inspected from within a pool
	if (!tile) { 
		tile = r2::Tile::fromWhite()->clone();
		ownsTile = true;
	}

	if (l != nullptr) {
		if (g == nullptr) {
			groupName = "";
			group = nullptr;
		}

		//don't subscribe
		lib = l;
		tile->setTexture( lib->tex );
		//don't subscribe

		if( !usePivot ) setCenterRatio(lib->defaultCenterX, lib->defaultCenterY);
	}

	if (g != nullptr && groupName != g )
		groupName = g;

	if (isReady()) {
		if (stopAllAnims) player.stop();

		group = lib->getGroup(groupName.c_str());
		frameData = lib->getFrameData(groupName.c_str(), frame);

		if (frameData == nullptr) {
#if 0
			if(groupName.length())
				trace("ABE:unknown frame "s + groupName.cpp_str());
#endif
			return;
		}
		
		setFrame(frame);
	}
}

void rd::ABatchElem::reset(){
	Super::reset();

	if (!tile) 
		setTile(rd::Pools::tiles.alloc(),true);

	player.reset();
	
	destroyed = false;
	flippedX = false;
	flippedY = false;
	usePivot = false;

	pivotX = 0.0f;
	pivotY = 0.0f;

	frame = 0;

	frameData = nullptr;
	lib = nullptr;

	groupName.clear();
	//setName("ABatchElem");
}

ABatchElem::ABatchElem(const ABatchElem& o) : BatchElem(o){
	flippedX = o.flippedX;
	flippedY = o.flippedY;
	usePivot = o.usePivot;
	set(o.lib, o.groupName.c_str(), o.frame);
}

r2::BatchElem* rd::ABatchElem::clone(r2::BatchElem* nu) const{
	auto anu = (rd::ABatchElem*) nu;
	if (anu == nullptr)
		anu = rd::Pools::aelems.alloc();

	r2::BatchElem::clone(anu);

	anu->flippedX = flippedX;
	anu->flippedY = flippedY;
	anu->usePivot = usePivot;

	anu->player = player;
	anu->player.spr = anu;

	anu->set(lib, groupName.c_str(), frame);
	anu->setCenterRatio(pivotX,pivotY);
	
	return anu;
}

void ABatchElem::setFlippedX(bool onOff) {
	flippedX= onOff;
	syncTile();
}
void ABatchElem::setFlippedY(bool onOff) {
	flippedY = onOff;
	syncTile();
}

bool ABatchElem::isFlippedX() {
	return flippedX;
}

bool ABatchElem::isFlippedY() {
	return flippedY;
}

ABatchElem* rd::ABatchElem::andDestroy(){
	player.setOnEnd( [=](auto) {
		rd::Garbage::trash(this);
		return true;
	});
	return this;
}

void ABatchElem::dispose() {
	SUPER::dispose();
	player.dispose();

	if( tile ){
		tile->destroy();
		tile = nullptr;
	}
	groupName = "";
	lib = 0;
	pivotX = pivotY = 0;
	frameData = 0;
}

Tile * ABatchElem::syncTile() {
	if (!isReady())
		return tile;

	if (!frameData)
		return nullptr;

	FrameData * fd = frameData;
	tile->setTexture( lib->textures[fd->texSlot] );
	tile->setPos(fd->x, fd->y);
	tile->setSize(fd->wid, fd->hei);

	if (usePivot) {
		tile->dx = -(fd->realFrame.realWid* pivotX + fd->realFrame.x);
		tile->dy = -(fd->realFrame.realHei* pivotY + fd->realFrame.y);
	}

	if(flippedX) tile->flipX();
	if(flippedY) tile->flipY();

	return tile;
}


void ABatchElem::im() {
	using namespace ImGui;
	if (ImGui::CollapsingHeader(ICON_MD_DIRECTIONS_RUN " Animated Batch Element")) {
		ImGui::PushID("BatchElemSettings"); ImGui::PushItemWidth(124);
		ImGui::Indent();

		ImGui::LabelText("group", groupName.c_str());
		if (lib)
			ImGui::Text((std::string("Lib: ") + lib->name).c_str());

		if( Button("Pick anim")){
			auto p = r2::im::TilePicker::forPicker();
			p->then([this](auto p, auto data) {
				auto tp = std::any_cast<r2::im::TilePicker*>(data);
				if (tp) {
					set(tp->pickedLib);
					playAndLoop(tp->pickedGroup->id);
					setCenterRatio(pivotX,pivotY);
				}
				return data;
			});
		}

		auto anm = player.getCurrentAnim();
		bool isLoop = true;
		if (anm)
			isLoop = (anm->plays == -1);
		if (isLoop) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Button));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
		}
		if (ImGui::Button(ICON_MD_LOOP)) {
			isLoop = !isLoop;
			if (anm) anm->plays = isLoop ? -1 : 1;
		}
		ImGui::PopStyleColor(2);
		ImGui::SameLine();

		if (player.isPlaying) {
			if (ImGui::Button(ICON_MD_PAUSE))
				player.pause();
		}
		else {
			if (ImGui::Button(ICON_MD_PLAY_ARROW)) {
				if (anm)
					player.resume();
				else
					replay();
				if (isLoop)
					player.loop();
				else if (anm) anm->plays = 1;
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

		if (Checkbox("Flip X", &flippedX)) 
			setFlippedX(flippedX);
		SameLine();
		if (Checkbox("Flip Y", &flippedY))
			setFlippedY(flippedY);
		
		if (ImGui::DragFloat2("Pivot - ratio", (float*)&pivotX, 0.1f, -1.0f, 1.0f, "%0.2f")) 
			setCenterRatio(pivotX, pivotY);
		ImGui::SameLine();
		if (ImGui::Button(ICON_MD_UNDO))
			setCenterRatio(0, 0);

		double frMin = 1;
		double frMax = 60;
		ImGui::SetNextItemWidth(60);
		ImGui::DragScalar("Frame Rate", ImGuiDataType_Double, &player.frameRate, 0.25, &frMin, &frMax, "%6.3lf");


		if (anm) {
			if (anm->groupName.length())
				ImGui::Text( Str64f("Tile Group: ", anm->groupName));
			else
				if (anm->groupData)
					ImGui::Text( Str64f("Tile Group: %s", anm->groupData->id ));
			double spMin = -1;
			double spMax = 4;
			ImGui::SetNextItemWidth(60);
			ImGui::DragScalar("Play Speed", ImGuiDataType_Double, &player.speed, 0.25, &spMin, &spMax, "%6.3lf");
			if (player.bus) {
				ImGui::Value("Anim Bus", player.bus->speed);
			}
			ImGui::Text("Current frame %i", player.getFrameNumber());
		}

		if (TreeNode("internal")) {
			Indent();
			player.im();
			Unindent();
			TreePop();
		}

		ImGui::PopItemWidth();
		ImGui::Unindent();
		ImGui::PopID();
	}
	r2::BatchElem::im();
}


#undef SUPER