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

	/**
	* Docking will not take internal pivot into account, 
	* it will make its best to respect docking propertis for a top-left based sprite based on width and height properties
	* which means for a text, you better not use blockAlign ( our model is not very good since 'block align' and 'position align' should be separated )
	*/
	r2::Node* dock(r2::Node* node, IContainer* ref, rd::Dir d, float offset);


	/**
	* Docking will not take internal pivot into account,
	* it will make its best to respect docking propertis for a top-left based sprite based on width and height properties
	* which means for a text, you better not use blockAlign ( our model is not very good since 'block align' and 'position align' should be separated )
	*/
	r2::Node* dock(r2::Node* node, IContainer* ref, rd::Dir d, Vector2 offset);
}

