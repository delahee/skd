#pragma once

namespace r {
	struct Vector4i {
	public:
		int x = 0;
		int y = 0;
		int z = 0;
		int w = 0;

		explicit Vector4i(int _x = 0,int _y = 0,int _z = 0,int _w=0) {
			x = _x;
			y = _y;
			z = _z;
			w = _w;
		};

		inline int*			ptr() { return &x; };
		inline const int*	ptr() const { return &x; };
		inline bool			isZero() const { return 0 == x == y == z == w ; };
	};
}
 
