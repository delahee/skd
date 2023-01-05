#include "stdafx.h"

#include "Data.hpp"
#include "r2/im/TilePicker.hpp"

rd::TileLib* Data::assets = 0;

void Data::update(double dt) {
	//assets->
}

void Data::init(){
	assets = r2::im::TilePicker::getOrLoadLib("assets.xml");
	int here = 0;

}