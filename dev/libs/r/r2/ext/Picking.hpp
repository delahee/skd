#pragma once

#include "r/Types.hpp"
#include "r2/Node.hpp"

namespace r2 {
	namespace ext {
		class Picking {
		public:

			//todo add map reduce fold style funcs
			static void pickNode(r2::Node * ref, r2::Node * n, const r::Vector2 & ev, r::u64 & res);
			static void pickNodes(r2::Node* ref, r2::Node* n, const r::Vector2& ev, std::vector<r2::Node*> &res);
			static void pickNodeByType(r2::Node* ref, r2::Node* n, const r::Vector2& ev, r::u64& res, NodeType nt);

			static void pickNodeOrElem(r2::Node *ref, r2::Node * n, const r::Vector2& ev, r::u64 & res);

			//rejection mask filters out node or be flags
			static void pickNodeOrElem(r2::Node* ref, r2::Node* n, const r::Vector2& ev, r::u64& res, u32 rejectionMask);
			static void traverse(r2::Node* cur, std::function<void(r2::Node*, r2::BatchElem*)> f);
		};
	}
}

