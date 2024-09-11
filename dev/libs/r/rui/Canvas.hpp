#pragma once

#include "r2/Node.hpp"
#include "rd/Dir.hpp"
#include "rui/Dock.hpp"
#include "rs/Display.hpp"

namespace rui {

	/************************************************************************/
	// A container that allows docking 
	// always measure refSize
	/************************************************************************/
	class Canvas : public r2::Node, public rui::IContainer {
	public:
		bool									enabled = true;
		rd::Sig									sigOnResize;
		std::map<r2::Node*, rd::SignalHandler*> resizeHolder;
		Vector2i								fallbackRefSize = Vector2i(1920, 1080);

												Canvas(r2::Node* parent);
		virtual									~Canvas();
		virtual void							onResize(const Vector2& ns) override;
		virtual void							resetContent();
		virtual void							dispose() override;

		float									getPixelRatio()const;
		//interface impls
		virtual Vector2							getRefSize();
		virtual float							getScaleX();
		virtual float							getScaleY();
		virtual Vector2							getActualSize();

		virtual void							onRemoveChild(r2::Node* n);

		//helpers for theoritically docking measures, don't use width() as it is content based
		inline float							w() { return getActualSize().x; };
		inline float							h() { return fallbackRefSize.y; };

		r2::Node*								dock(r2::Node* spr, rd::Dir dir, float ofs = 0);
		r2::Node*								dock(r2::Node* spr, rd::Dir dir, Vector2 ofs);

		//don't suscribe to heavy resize for quick patching
		r2::Node*								dockOnce(r2::Node* spr, rd::Dir dir, Vector2 ofs);
		inline r2::Node*						dockUp(r2::Node* spr, float ofs = 0) { dock(spr, rd::Dir(rd::Dir::UP | rd::Dir::LEFT_RIGHT), ofs); return spr; };
		inline r2::Node*						dockLeft(r2::Node* spr, float ofs = 0) { dock(spr, rd::Dir(rd::Dir::LEFT | rd::Dir::UP_DOWN), ofs); return spr; };

		virtual void							runAndRegResize(std::function<void(void)>);

		r::Vector2								getMousePos();
	};
}