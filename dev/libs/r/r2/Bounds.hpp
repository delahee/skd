#pragma once

#include "r/Types.hpp"

namespace r2 {
	struct Bounds {
	public:

		double xMin = 1e20;
		double yMin = 1e20;

		double xMax = -1e20;
		double yMax = -1e20;

		Bounds() {
			empty();
		}

		inline double* ptr() {
			return &xMin;
		};

		void im();

		Bounds& empty() {
			xMin = 1e20;
			yMin = 1e20;
			xMax = -1e20;
			yMax = -1e20;
			return *this;
		}


		Bounds& zero() {
			xMin = 0;
			yMin = 0;
			xMax = 0;
			yMax = 0;
			return *this;
		}

		Bounds& all() {
			xMin = -1e20;
			yMin = -1e20;
			xMax = 1e20;
			yMax = 1e20;
			return *this;
		}

		Vector2 getCenter() const {
			return Vector2(getCenterX(), getCenterY());
		}

		double getCenterX() const {
			return 0.5 * (xMax + xMin);
		}

		double getCenterY() const {
			return 0.5 * (yMax + yMin);
		}

		inline double getWidth() const {
			return std::max(0.0, xMax - xMin);
		}

		inline double getHeight() const {
			return std::max(0.0, yMax - yMin);
		}

		inline double randomX()const {
			return xMin + rs::Sys::randf() * getWidth();
		}

		inline double randomY() const {
			return yMin + rs::Sys::randf() * getHeight();
		}

		inline bool isEmpty() const {
			return xMax <= xMin || yMax <= yMin;
		}

		inline Bounds& offset(double dx, double dy) {
			xMin += dx;
			xMax += dx;
			yMin += dy;
			yMax += dy;
			return *this;
		}

		inline Bounds& scale(double dx, double dy) {
			*this = fromCenterSize(getCenterX(), getCenterY(), getWidth() * dx, getHeight() * dy);
			return *this;
		}

		double left() const;
		double right() const;

		double top() const;
		double down() const;

		double up() const;
		double bottom() const;

		Bounds& load4(double x, double y, double width, double height) {
			xMin = x;
			yMin = y;
			xMax = x + width;
			yMax = y + height;
			return *this;
		};

		Bounds& addPoint(int x, int y) {
			if (x < xMin) xMin = x;
			if (x > xMax) xMax = x;
			if (y < yMin) yMin = y;
			if (y > yMax) yMax = y;
			return *this;
		};

		Bounds& addPoint(double x, double y) {
			if (x < xMin) xMin = x;
			if (x > xMax) xMax = x;
			if (y < yMin) yMin = y;
			if (y > yMax) yMax = y;
			return *this;
		};

		Bounds& addPoint(float x, float y) {
			if (x < xMin) xMin = x;
			if (x > xMax) xMax = x;
			if (y < yMin) yMin = y;
			if (y > yMax) yMax = y;
			return *this;
		};

		Bounds& addPoint(const Pasta::Vector3& p) {
			if (p.x < xMin) xMin = p.x;
			if (p.x > xMax) xMax = p.x;
			if (p.y < yMin) yMin = p.y;
			if (p.y > yMax) yMax = p.y;
			return *this;
		};

		Bounds& add(const Bounds& b) {
			if (b.xMin < xMin) xMin = b.xMin;
			if (b.xMax > xMax) xMax = b.xMax;
			if (b.yMin < yMin) yMin = b.yMin;
			if (b.yMax > yMax) yMax = b.yMax;
			return *this;
		};

		Bounds getTransformed(const Pasta::Matrix44& mat) {
			Bounds b = *this;
			b.transform(mat);
			return b;
		};

		void translate(const vec2& v) {
			xMin += v.x;
			xMax += v.x;
			yMin += v.y;
			yMax += v.y;
		};

		Bounds getTranslated(const vec2& v) {
			Bounds b = *this;
			b.translate(v);
			return b;
		};

		void transform(const Pasta::Matrix44& mat) {
			if (isEmpty())//ok let's pray for someone's soul here
				return;

			double l = left();
			double t = top();
			double b = bottom();
			double r = right();

			empty();

			Pasta::Vector3 topLeft = mat * Pasta::Vector3(l, t, 0.f);
			addPoint(topLeft);
			Pasta::Vector3 topRight = mat * Pasta::Vector3(r, t, 0.f);
			addPoint(topRight);
			Pasta::Vector3 bottomLeft = mat * Pasta::Vector3(l, b, 0.f);
			addPoint(bottomLeft);
			Pasta::Vector3 bottomRight = mat * Pasta::Vector3(r, b, 0.f);
			addPoint(bottomRight);
		}

