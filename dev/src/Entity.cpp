#include "stdafx.h"

#include "Game.hpp"
#include "Entity.hpp"
#include "UserTypes.hpp"
#include "rd/Garbage.hpp"
#include "r2/fx/Part.hpp"

std::vector<Entity*> Entity::ALL;

void Entity::init(EntityData * _data) {
	data = _data;
	if (spr == 0)
		spr = rd::ABitmap::fromPool(Data::assets, data->name.c_str(), this);
	else
		spr->set(Data::assets, data->name.c_str());
	spr->setCenterRatio(0.5, 1);
	if(data->speed==0)
		vars.set("gpType", "tower");
	else 
		vars.set("gpType", "monster");
	name = data->name + std::to_string(uid);
	if (!spr->player.isReady())
		int here = 0;
	hp = data->hp;
	blinking = false;
	fadingOut= false;
	alpha = 1;
}

Entity::Entity(Game*g,r2::Node* parent) : r2::Node(parent){
	game = g;
	ALL.push_back(this);
}

Entity::~Entity() {
	rs::Std::remove(ALL,this);
	dispose();
}

void Entity::im(){
	using namespace ImGui;
	if (TreeNode("data")) {
		Value("name", data->name);
		DragFloat("speed", &data->speed,0.001f,0,10);
		Value("hp", data->hp);
		Value("good", data->good);
		TreePop();
	}

	Value("x", x);
	Value("y", y);

	Value("rx", rx);
	Value("cx", cx);

	Value("ry", ry);
	Value("cy", cy);

	Value("cooldown", cooldown);
	DragFloat("progress", &progress, 0, 10);

	r2::Im::imNodeListEntry("this",this);
	r2::Im::imNodeListEntry("spr",spr);
	Super::im();
}

void Entity::update(double dt) {
	Super::update(dt);
	if( path ){
		auto n = path->data.plot(progress);

		cx = int(n.x / Cst::GRID);
		rx = (1.0f * n.x / Cst::GRID) - cx;

		cy = int(n.y / Cst::GRID);
		ry = (1.0f * n.y / Cst::GRID) - cy;

		if( progress >= 0.999f){
			game->hit();
			rd::Garbage::trash(this);
		}
		progress += dt * data->speed;
	}
	else {
		rx += dt * dx;
		while (rx > 1) { cx++; rx--; }
		while (rx < 0) { cx--; rx++; }

		ry += dt * dy;
		while (ry > 1) { cy++; ry--; }
		while (ry < 0) { cy--; ry++; }
	}

	bool fl = (prevPos - getPixelPos()).x < 0;
	spr->setFlippedX(fl);
	prevPos = getPixelPos();

	if(data->good)
	for(auto e : ALL){
		if (e != this && e->data->isMonster()) {
			Vector2 from = getPos();
			Vector2 to = e->getPos();
			if ((to-from).getNormSquared() < (data->range * data->range))
				fire(e);
		}
	}
	cooldown -= dt;

	syncPos();

	if (fadingOut) {
		spr->alpha -= 0.01f;
		if (spr->alpha < 0)
			rd::Garbage::trash(this);
	}

	if(blinking>0){
		int fr4 = rs::Timer::frameCount % 8;
		if(fr4 <= 4){
			spr->alpha = 1.0f - spr->alpha;
		}
		blinking -= dt;
		if (blinking <= 0)
			spr->alpha = 1;
	}
}

void Entity::setPixelPos(const Vector2& pos) {
	x = pos.x;
	y = pos.y;

	cx = pos.x / Cst::GRID;
	cy = pos.y / Cst::GRID;

	rx = 1.0f * (x - cx * Cst::GRID) / Cst::GRID;
	ry = 1.0f * (y - cy * Cst::GRID) / Cst::GRID;
}

Vector2 Entity::getPixelPos(){
	return Vector2((cx + rx) * Cst::GRID, (cy + ry) * Cst::GRID);
}

void Entity::syncPos(){
	x = std::lrint((cx + rx) * Cst::GRID);
	y = std::lrint((cy + ry) * Cst::GRID);
}

void Entity::hit(int dmg, EntityData* by) {
	hp -= dmg;

	bool dead = hp <= 0;
	if (dead) {
		if (data->isMonster()) {
			game->onFrag();
		}
		if (by == nullptr) {
			rd::Garbage::trash(this);
		}
		else {
			if (by->name == "bike_park") {
				init(Data::entities["bike"]);
				fadingOut = true;
			}
		}
	}
	else {
		blinking = 0.2f;
	}
}

void Entity::fire(Entity*opp) {
	if (cooldown > 0)
		return;
	int here = 0;
	
	auto proj = rd::ABitmap::fromLib(Data::assets, data->attack.c_str(), game->cells);
	proj->name = "bullet";
	proj->vars.set("gpType", "proj");
	proj->x = x;
	proj->y = y;	

	auto p = new r2::fx::Part(proj,&game->al);
	
	Vector2 dir = opp->getPos() - getPos();
	dir = dir.getNormalizedSafeZero();
	float sp = data->projSpeed;
	p->dx = dir.x * sp;
	p->dy = dir.y * sp;
	p->setLife(35);
	p->dr = rd::Rand::get().either( -0.1f ,0.1f);
	p->onUpdate = [=](auto) {
		for (auto e : Entity::ALL) {
			if ( e->data->isMonster() ) {
				Vector2 from = p->getPos();
				Vector2 to = e->getPos();
				int range = 10;
				if ((to - from).getNorm() <= range) {
					p->kill();
					e->hit(data->dmg,data);
					break;
				}
			}
		}
	};

	cooldown = data->cooldown;
}

bool EntityData::isMonster() {
	for(auto &s :tags)
		if (s == "monster")
			return true;
	return false;
}
