#include "stdafx.h"
#include "Copy.hpp"

using namespace r;
using namespace r2;
using namespace r2::filter;

Copy::Copy() : Filter() {
	name = std::string("Layer#") + std::to_string(rs::Sys::getUID()); 
	type = FilterType::FT_COPY;
}

r2::filter::Copy::~Copy() {

}