		inline bool contains(const Pasta::Vector2& p) {
			return p.x >= xMin && p.x < xMax&& p.y >= yMin && p.y < yMax;
		}

		inline bool contains(float px, float py) {
			return px >= xMin && px < xMax&& py >= yMin && py < yMax;
		}

		inline bool intersects(const Bounds& b) const {
			return !(xMin > b.xMax || yMin > b.yMax || xMax < b.xMin || yMax < b.yMin);
		}

		inline bool intersectsStrict(const Bounds& b) const {
			return !(xMin >= b.xMax || yMin >= b.yMax || xMax <= b.xMin || yMax <= b.yMin);
		}

		bool testCircle(double px, double py, double r) {
			double closestX = std::clamp(px, xMin, xMax);
			double closestY = std::clamp(py, yMin, yMax);

			double distX = px - closestX;
			double distY = py - closestY;

			double distSq = distX * distX + distY * distY;
			return distSq < r* r;
		}

		static Bounds fromCenterSize(double x, double y, double w, double h) {
			Bounds b;
			b.xMin = x - w * 0.5;
			b.yMin = y - w * 0.5;

			b.xMax = x + w * 0.5;
			b.yMax = y + h * 0.5;
			return b;
		}

		static Bounds fromTLWH(const r::Vector2& tl, const r::Vector2& wh) {
			return fromTLWH(tl.x, tl.y, wh.x, wh.y);
		};

		static Bounds fromTLWH(double x, double y, double w, double h) {
			Bounds b;
			b.xMin = x;
			b.yMin = y;

			b.xMax = x + w;
			b.yMax = y + h;
			return b;
		}

		static Bounds fromTLBR(double t, double l, double b, double r) {
			Bounds nu;
			nu.xMin = l;
			nu.yMin = t;
			nu.xMax = r;
			nu.yMax = b;
			return nu;
		}

		std::string toString();

		double width() const {
			return abs(right() - left());
		}

		double height() const {
			return abs(bottom() - top());
		}

		inline r::Vector2 getTopLeft() { return r::Vector2(left(), top()); };
		inline r::Vector2 getTopRight() { return r::Vector2(right(), top()); };
		inline r::Vector2 getBottomLeft() { return r::Vector2(left(), bottom()); };
		inline r::Vector2 getBottomRight() { return r::Vector2(right(), bottom()); };

		inline r::Vector2 getSize() {
			return r::Vector2(width(), height());
		};

	};

	struct BoundsI {
	public:

		int xMin = 1 << 28;
		int yMin = 1 << 28;

		int xMax = -(1 << 28);
		int yMax = -(1 << 28);

		BoundsI() {
			empty();
		}

		inline int* ptr() {
			return &xMin;
		}

		void im();

		BoundsI& empty() {
			xMin = 1 << 28;
			yMin = 1 << 28;
			xMax = -(1 << 28);
			yMax = -(1 << 28);
			return *this;
		}

		BoundsI& zero() {
			xMin = 0;
			yMin = 0;
			xMax = 0;
			yMax = 0;
			return *this;
		}

		BoundsI& all() {
			xMin = -(1 << 28);
			yMin = -(1 << 28);
			xMax = 1 << 28;
			yMax = 1 << 28;
			return *this;
		}

		inline BoundsI inflate(int dx, int dy) const {
			BoundsI self = *this;
			self.xMin -= dx >> 1;
			self.yMin -= dy >> 1;
			self.xMax += dx >> 1;
			self.yMax += dy >> 1;
			return self;
		};

		Vector2i getCenter() {
			return Vector2i(getCenterX(), getCenterY());
		}

		double getCenterX() {
			return 0.5 * (xMax + xMin);
		}

		double getCenterY() {
			return 0.5 * (yMax + yMin);
		}

		inline int getWidth() {
			return std::max(0, xMax - xMin);
		}

		inline int getHeight() {
			return std::max(0, yMax - yMin);
		}

		inline int randomX() {
			return xMin + rs::Sys::random(getWidth());
		}

		inline int randomY() {
			return yMin + rs::Sys::random(getHeight());
		}

		inline bool isEmpty() {
			return xMax <= xMin || yMax <= yMin;
		}

		inline BoundsI& offset(int dx, int dy) {
			xMin += dx;
			xMax += dx;
			yMin += dy;
			yMax += dy;
			return *this;
		}

		int left() const;
		int right() const;

