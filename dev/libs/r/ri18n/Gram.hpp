#pragma once

#include <vector>

#include "Lex.hpp"


namespace ri18n {
	struct AstNode;

	class Gram {
	public:
		Gram() {

		}

		AstNode * parse(const std::vector<Lexem>&str);

	protected:
		AstNode * parse(const std::vector<Lexem>&str, int pos);

	};
}