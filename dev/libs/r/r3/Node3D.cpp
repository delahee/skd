#include "stdafx.h"

#include "Node3D.hpp"

using namespace r3;

Pasta::Matrix44 Node3D::getLocalMatrix() {
	Pasta::Matrix44 mat;
	mat.setScale(scaleX, scaleY, scaleZ);
	if (rotationX) mat.rotateX(PASTA_RAD2DEG(rotationX));
	if (rotationY) mat.rotateY(PASTA_RAD2DEG(rotationY));
	if (rotation) mat.rotateZ(PASTA_RAD2DEG(rotation));
	mat.translate(Pasta::Vector3(x, y, z));
	return mat;
}