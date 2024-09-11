#pragma once

#include "rd/RandBeanBag.hpp"

namespace ri18n {
	class Nar;

	struct RandTextEntry {
		int							idx = 0;
		Str							key;
		Str							content;
		std::vector<std::string>	chunks;
		std::vector<AstNode*>		asts;
		RandBeanBag<int>			bag;

		std::string					randChunk(rd::Rand* random=0);
		void						im();
	};

	class RandText {
	public:
		std::vector<RandTextEntry*>				index;
		std::unordered_map<Str, RandTextEntry*>	lines;

									RandText();
		std::string					gen(const char* key, rd::Rand* random = 0);
		void						im();
		static RandText*			parse(const char* src);
	};
}