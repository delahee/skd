#include "stdafx.h"
#include "Layer.hpp"
#include "ColorMatrix.hpp"
#include "../../rd/JSerialize.hpp"

using namespace std;
using namespace r2;
using namespace r2::filter;

static string colMatrixPrmName = string("uColorMatrix");

#define SUPER r2::filter::Layer
r2::filter::ColorMatrix::ColorMatrix() : Layer(){
	type = FilterType::FT_COLORMATRIX;

	ctrl.mode = ColorMatrixMode::CMM_Matrix;
	mat = &ctrl.mat;
	ctrl.mat = Pasta::Matrix44::identity;
	pushBitmapOp([=](r2::Bitmap&bmp) { bmpOp(bmp); });
}

r2::filter::ColorMatrix::ColorMatrix(const Pasta::Matrix44 & _mat) : Layer(){
	type = FilterType::FT_COLORMATRIX;

	setMatrix(_mat);
	pushBitmapOp([=](r2::Bitmap&bmp) { bmpOp(bmp); });
}

void r2::filter::ColorMatrix::bmpOp(r2::Bitmap&bmp) {
	if (!mat) return;

	bmp.mkUber();
	bmp.shaderFlags |= USF_ColorMatrix;
	bmp.updateShaderParam(colMatrixPrmName, this->mat->ptr(),16);
}

void r2::filter::ColorMatrix::setMatrix(const Pasta::Matrix44 & _mat){
	ctrl.mat = _mat;
	mat = &ctrl.mat;
}

void r2::filter::ColorMatrix::setMatrix(const Pasta::Matrix44 * _mat){
	mat = _mat;
}

void r2::filter::ColorMatrix::setHSV(float hue,float sat,float val){
	rd::ColorLib::colorHSV(ctrl.mat, hue, sat, val);
	ctrl.mat = ctrl.mat.transpose();
	invalidate();
}


r2::Filter* r2::filter::ColorMatrix::clone(r2::Filter* _obj) {
	ColorMatrix* obj = (ColorMatrix*)_obj;
	if (!obj) obj = new ColorMatrix();
	SUPER::clone(obj);
	obj->ctrl = ctrl;
	obj->mat = mat;
	return obj;
}

void r2::filter::ColorMatrix::serialize(Pasta::JReflect& jr, const char* name) {
	if (name) jr.visitObjectBegin(name);
	SUPER::serialize(jr, 0);
	ctrl.serialize(&jr,"ctrl");
	if (name) jr.visitObjectEnd(name);
}