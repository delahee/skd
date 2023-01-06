#include "stdafx.h"

#include <unordered_map>

#include "r2/Node.hpp"
#include "rui/Canvas.hpp"
#include "r2/StaticBox.hpp"

#include "Game.hpp"
#include "rd/JSerialize.hpp"
#include <EASTL/variant.h>
#include "rd/Garbage.hpp"
#include "Entity.hpp"

static std::unordered_map<Str, EntityData*> edata;


void Game::defeat(){
	auto sc = root->getScene();
	auto n = new r2::Node(sc);
	n->x = sc->getPanX();
	n->y = sc->getPanY();
	auto r = r2::Bitmap::fromColor(r::Color::Red, n);
	r->setSize(rs::Display::width(),rs::Display::height());
	auto txt = new r2::Text(nullptr,"DEFEAT!", n);
	txt->setBlockAlign(r2::Text::ALIGN_CENTER);
	txt->x = 280;
	txt->y = 150;
	txt->scaleX = txt->scaleY = sc->getZoomY();
}

void Game::victory() {
	auto sc = root->getScene();
	auto n = new r2::Node(sc);
	n->x = sc->getPanX();
	n->y = sc->getPanY();
	auto r = r2::Bitmap::fromColor(r::Color::Green, n);
	r->setSize(rs::Display::width(), rs::Display::height());
	auto txt = new r2::Text(nullptr, "VICTORY!", n);
	txt->setBlockAlign(r2::Text::ALIGN_CENTER);
	txt->x = 280;
	txt->y = 150;
	txt->scaleX = txt->scaleY = sc->getZoomY();
}

void Game::hit() {
	if( 0 == livesFlow->nbChildren()){
		defeat();
	}
	else {
		livesFlow->children[0]->destroy();
	}
}

Game::Game(r2::Scene* sc, rd::AgentList* parent) : Super(parent) {
	root = new r2::StaticBox( r2::Bounds::fromTLWH(0,0,Cst::W,Cst::H),sc);
	
	Data::init();

	tool.g = this;
	board = new r2::Graphics(root);
	board->name = "board";
	board->y += 10;

	auto b = new r2::Bitmap(r2::Tile::fromWhite(), board);
	b->setSize(Cst::W, Cst::H);

	r2::Graphics* grid = new r2::Graphics(board);//create layers
	bg = new r2::Batch(board);
	bg->name = "bg";
	cells = new r2::Node(board);

	{
		auto p = path = new Path(&al);
		p->g = this;
		p->cursor = r2::Bitmap::fromColor(r::Color::Cyan, cells);
		p->cursor->setSize(Cst::GRID, Cst::GRID);
		p->add(Vector2(Cst::GRID * 0.5, Cst::GRID * 0.5));
		p->add(Vector2(Cst::W * 0.5 - Cst::GRID * 0.5, Cst::H * 0.5 - Cst::GRID * 0.5));
		p->add(Vector2(Cst::W - Cst::GRID * 0.5, Cst::H - Cst::GRID * 0.5));
	}
	loadMap();
	
	for(int y = 0; y < Cst::GRID_H+1;++y)
		for (int x = 0; x < Cst::GRID_W+1; ++x) {
			grid->drawLine(Vector2(x * Cst::GRID, 0), Vector2(x * Cst::GRID, Cst::H),1);
			grid->drawLine(Vector2(0, y * Cst::GRID), Vector2(Cst::W, y * Cst::GRID),1);
		}
	grid->color = r::Color::Orange.mulAlpha(0.5);

	

	bossPortrait = rd::ABitmap::fromLib(Data::assets,"pixel",root);
	bossPortrait->setCenterRatio(0.5, 1);
	bossPortrait->setSize(64, 64);
	bossPortrait->x = Cst::W * 0.5;
	bossPortrait->y += 30;
	bossPortrait->backward(1);

	kiwiPortrait = rd::ABitmap::fromLib(Data::assets, "kiwi", root);
	bossPortrait->setCenterRatio(0.5, 1);
	kiwiPortrait->setScale(4, 4);
	kiwiPortrait->x = -54;
	kiwiPortrait->y = 167;
	kiwiPortrait->player.speed = 0.66f;

	livesFlow = new r2::Flow(root);
	for(int i = 0; i < 5; i++){
		auto ra = rd::ABitmap::mk("kiwi_head", Data::assets, livesFlow);
		ra->setCenterRatio();
	}
	livesFlow->x = 32;
	livesFlow->y = 270;
	livesFlow->horizontalSpacing = 10;
	livesFlow->reflow();


	{
		auto d = new EntityData();
		d->name = "car";
		d->tags.push_back("vehicle");
		d->tags.push_back("monster");
		d->speed = 0.1f;
		edata[d->name] = d;
	}

	{
		auto d = new EntityData();
		d->name = "bike_park";
		d->tags.push_back( "turret" );
		d->speed = 0.0f;
		d->dmg = 5;
		edata[d->name] = d;
	}

	//TODO
	//add one turret
	//add wave definition
	//add title screen
	//add credits.txt
	//add sound
	//add cinematics
	//add two more ennemies
	//add three turrets
	//add three turret upgrades
	//add particles
	
	//finished !
}

