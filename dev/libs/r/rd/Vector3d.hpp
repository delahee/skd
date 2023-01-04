#pragma once

namespace rd {
	class Vector3d {
	public:
		double x = 0.0;
		double y = 0.0;
		double z = 0.0;

		Vector3d() {

		};

		Vector3d(double _x, double _y, double _z) 
			:x(_x), y(_y), z(_z)
		{

		};
	};
}
 
