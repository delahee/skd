#include "stdafx.h"
#include "Layer.hpp"


using namespace r2;
using namespace r2::filter;
using namespace r;

Layer::Layer() : Filter() { 
	name = std::string("Layer#") + std::to_string(rs::Sys::getUID()); 
	type = FilterType::FT_LAYER;
}

r2::filter::Layer::~Layer() {

}
