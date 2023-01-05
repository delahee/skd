#include "stdafx.h"

#include "Game.hpp"
#include "Entity.hpp"
#include "UserTypes.hpp"
#include "rd/Garbage.hpp"

void Entity::init(EntityData * _data) {
	data = _data;
	spr = rd::ABitmap::fromPool(Data::assets,data->name.c_str(),this);
	spr->setCenterRatio(0.5, 1);
	if(data->speed==0)
		vars.set("gpType", "tower");
	else 
		vars.set("gpType", "monster");
	name = data->name + std::to_string(uid);

}

Entity::Entity(Game*g,r2::Node* parent) : r2::Node(parent){
	game = g;
	
}

void Entity::im(){
	using namespace ImGui;
	if (TreeNode("data")) {
		Value("name", data->name);
		DragFloat("speed", &data->speed,0.001,0,10);
		Value("hp", data->hp);
		Value("good", data->good);
		TreePop();
	}

	Value("rx", rx);
	Value("cx", cx);

	Value("ry", ry);
	Value("cy", cy);
	DragFloat("progress", &progress, 0, 10);

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
		ry += dt * dy;

		//no coll so far ?
	}

	bool fl = (prevPos - getPixelPos()).x < 0;
	spr->setFlippedX(fl);
	prevPos = getPixelPos();

	syncPos();
}

Vector2 Entity::getPixelPos(){
	return Vector2((cx + rx) * Cst::GRID, (cy + ry) * Cst::GRID);
}

void Entity::syncPos(){
	spr->x = std::lrint((cx + rx) * Cst::GRID);
	spr->y = std::lrint((cy + ry) * Cst::GRID);
}
