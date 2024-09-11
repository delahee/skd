#pragma once

#include "r/Types.hpp"

namespace rd {
	enum Dir {
		NONE = 0,

		UP = 1,
		DOWN = 2,
		LEFT = 4,
		RIGHT = 8,

		UP_LEFT = (1 | 4),
		UP_RIGHT = (1 | 8),

		DOWN_LEFT = (2 | 4),
		DOWN_RIGHT = (2 | 8),

		DIAG_TL = (UP | LEFT),
		DIAG_TR = (UP | RIGHT),
		DIAG_BR = (DOWN | RIGHT),
		DIAG_BL = (DOWN | LEFT),

		UP_DOWN = (1 | 2),//often aka vert center
		LEFT_RIGHT = (4 | 8),//often aka horiz center

		UP_DOWN_LEFT = (1 | 2 | 4),
		UP_DOWN_RIGHT = (1 | 2 | 8),
		UP_LEFT_RIGHT = (1 | 4 | 8),
		DOWN_LEFT_RIGHT = (2 | 4 | 8),
		TLDR = (1 | 2 | 4 | 8),
	};

	class DirLib {
	public:
		static const char*				dirToString(rd::Dir dir);
		static bool						isOppositeDir(rd::Dir a, rd::Dir b);
		static std::vector<Dir>			straightDirs;
		static std::vector<Dir>			allDirs;

		static r::vec2					followDir(const r::vec2& v, rd::Dir dir);
		static r::vec2i					followDir(const r::vec2i& v, rd::Dir dir);
		static r::vec3i					followDir(const r::vec3i& v, rd::Dir dir);
	};
}

namespace std {
	inline std::string to_string(rd::Dir d) { return rd::DirLib::dirToString(d); };
}