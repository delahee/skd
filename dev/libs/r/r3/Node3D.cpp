#include "stdafx.h"

#include "Node3D.hpp"

using namespace r3;
using namespace rs;

Pasta::Matrix44 Node3D::getLocalMatrix() {
	Pasta::Matrix44 mat;
	mat.setScale(scaleX, scaleY, scaleZ);
	if (rotationX) mat.rotateX(PASTA_RAD2DEG(rotationX));
	if (rotationY) mat.rotateY(PASTA_RAD2DEG(rotationY));
	if (rotation) mat.rotateZ(PASTA_RAD2DEG(rotation));
	mat.translate(Pasta::Vector3(x, y, z));
	return mat;
}

double Node3D::getValue(TVar valType) {
	switch (valType) {
	case VX:		return x;
	case VY:		return y;
	case VZ:		return z;
	case VScaleX:	return scaleX;
	case VScaleY:	return scaleY;
	case VRotation:	return rotation;
	case VScale:	return (scaleX + scaleY) / 2.0f;

	case VWidth:	return width();
	case VHeight:	return height();
	case VAlpha:	return alpha;
	default:		return 0.0;
	}

}

double Node3D::setValue(TVar valType, double val) {
	switch (valType) {
	case VScaleZ:	scaleZ = val;	trsDirty = true;	break;
	default: {
		return Super::setValue(valType, val);
	}
	};
	return val;
}