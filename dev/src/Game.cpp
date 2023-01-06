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
#include "r2/fx/Part.hpp"
#include "FX.hpp"

static int towerEverBuild = 0;
static r::Color KIWI = r::Color(0x663931);

void Game::onFrag(){
	frags++;

	if (frags % 3 == 0) {
		auto s = rd::ABitmap::mk("kiwifruit", Data::assets, fragFlow);
		s->setCenterRatio(0.5, 1);
		FX::blink(s);
		sfx("snd/kiwi_up.wav");
	}
}

void Wave::stop() {
	if(cc)
		cc->pause();
	stopped = true;
}

static bool defeated = false;
void Game::defeat(){
	if (defeated)
		return;

	defeated = true;
	cells->visible = false;

	auto sc = root->getScene();
	auto stage = sc->findByName("stage");
	auto w = rs::Display::width() / sc->getZoomX();
	auto h = rs::Display::height() / sc->getZoomY();
	auto n = new r2::Node(stage);
	auto r = r2::Bitmap::fromColor(r::Color::Red, n);
	r->setSize(rs::Display::width(),rs::Display::height());
	auto txt = new r2::Text(nullptr,"DEFEAT!", n);
	txt->setBlockAlign(r2::Text::ALIGN_CENTER);
	txt->setTextColor(r::Color::Black);
	txt->addOutline(KIWI);
	txt->x = w*0.5;
	txt->y = h * 0.5;	
	txt->scaleX = txt->scaleY = sc->getZoomY();
	if(curWave)curWave->stop();
	sfx("snd/loose.wav");
	rs::Timer::delay(1000, [=]() { sfx("snd/laugh.wav"); });

	auto bp = rd::ABitmap::fromLib(Data::assets, "elon", n);
	bp->setScale(2,2);
	bp->setCenterRatio(0.5, 1);
	bp->x = w*0.5;
	bp->y = h+40;
	tw.create(bp,VY, h+10,TType::TEaseIn);

}

void Game::intro(){
	auto sc = root->getScene();
	auto n = r2::Node::fromPool(sc->getByName("stage"));
	auto b = r2::Bitmap::fromColor(r::Color::Red, n);
	auto w= rs::Display::width() / sc->getZoomX();
	auto h = rs::Display::height() / sc->getZoomY();
	b->setSize(w,h);
	auto t = r2::Text::fromPool(nullptr, "SUPER\nKIWI DEFENSE", r::Color::Green, n);
	t->setBlockAlign(r2::Text::ALIGN_CENTER);
	t->setScale(3, 3);
	t->x = w * 0.5f;
	t->y = h * 0.33f;
	t->addOutline(KIWI);

	auto ab= rd::ABitmap::fromPool(Data::assets, "kiwi", n);
	ab->setCenterRatio(0.5,1);
	ab->x = w * 0.33f;
	ab->y = h * 0.96f;
	ab->setScale(8, 8);

	auto e = r2::Text::fromPool(nullptr, "CLICK TO CONTINUE", r::Color::Green, n);
	e->x = w * 0.5f;
	e->y = h * 0.66f;
	e->setScale(2, 2);

	cells->visible = false;

	static bool exiting = false;
	e->bhv = [=](auto) {
		e->alpha = 1.0f * ((rs::Timer::frameCount % 16) <= 8);
		if( !exiting&&rs::Sys::isMousePressed){
			cells->visible = true;

			sfx("snd/intro.wav");
			beginGame();
			auto p = tw.create(n, VAlpha, 0, TType::TBurnIn, 600);
			p->onEnd = [=](auto) {
				n->destroy();
			};
			exiting = true;
		}
	};

}

void Game::sfx(const char * name){
	rd::AudioMan::get().playFile(name);
}

void Game::beginGame(){
	auto defy = bossPortrait->y;
	bossPortrait->y += 25;
	tw.create(bossPortrait, VY, defy, TType::TEaseOut, 500);
	bossPortrait->replay(10);
	startWave();
}