		int top() const;
		int down() const;

		int up() const;
		int bottom() const;

		BoundsI& load4(int x, int y, int width, int height) {
			xMin = x;
			yMin = y;
			xMax = x + width;
			yMax = y + height;
			return *this;
		};

		BoundsI& addPoint(int x, int y) {
			if (x < xMin) xMin = x;
			if (x > xMax) xMax = x;
			if (y < yMin) yMin = y;
			if (y > yMax) yMax = y;
			return *this;
		};

		BoundsI& addPoint(const Pasta::Vector3i& p) {
			if (p.x < xMin) xMin = p.x;
			if (p.x > xMax) xMax = p.x;
			if (p.y < yMin) yMin = p.y;
			if (p.y > yMax) yMax = p.y;
			return *this;
		};

		BoundsI& add(const BoundsI& b) {
			if (b.xMin < xMin) xMin = b.xMin;
			if (b.xMax > xMax) xMax = b.xMax;
			if (b.yMin < yMin) yMin = b.yMin;
			if (b.yMax > yMax) yMax = b.yMax;
			return *this;
		};

		BoundsI getTransformed(const Pasta::Matrix44& mat) {
			BoundsI b = *this;
			b.transform(mat);
			return b;
		};

		void transform(const Pasta::Matrix44& mat) {
			if (isEmpty())//ok let's pray for someone's soul here
				return;

			int l = left();
			int t = top();
			int b = bottom();
			int r = right();

			empty();

			Pasta::Vector3 topLeft = mat * Pasta::Vector3(l, t, 0.f);
			addPoint(Vector3i(std::lrint(topLeft.x), std::lrint(topLeft.y), std::lrint(topLeft.z)));

			Pasta::Vector3 topRight = mat * Pasta::Vector3(r, t, 0.f);
			addPoint(Vector3i(std::lrint(topRight.x), std::lrint(topRight.y), std::lrint(topRight.z)));

			Pasta::Vector3 bottomLeft = mat * Pasta::Vector3(l, b, 0.f);
			addPoint(Vector3i(std::lrint(bottomLeft.x), std::lrint(bottomLeft.y), std::lrint(bottomLeft.z)));

			Pasta::Vector3 bottomRight = mat * Pasta::Vector3(r, b, 0.f);
			addPoint(Vector3i(std::lrint(bottomRight.x), std::lrint(bottomRight.y), std::lrint(bottomRight.z)));
		}

		inline bool contains(const Pasta::Vector2& p) {
			return p.x >= xMin && p.x < xMax&& p.y >= yMin && p.y < yMax;
		}
		
		inline bool contains(const Pasta::Vector2i& p) {
			return p.x >= xMin && p.x < xMax&& p.y >= yMin && p.y < yMax;
		}

		inline bool contains(float px, float py) {
			return px >= xMin && px < xMax&& py >= yMin && py < yMax;
		}
		
		inline bool contains(int px, int py) {
			return px >= xMin && px < xMax&& py >= yMin && py < yMax;
		}

		inline bool intersects(const BoundsI& b) const {
			return !(xMin > b.xMax || yMin > b.yMax || xMax < b.xMin || yMax < b.yMin);
		}

		inline bool intersectsStrict(const BoundsI& b) const {
			return !(xMin >= b.xMax || yMin >= b.yMax || xMax <= b.xMin || yMax <= b.yMin);
		}

		bool testCircle(int px, int py, int r);

		static BoundsI fromCenterSize(int x, int y, int w, int h);

		static BoundsI fromTLWH(int x, int y, int w, int h) {
			BoundsI b;
			b.xMin = x;
			b.yMin = y;
			b.xMax = x + w;
			b.yMax = y + h;
			return b;
		}

		static BoundsI fromTLBR(int t, int l, int b, int r) {
			BoundsI nu;
			nu.xMin = l;
			nu.yMin = t;
			nu.xMax = r;
			nu.yMax = b;
			return nu;
		}

		std::string toString();

		int width() const {
			return abs(right() - left());
		}

		int height() const {
			return abs(bottom() - top());
		}

		inline r::Vector2i getTopLeft() { return r::Vector2i(left(), top()); };
		inline r::Vector2i getTopRight() { return r::Vector2i(right(), top()); };
		inline r::Vector2i getBottomLeft() { return r::Vector2i(left(), bottom()); };
		inline r::Vector2i getBottomRight() { return r::Vector2i(right(), bottom()); };

		inline r::Vector2i getSize() {
			return r::Vector2i(width(), height());
		};
	};

}