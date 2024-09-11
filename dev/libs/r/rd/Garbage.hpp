#pragma once

#include "r/Types.hpp"

namespace r { class Object; }
namespace rd { class Agent; }
namespace r2 { 
	class BatchElem; 
	class Node; 
}

namespace rd {
	class Garbage {
	public:
		static void init();
		static void trashObject(r::Object* n);
		static void trash(r2::Node* n);
		static void trash(r2::BatchElem* n);
		static void trash(rd::Agent* n);
		static void gc();
	};
}