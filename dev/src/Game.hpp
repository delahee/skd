#pragma once
#include "rd/Agent.hpp"
#include "rd/ext/Interp.hpp"

class Game;

namespace r2 { class Node; }
namespace rui { class Canvas; }


struct Path : rd::Agent {
	Game* g = 0;

	typedef rd::Agent Super;
	Path(rd::AgentList* al) : Super(al) {

	};

	r2::Bitmap*				cursor = 0;
	float					progress = 0;

	rd::ext::CurveC2		data;

	void add(const r::Vector2& p) { data.data.push_back(p); };
	void update(double dt);
	void reflectProgress(r2::Bitmap* c, float p);
	void debugDraw();
};

struct TileBrush {
	std::string	name;
	r2::Tile*	t = nullptr;
};

enum class PaintMode : int {
	Erase,
	Ground,
	Path,
	Tower,
	None = -1
};


struct Tool{
	bool								allowTilePaint = true;
	int									mode = int(PaintMode::None);

	int									brush = 0;

	eastl::vector<int>					map;
	std::vector<Vector2>				towerSpot;
	eastl::vector<TileBrush>			painter;

	Game*								g=0;

	static inline int			cell2int(int x,int y){
		return (y << 8) | x;
	}
	void save();
	void load();
	//void visit(Pasta::JReflect& jr);
};


class Game : rd::Agent {
public:
	typedef rd::Agent Super;
	Tool			tool;
	r2::Node*		root = 0;
	r2::Node*		board = 0;
	r2::Node*		cells = 0;
	rd::AgentList	al;

	r2::Batch*		bg;

	rd::ABitmap*	bossPortrait = 0;
	rd::ABitmap*	kiwiPortrait = 0;

	Path*			path=0;

	r2::Flow*		livesFlow = 0;

	void			victory();
	void			defeat();
	void			hit();

					Game(r2::Scene* sc, rd::AgentList* parent);
	virtual void	update(double dt);

	void			im();

	void			loadMap();
	void			dressMap();
};

