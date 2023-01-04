#pragma once

#include <functional>

namespace Pasta {
	struct JReflect;
}

namespace r2 {
	class Serialize {
	public:
		static std::function<r2::Node* (NodeType nt)> customTypeResolver;
	};
}