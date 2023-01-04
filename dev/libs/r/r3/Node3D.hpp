#pragma once

#include "r2/Node.hpp"

namespace r3 {
	class Node3D : public r2::Node {
	public:
		double scaleZ = 1.0;
		double rotationX = 0.0;
		double rotationY = 0.0;

		Node3D(r2::Node* parent) : r2::Node(parent) {}

		virtual Pasta::Matrix44 getLocalMatrix();

		virtual void im();
	};
}