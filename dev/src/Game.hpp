#pragma once
#include "rd/Agent.hpp"
#include "rd/ext/Interp.hpp"
#include "UserTypes.hpp"
#include "rgp/Cine.hpp"
class Game;
class rgp::CineController;

namespace r2 { class Node; }
namespace rui { class Canvas; }

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

struct Wave{
	Game* g=0;
	rgp::CineController * cc=0;
	bool stopped = false;
	void stop();
};

class Game : rd::Agent {
public:
	typedef rd::Agent Super;
	Tool			tool;
	r2::Node*		scRoot = 0;
	r2::Node*		root = 0;
	r2::Node*		board = 0;
	r2::Node*		cells = 0;
	rd::AgentList	al;

	r2::Batch*		bg;

	rd::ABitmap*	bossPortrait = 0;
	rd::ABitmap*	kiwiPortrait = 0;

	Path*			path=0;
	int				frags = 0;
	r2::Flow*		livesFlow = 0;
	r2::Flow*		fragFlow = 0;
	rd::Tweener		tw;

	Wave*			wave1 = 0;
	Wave*			curWave = 0;
	void			intro();
	void			sfx(const char* name);
	void			beginGame();
	void			victory();
	void			defeat();
	void			hit();

					Game(r2::Node * root,r2::Scene* sc, rd::AgentList* parent);
	virtual void	update(double dt);

	void			spawn(const char*c) { spawn(StrRef(c)); };
	void			spawn(Str& sp);
	bool			im();

	void			loadMap();
	void			dressMap();

	void			startWave();

	void			onFrag();
};

