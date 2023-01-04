#pragma once
#include "Bounds.hpp"
#include "Node.hpp"

//TODO add this to hierach explo creation capabilitie
namespace r2 {
	class StaticBox : public r2::Node {
	public:
		r2::Bounds			bnd;

		StaticBox(const r2::Bounds& b, r2::Node* p);
		virtual ~StaticBox();
		/**
			Only process Node as a leaf
			Get the bounds relative to the node coordinate system
			If you want to force bounds size, do it here
			*/
		virtual r2::Bounds	getMyLocalBounds();

		//does only take content into account, no leaf filtering stuff, used by filtering callee 
		//ads children
		virtual r2::Bounds	getMeasures(bool forFilter) override;

		virtual double width();
		virtual double height();
	};

}