void Game::update(double dt) {
	Super::update(dt);
	al.update(dt);

	im();	
}

template <> void Pasta::JReflect::visit(std::vector<Vector2> & v, const char* name) {
	u32 arrSize = v.size()*2;
	if (visitArrayBegin(name, arrSize)) {
		if (isReadMode())
			if (v.size() < arrSize >> 1)
				v.resize(arrSize >> 1);
		for (u32 i = 0; i < arrSize; ++i) {
			visitIndexBegin(i);
			Vector2& vf = v[i>>1];
			if( 0==(i & 1) )
				visit(vf.x, nullptr);
			else
				visit(vf.y, nullptr);
			visitIndexEnd();
		}
	}
	visitArrayEnd(name);
}

static void visitEastl(Pasta::JReflect&jr,eastl::vector<Vector2>& v, const char* name) {
	u32 arrSize = v.size() * 2;
	if (jr.visitArrayBegin(name, arrSize)) {
		if (jr.isReadMode())
			if( v.size() < arrSize >>1)
				v.resize(arrSize >> 1);
		for (u32 i = 0; i < arrSize; ++i) {
			jr.visitIndexBegin(i);
			Vector2& vf = v[i >> 1];
			if (0 == (i & 1))
				jr.visit(vf.x, nullptr);
			else
				jr.visit(vf.y, nullptr);
			jr.visitIndexEnd();
		}
	}
	jr.visitArrayEnd(name);
}

template<> void Pasta::JReflect::visit(Tool&t, const char* name) {
	visit(t.map, "map");
	visit(t.towerSpot, "towerSpot");
	visitEastl(*this,t.g->path->data.data, "path");
}

void Tool::save(){
	rs::jSerialize(*this, "editor", "map.json", "all");
}

void Tool::load() {
	rs::jDeserialize(*this, "editor", "map.json", "all");
}


void Game::spawn(Str& sp) {
	auto e = new Entity(this, cells);
	EntityData* data = edata[sp];
	e->init(data);
	e->path = path;
}