void Game::victory() {

	cells->visible = false;
	auto sc = root->getScene();
	auto stage = sc->findByName("stage");
	auto w = rs::Display::width() / sc->getZoomX();
	auto h = rs::Display::height() / sc->getZoomY();
	auto n = new r2::Node(stage);
	auto r = r2::Bitmap::fromColor(r::Color::Green, n);
	r->setSize(rs::Display::width(), rs::Display::height());
	auto txt = new r2::Text(nullptr, "VICTORY!", n);
	txt->setBlockAlign(r2::Text::ALIGN_CENTER);
	txt->x = w*0.5f;
	txt->y = h*0.5f;
	txt->z = -10;
	txt->scaleX = txt->scaleY = sc->getZoomY();
	txt->addOutline(r::Color(0x663931));
	auto c = rd::ABitmap::fromPool( Data::assets, "cup", n);
	c->setCenterRatio();
	c->y = txt->y+ 50;
	c->x = txt->x;
	c->setScale(2, 2);
	tw.create(c, VY, c->y - 10,TType::TEaseOut,-1);
	if (curWave)
		curWave->stop();

	double del = 800;
	sfx("snd/vict.wav"); 
	rs::Timer::delay(del, [=]() { sfx("snd/vict.wav"); });
	rs::Timer::delay(del*2, [=]() { sfx("snd/vict.wav"); });
	rs::Timer::delay(del*3, [=]() { sfx("snd/vict.wav"); });
}

void Game::hit() {
	sfx("snd/hitplayer.wav");
	if( 0 == livesFlow->nbChildren())
		defeat();
	else {
		livesFlow->children[0]->destroy();
	}
}



Game::Game(r2::Node* _root, r2::Scene* sc, rd::AgentList* parent) : Super(parent) {
	scRoot = _root;
	root = new r2::StaticBox(r2::Bounds::fromTLWH(0, 0, Cst::W, Cst::H), scRoot);

	Data::init();

	rd::AudioMan::get().init();

	tool.g = this;
	board = new r2::Graphics(root);
	board->name = "board";
	board->y += 15;

#if 0
	auto b = new r2::Bitmap(r2::Tile::fromWhite(), board);
	b->setSize(Cst::W, Cst::H);
#endif
	r2::Graphics* grid = new r2::Graphics(board);//create layers
	bg = new r2::Batch(board);
	bg->name = "bg";
	cells = new r2::Node(board);
	cells->name = "cells";

	{
		auto p = path = new Path(&al);
		p->g = this;
		p->cursor = r2::Bitmap::fromColor(r::Color::Cyan, cells);
		p->cursor->setSize(Cst::GRID, Cst::GRID);
		p->add(Vector2(Cst::GRID * 0.5, Cst::GRID * 0.5));
		p->add(Vector2(Cst::W * 0.5 - Cst::GRID * 0.5, Cst::H * 0.5 - Cst::GRID * 0.5));
		p->add(Vector2(Cst::W - Cst::GRID * 0.5, Cst::H - Cst::GRID * 0.5));
#ifndef PASTA_DEBUG
		p->cursor->visible = false;
#endif
	}
	loadMap();

#if 0
	for (int y = 0; y < Cst::GRID_H + 1; ++y)
		for (int x = 0; x < Cst::GRID_W + 1; ++x) {
			grid->drawLine(Vector2(x * Cst::GRID, 0), Vector2(x * Cst::GRID, Cst::H), 1);
			grid->drawLine(Vector2(0, y * Cst::GRID), Vector2(Cst::W, y * Cst::GRID), 1);
		}
	grid->color = r::Color::Orange.mulAlpha(0.5);
#endif


	bossPortrait = rd::ABitmap::fromLib(Data::assets, "elon", root);
	bossPortrait->setCenterRatio(0.5, 1);
	bossPortrait->x = Cst::W * 0.5;
	bossPortrait->y += 20;
	bossPortrait->backward(1);
	bossPortrait->player.stop();

	kiwiPortrait = rd::ABitmap::fromLib(Data::assets, "kiwi", root);
	bossPortrait->setCenterRatio(0.5, 1);
	kiwiPortrait->setScale(4, 4);
	kiwiPortrait->x = -54;
	kiwiPortrait->y = 167;
	kiwiPortrait->player.speed = 0.66f;

	livesFlow = new r2::Flow(root);
	for (int i = 0; i < 5; i++) {
		auto ra = rd::ABitmap::mk("kiwi_head", Data::assets, livesFlow);
		ra->setCenterRatio();
	}
	livesFlow->x = 32;
	livesFlow->y = 270;
	livesFlow->horizontalSpacing = 10;
	livesFlow->reflow();

	fragFlow = new r2::Flow(root);
	fragFlow->name = "fragFlow";
	fragFlow->x = Cst::W;
	fragFlow->y = 270;
	fragFlow->horizontalSpacing = -20;
	fragFlow->reflow();

	//add two starting fruit
	{
		auto s = rd::ABitmap::mk("kiwifruit", Data::assets, fragFlow);
		s->setCenterRatio(0.5, 1);
		s = rd::ABitmap::mk("kiwifruit", Data::assets, fragFlow);
		s->setCenterRatio(0.5, 1);
	}

	wave1 = new Wave();
	auto cc = wave1->cc = new rgp::CineController("wave1", &al);

	auto slowWave3 = [=]() { for (int i = 0; i < 3; ++i) {
		cc->add([=]() { spawn("car");  });
		cc->waitForSeconds(3);
	}};

	auto fastWave3 = [=]() { for (int i = 0; i < 4; ++i) {
		cc->add([=]() { spawn("car");  });
		cc->waitForSeconds(1);
	}};

	slowWave3();
	fastWave3();
	slowWave3();

	slowWave3();
	fastWave3();
	slowWave3();

	fastWave3();
	fastWave3();
	fastWave3();

	cc->waitForSeconds(20);
	cc->add([=]() {victory(); });

#ifndef PASTA_DEBUG
	intro();
#endif
	//add sound
	//add two more ennemies
	//add three turrets
	//add three turret upgrades
	//add particles
	
	//finished !
}

