#pragma once

namespace rs{
	enum TVar {
		VNone = 0,
		VX,
		VY,
		VZ,

		VScaleX,
		VScaleY,

		VAlpha,//valpha is cumulative for children
		VRotation,

		VR,
		VG,
		VB,
		VA,

		VScale,

		VWidth,
		VHeight,

		VVisibility,

		VScaleZ,

		VCustom0 = 256,
		VCustom1,
		VCustom2,
		VCustom3,

		VCustom4,
		VCustom5,
		VCustom6,
		VCustom7,

		VPointer = 1024,

		VCount
	};

	class ITweenable {
	public:
		virtual			~ITweenable() {};

		virtual double	getValue(TVar valType) = 0;
		virtual double	setValue(TVar valType, double val) = 0;
	};
}