void Game::im(){
	static bool wasMousePressed = false;

	using namespace ImGui;

	static bool opened = true;

	bool isMouseJustPressed = !wasMousePressed && rs::Sys::isMousePressed;
	Begin("Game", &opened);
	Value("dt", rs::Timer::dt);
	Value("fps", std::lrint(1.0f / rs::Timer::dt));

	if(TreeNodeEx("action",ImGuiTreeNodeFlags_DefaultOpen)){
		if (Button("defeat"))
			defeat();
		if (Button("victory"))
			victory();

		if (Button("spawn monster")) {
			spawn(StrRef("car"));
		}
		TreePop();
	}

	if (TreeNodeEx("entities", ImGuiTreeNodeFlags_DefaultOpen)) {
		for(auto e : cells->children){
			auto asEnt = dynamic_cast<Entity*>(e);
			if (!asEnt) continue;;

			if (TreeNode(asEnt->name.c_str())) {
				asEnt->im();
				TreePop();
			}
		}
		TreePop();
	}

	static bool paintMode = false;
	Checkbox("paintmode", &paintMode);

	if(paintMode){

		static int brushSize = 1;
		DragInt("brushSize", &brushSize);
		if (Button("erase"))
			tool.mode = (int)PaintMode::Erase;
		SameLine();
		if (Button("pnt ground"))
			tool.mode = (int)PaintMode::Ground;
		SameLine();
		if (Button("pnt path"))
			tool.mode = (int)PaintMode::Path;
		SameLine();
		if (Button("pnt towers"))
			tool.mode = (int)PaintMode::Tower;
		SameLine();
		NewLine();

		auto sc = root->getScene();
		auto mpos = root->getGlobalMatrix() * sc->getViewMatrix().inverse() * Vector2(rs::Sys::mouseX, rs::Sys::mouseY) - board->getPos();
		auto cpos = mpos / Cst::GRID;
		Vector2i cposi{ int(cpos.x),int(cpos.y) };

		Value("mpos", mpos);
		Value("cpos", cpos);

		if (tool.mode == 0) {
			if (rs::Sys::isMousePressed) {
				if (cposi.x >= 0 && cpos.y >= 0) {

					for(int sy = 0; sy<brushSize;sy++ )
						for (int sx = 0; sx < brushSize; sx++) {
							int k = Tool::cell2int(cposi.x + sx, cposi.y + sy);
							if (tool.map.size() <= k)
								tool.map.resize(k + 1);
							tool.map[k] = 0;
						}
					dressMap();
					tool.save();
				}
			}
		}
		if(tool.mode == (int)PaintMode::Ground){
			for (int i = 0; i < tool.painter.size(); i++) {
				if( ImGui::RadioButton(tool.painter[i].name.c_str(), tool.brush ==i)){
					tool.brush = i;
				}
			}

			if(rs::Sys::isMousePressed){
				if (cposi.x >= 0 && cpos.y >= 0) {

					for (int sy = 0; sy < brushSize; sy++)
						for (int sx = 0; sx < brushSize; sx++) {
							int k = Tool::cell2int(cposi.x+sx, cposi.y+sy);
							if (tool.map.size() <= k)
								tool.map.resize(k + 1);
							tool.map[k] = tool.brush;
						}
					tool.save();
					dressMap();
				}
			}
		}

		if(tool.mode == (int)PaintMode::Path){
			DragFloat("progress",&path->progress);
			if (Button("clear")) {
				path->data.data.clear();
				tool.save();
			}

			if (isMouseJustPressed && cpos.x >= 0 && cpos.y >= 0) {

				Vector2 dst = Vector2((cposi.x + 0.5) * Cst::GRID, (cposi.y + 0.5) * Cst::GRID);

				auto& buf = path->data.data;
				auto iter = buf.begin();
				while (iter != buf.end()) {

					if (( (*iter) - dst ).getNorm() < 5)
						break;
					iter++;
				}

				if(iter != buf.end())
					path->data.data.erase(iter);
				else 
					path->data.data.push_back(dst);
				tool.save();
			}

			path->debugDraw();
		}

		if (tool.mode == (int)PaintMode::Tower) {
			if (isMouseJustPressed) {
				if (cpos.x >= 0 && cpos.y >= 0) {
					auto iter = tool.towerSpot.begin();
					while(iter!= tool.towerSpot.end()){
						if (((*iter) - mpos).getNorm() < 10)
							break;
						iter++;
					}
					if (iter == tool.towerSpot.end())
						tool.towerSpot.push_back(mpos);
					else
						tool.towerSpot.erase(iter);

					tool.save();
					dressMap();
				}
			}
		}

		if (Button("Save")) {
			tool.save();
		}

		if (Button("Load")) {
			tool.load();
		}
	}
	End();

	wasMousePressed = rs::Sys::isMousePressed;
}

void Game::loadMap() {
	{
		TileBrush tb;
		tb.name = "transp";
		tb.t = nullptr;
		tool.painter.push_back(tb);
	}

	{
		TileBrush tb;
		tb.name = "grass";
		tb.t = r2::Tile::fromColor(r::Color(0x24A03E));
		tool.painter.push_back(tb);
	}

	{
		TileBrush tb;
		tb.name = "ground";
		tb.t = r2::Tile::fromColor(r::Color(0xDEB05F));
		tool.painter.push_back(tb);
	}

	tool.load();
	dressMap();
}

