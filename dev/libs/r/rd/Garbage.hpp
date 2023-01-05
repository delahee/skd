#pragma once

namespace r2 { 
	class BatchElem; 
	class Node; 
}

namespace rd {
	class Garbage {
	public:
		static void init();
		static void trash(r2::Node* n);
		static void trash(r2::BatchElem* n);
	};
}