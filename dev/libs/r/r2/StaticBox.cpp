#include "stdafx.h"
#include "StaticBox.hpp"

using namespace r2;

#define PARENT r2::Node
StaticBox::StaticBox(const r2::Bounds & b, r2::Node* p) : PARENT(p) {
	bnd = b;
	setName("StaticBox");
}

StaticBox::~StaticBox() {

}

r2::Bounds r2::StaticBox::getMyLocalBounds(){
	Bounds calc = bnd;
    calc.transform(getLocalMatrix());
	return calc;
}

r2::Bounds StaticBox::getMeasures(bool forFilter){
	if (forFilter) 
		return PARENT::getMeasures(forFilter);
	
	syncMatrix();
	return getMyLocalBounds();
}

double r2::StaticBox::width()
{
	return bnd.width();
}

double r2::StaticBox::height()
{
	return bnd.height();
}

#undef PARENT