void Game::dressMap(){
	bg->destroyAllElements();

	for( int i = 0; i < tool.map.size();++i){
		int x = i & 255;
		int y = (i >> 8);

		auto& idx = tool.map[i];
		auto& tb = tool.painter[idx];
		if(tb.t == nullptr)
			continue;
		auto el = bg->alloc();
		el->setTile(tb.t);
		el->ownsTile = false;
		el->setSize(Cst::GRID, Cst::GRID);
		el->x = x * Cst::GRID;
		el->y = y * Cst::GRID;
	}

	for (auto b : cells->children) 
		if( b->vars.getStr("gpType") == "towerSpot")
			rd::Garbage::trash(b);
	
	for (auto &sp : tool.towerSpot) {
		auto b = r2::Node::fromPool(cells);
		b->vars.set("gpType", "towerSpot");
		b->x = (int)sp.x;
		b->y = (int)sp.y;

		auto c = rd::ABitmap::mk("partCircle", Data::assets, b);
		c->name = "foundation";
		c->setCenterRatio(0.5, 0.5);
		float s = 1.4;
		c->setSize(s*Cst::GRID, s*Cst::GRID / 1.414f);
		c->color = r::Color(0xeec39a);

		auto h = rd::ABitmap::mk("hammer", Data::assets, b);
		h->name = "hammer";
		h->setCenterRatio();
		h->bhv = [=](auto) {
			int ofs = 7;
			h->y = (int)(-ofs +  sin(h->uid + rs::Timer::now * 4) * 3);
		};
		r2::Interact* it = new r2::Interact(b);
		it->x = -8;
		it->y = -16;
		it->rectWidth = h->width();
		it->rectHeight = 25;
		it->name = "it";

		//todo r2::Graphics::circle( elem );
		it->onMouseOvers.push_back([=](auto & ev) {
			if( !b->findByName("sel")){
				auto c = r2::Graphics::fromPool(b);
				c->name = "sel";
				c->setGeomColor(r::Color::Red);
				c->drawCircle(0, 0, 12, 2);
				c->toBack();
			}
		});

		it->onMouseButtonDowns.push_back([=](auto&ev) {
			if( b->vars.getStr("gpType") == "towerSpot"){
				b->vars.set("gpType", "tower");

				//delete old
				rd::ABitmap* ab = (rd::ABitmap *) h->findByName("hammer");
				rd::Garbage::trash(ab);

				//turn in tower
				//auto h = rd::ABitmap::mk("bikePark", Data::assets, b);
				//h->name = "bikePark";
				//h->setCenterRatio();
				auto e = new Entity(this, b);
				e->init(edata["bike_park"]);
				e->spr->setCenterRatio();

				//
				auto sel = b->findByName("sel");
				if (sel) rd::Garbage::trash(sel);
			}
		});

		it->onMouseOuts.push_back([=](auto& ev) {
			auto sel = b->findByName("sel");
			if (sel) rd::Garbage::trash(sel);
		});
	}
}


void Path::update(double dt) {
	Super::update(dt);

	if (data.size() < 2)
		return;

	float speed = 0.1f;
	if( progress >= 1.0f){
		progress = 0;
	}
	else {
		progress += dt * speed;
	}

	progress = r::Math::clamp(progress, 0, 1.0f);
	reflectProgress(cursor, progress);
}

void Path::reflectProgress(r2::Bitmap* c, float p){
	Vector2 pos = data.plot(p);
	c->x = pos.x;
	c->y = pos.y;
	c->trsDirty = true;
}

void Path::debugDraw() {
	for(int i = 0; i < data.size();++i){
		Vector2 p = data.data[i];
		r2::Im::outerRect( r2::Bounds::fromTLBR(p.y-1,p.x-1,p.y+1,p.x+1), r::Color::Blue, g->cells);
		for(int j = 0; j < 50;++j){
			
		}
	}
}

