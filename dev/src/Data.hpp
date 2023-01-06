#pragma once
#include <unordered_map>

struct EntityData;

class Data {
public:
	static rd::TileLib* assets;
	static std::unordered_map<Str, EntityData*> 
						entities;
	static void			init();
	static void			update(double dt);

};