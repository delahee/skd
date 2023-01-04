#pragma once

namespace r2{
struct Rect {
	double x = 0.0;
	double y = 0.0;

	double width = 0.0;
	double height = 0.0;

	Rect(double _x, double _y, double _w, double _h) :x(_x),y(_y),width(_w),height(_h){ }
	Rect() { };
	Rect(const Rect & r) {
		x = r.x;
		y = r.y;
		width = r.width;
		height = r.height;
	};

	~Rect();

	static inline Rect fromPosSize( double x, double y, double width, double height) {
		return Rect(x, y, width, height);
	};

	static inline Rect fromTLBR(double xtl, double ytl, double xbr, double ybr) {
		return Rect(xtl, ytl, xbr-xtl, ybr - ytl);
	};

};
}