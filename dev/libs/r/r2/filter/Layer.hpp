#pragma once

#include "../Filter.hpp"

/***
* this is means for static copies refreshing once in a while 
* it is freezed by default
* use invalidate to trigger dynamism
* will invalidate if you change source
*/
namespace r2 {
	namespace filter {
		class Layer : public Filter {
		public:
									Layer();
			virtual					~Layer();
			virtual bool			shouldForceAlpha() override { return true; };
		};
	}
}