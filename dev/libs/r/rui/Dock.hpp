#pragma once

namespace r2 {
	class Node;
}

namespace rui {
	class IContainer {
	public:
		virtual float	getScaleX() = 0;//returns pixel density x ( may be useful for blur computations )
		virtual float	getScaleY() = 0;//returns pixel density y 
		virtual Vector2 getRefSize() = 0;//returns authoring size of container
		virtual Vector2 getActualSize() = 0;//returns current size for docking in virtual units for ex 1920x1080 if in 4k will return 1920x1080 but parent is likely scale x2
	};

	r2::Node* dock(r2::Node* node, IContainer* ref, r::DIRECTION d, float offset);
	r2::Node* dock(r2::Node* node, IContainer* ref, r::DIRECTION d, Vector2 offset);
}

