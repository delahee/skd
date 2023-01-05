#include "stdafx.h"

#include "r2/Node.hpp"
#include "rui/Canvas.hpp"

#include "r2/StaticBox.hpp"

#include "Game.hpp"
#include "rd/JSerialize.hpp"

Game::Game(r2::Scene* sc, rd::AgentList* parent) : Super(parent) {
	root = new r2::StaticBox( r2::Bounds::fromTLWH(0,0,Cst::W,Cst::H),sc);
	
	Data::init();

	board = new r2::Graphics(root);
	board->name = "board";
	board->y += 10;

	auto b = new r2::Bitmap(r2::Tile::fromWhite(), board);
	b->setSize(Cst::W, Cst::H);

	loadMap();

	r2::Graphics* grid = new r2::Graphics(board);
	for(int y = 0; y < Cst::GRID_H+1;++y)
		for (int x = 0; x < Cst::GRID_W+1; ++x) {
			grid->drawLine(Vector2(x * Cst::GRID, 0), Vector2(x * Cst::GRID, Cst::H),1);
			grid->drawLine(Vector2(0, y * Cst::GRID), Vector2(Cst::W, y * Cst::GRID),1);
		}
	grid->color = r::Color::Orange.mulAlpha(0.5);

	cells = new r2::Node(board);
	auto p = new Path(&al);
	p->cursor = r2::Bitmap::fromColor(r::Color::AcidGreen, cells);
	p->cursor->setSize(Cst::GRID, Cst::GRID);
	p->add( Vector2(Cst::GRID * 0.5, Cst::GRID * 0.5 ));
	p->add( Vector2(Cst::W*0.5 - Cst::GRID * 0.5, Cst::H*0.5 - Cst::GRID * 0.5 ));
	p->add( Vector2(Cst::W - Cst::GRID * 0.5, Cst::H - Cst::GRID * 0.5 ));

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

	
	//put up with one path
	// put up with one enemy
	//add lives
	//add wave
	//add turret points
	//add three turrets
	//add three turret upgrades
	//add particles
	//add cinematics
	//add graphics
	//finished !
}

void Game::update(double dt) {
	Super::update(dt);
	al.update(dt);

	im();	
}

void Tool::save(){
	rs::jSerialize(map, "editor", "map.json","map");
}

void Tool::load() {
	rs::jDeserialize(map, "editor", "map.json", "map");
}

void Game::im(){
	using namespace ImGui;

	static bool opened = true;
	Begin("Game", &opened);
	Value("dt", rs::Timer::dt);
	Value("fps", std::lrint(1.0f / rs::Timer::dt));

	static bool paintMode = false;
	Checkbox("paintmode", &paintMode);

	if(paintMode){

		static int brushSize = 1;
		DragInt("brushSize", &brushSize);
		if (Button("erase"))
			tool.mode = 0;
		if (Button("paint ground"))
			tool.mode = 1;
		SameLine();
		NewLine();

		auto sc = root->getScene();
		auto mpos = root->getGlobalMatrix() * sc->getViewMatrix().inverse() * Vector2(rs::Sys::mouseX, rs::Sys::mouseY) - board->getPos();
		auto cpos = mpos / Cst::GRID;
		Vector2i cposi{ int(cpos.x),int(cpos.y) };

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
		if(tool.mode == 1){
			Value("mpos", mpos);
			Value("cpos", cpos);

			if (Button("Save")) {
				tool.save();
			}

			if (Button("Load")) {
				tool.load();
			}

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
	}
	End();
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
		tb.t = r2::Tile::fromColor(r::Color::AcidGreen);
		tool.painter.push_back(tb);
	}

	{
		TileBrush tb;
		tb.name = "ground";
		tb.t = r2::Tile::fromColor(r::Color(0xDEB05F));
		tool.painter.push_back(tb);
	}

	bg = new r2::Batch(board);
	bg->name = "bg";
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
}


void Path::update(double dt) {
	Super::update(dt);

	float speed = 0.1f;
	if( progress >= data.size()-2){
		progress = 0;
	}
	else {
		progress += dt * speed;
	}

	progress = r::Math::clamp(progress, 0, data.size() - 2);
	reflectProgress(cursor, progress);
}

void Path::reflectProgress(r2::Bitmap* c, float p){
	Vector2 pos = data.plot(p);
	c->x = pos.x;
	c->y = pos.y;
	c->trsDirty = true;
}

