#include "stdafx.h"

#include "Data.hpp"
#include "r2/im/TilePicker.hpp"
#include "Entity.hpp"

rd::TileLib* Data::assets = 0;
std::unordered_map<Str, EntityData*> Data::entities;

void Data::update(double dt) {
	//assets->
}

void Data::init(){
	assets = r2::im::TilePicker::getOrLoadLib("assets.xml");
	int here = 0;


	{
		auto d = new EntityData();
		d->name = "car";
		d->tags.push_back("vehicle");
		d->tags.push_back("monster");
		d->speed = 0.05f;
		entities[d->name] = d;
	}

	{
		auto d = new EntityData();
		d->name = "bike";
		d->tags.push_back("vehicle");
		d->speed = 0.05f;
		entities[d->name] = d;
	}

	{
		auto d = new EntityData();
		d->name = "bike_park";
		d->tags.push_back("turret");
		d->attack = "bike";
		d->projSpeed = 1.5;
		d->speed = 0.0f;
		d->dmg = 5;
		d->good = true;
		d->cooldown = 2;
		entities[d->name] = d;
	}

}