void Game::startWave(){
	auto cc = wave1->cc;
	cc->start();
	curWave = wave1;

	sfx("snd/8bitsurf.mp3");
}

void Game::update(double dt) {
	Super::update(dt);
	al.update(dt);
	tw.update(dt);
#ifdef PASTA_DEBUG
	im();	
#endif
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
	EntityData* data = Data::entities[sp];
	e->init(data);
	e->path = path;
	bossPortrait->replay(3);
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
		if (Button("intro"))
			intro();
		if (Button("defeat"))
			defeat();
		if (Button("victory"))
			victory(); 
		
		if (Button("start wave1"))
			startWave();

		if (Button("stop"))
			if (curWave)
				curWave->stop();

		if (Button("spawn monster")) {
			spawn(StrRef("car"));
		}
		TreePop();
	}

	if (TreeNodeEx("entities", ImGuiTreeNodeFlags_DefaultOpen)) {
		for(auto e : Entity::ALL){
			auto asEnt = dynamic_cast<Entity*>(e);
			if (!asEnt) continue;;

			if (TreeNode(asEnt->name.c_str())) {
				asEnt->im();
				TreePop();
			}
		}
		TreePop();
	}

	if (TreeNodeEx("Parts", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (auto p : r2::fx::Part::ALL) {
			if (TreeNode(p->name.c_str())) {
				p->im();
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
		float s = 1.4f;
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
				sfx("snd/hover.wav");

				if(towerEverBuild == 0){
					auto hint = ri18n::RichText::mk("*Click* to build a **bike park**\n**bike parks** converts cars to *bikes*",
						b->parent);
					hint->name = "hint";
					hint->y += 8;
					hint->addOutline(KIWI);
					hint->x += b->x;
					hint->y += b->y;
				}
				if (towerEverBuild == 1) {
					auto hint = ri18n::RichText::mk("*bikes* are cleaner than cars\na cleaner world is healthier for *kiwis*",
						b->parent);
					hint->name = "hint";
					hint->y += 8;
					hint->addOutline(KIWI);
					hint->x += b->x;
					hint->y += b->y;
				}
			}
		});

		it->onMouseButtonDowns.push_back([=](auto&ev) {
			if( b->vars.getStr("gpType") == "towerSpot"){

				if(fragFlow->children.empty()){
					auto msg = ri18n::RichText::mk( "Not enough *kiwi*\nto build a **bike park**",cells);
					msg->x = b->x;
					msg->y = b->y;
					msg->addOutline(KIWI);
					msg->centered();
					tw.delay(400,msg, VY, msg->y - 10);
					tw.delay(400,msg, VAlpha, 0);
					sfx("snd/click.wav");

					return;
				}


				fragFlow->children[0]->destroy();
				b->vars.set("gpType", "tower");

				//delete old
				rd::ABitmap* ab = (rd::ABitmap *) h->findByName("hammer");
				rd::Garbage::trash(ab);

				//turn in tower
				auto e = new Entity(this, cells);
				e->init(Data::entities["bike_park"]);
				e->setPixelPos(b->getPos());
				e->spr->setCenterRatio();

				sfx("snd/build.wav");

				towerEverBuild++;

				//
				auto sel = b->findByName("sel");
				if (sel) rd::Garbage::trash(sel);
				auto hint = b->parent->findByName("hint");
				if (hint) hint->destroy();
			}
		});

		it->onMouseOuts.push_back([=](auto& ev) {
			auto sel = b->findByName("sel");
			if (sel) rd::Garbage::trash(sel);
			auto hint = b->parent->findByName("hint");
			if (hint) hint->destroy();
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
	}
}

