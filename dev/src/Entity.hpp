#pragma once

#include "r2/Node.hpp"

struct EntityData{
	std::string		name;
	std::string		attack;
	std::string		family;

	bool			good = false;
	float			speed = 1;
	int				hp = 10;
	int				maxUpgrade = 0;
};

class Entity : public r2::Node {
public:
	typedef r2::Node Super;
	EntityData*		data = 0;
	Path*			path = 0;
	rd::ABitmap*	spr = 0;
	Game*			game = 0;
	float			progress = 0;

	float			rx = 0;
	float			ry = 0;

	int				cx = 0;
	int				cy = 0;

	int				dx = 0;
	int				dy = 0;

	int				upgrade = 0;

	Vector2	prevPos;

	enum class State : int {
		Running,
	};

	void init(EntityData*data);

					Entity(Game*g,r2::Node* parent);
					void im();
	virtual void	update(double dt);
	Vector2			getPixelPos();
	void			syncPos();
};