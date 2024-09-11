#pragma once

#include "r/Types.hpp"
#include "r2/Bounds.hpp"

namespace r3 {
	struct Bounds {
	public:

		double xMin = 1e20;
		double yMin = 1e20;
		double zMin = 1e20;

		double xMax = -1e20;
		double yMax = -1e20;
		double zMax = -1e20;

		Bounds() {
			empty();
		}

		inline double* ptr() {
			return &xMin;
		}

		inline Bounds& empty() {
			xMin = yMin = zMin = 1e20;
			xMax = yMax = zMax = -1e20;
			return *this;
		}

		inline Bounds& zero() {
			xMin  = yMin = zMin = 0;
			xMax = yMax = zMax = 0;
			return *this;
		}

		inline Bounds& all() {
			xMin = yMin = zMin = - 1e20;
			xMax = yMax = zMax = 1e20;
			return *this;
		}

		inline Vector3 getCenter() const {
			return Vector3(getCenterX(), getCenterY(),getCenterZ());
		}

		inline Vector3 getMin() const {
			return Vector3(xMin, yMin, zMin);
		};

		inline Vector3 getMax() const {
			return Vector3(xMax, yMax, zMax);
		};

		inline double getCenterX() const {
			return 0.5 * (xMax + xMin);
		}

		inline double getCenterY() const {
			return 0.5 * (yMax + yMin);
		}

		inline double getCenterZ() const {
			return 0.5 * (zMax + zMin);
		}

		inline double getWidth() const {
			return std::max(0.0, xMax - xMin);
		}

		inline double getLength() const {
			return std::max(0.0, yMax - yMin);
		}

		inline double getHeight() const {
			return std::max(0.0, zMax - zMin);
		}

		inline double randomX() const {
			return xMin + rs::Sys::randf() * getWidth();
		}

		inline double randomY() const {
			return yMin + rs::Sys::randf() * getLength();
		}
		
		inline double randomZ() const {
			return zMin + rs::Sys::randf() * getHeight();
		}

		inline bool isEmpty() {
			return xMax <= xMin || yMax <= yMin || zMax <= zMin;
		}

		inline Bounds& addPoint(int x, int y, int z) {
			if (x < xMin) xMin = x;
			if (y < yMin) yMin = y;
			if (z < zMin) zMin = z;

			if (x > xMax) xMax = x;
			if (y > yMax) yMax = y;
			if (z > zMax) zMax = z;
			return *this;
		};

		inline Bounds& addPoint(const Vector3i & v) {
			if (v.x < xMin) xMin = v.x;
			if (v.y < yMin) yMin = v.y;
			if (v.z < zMin) zMin = v.z;
			if (v.x > xMax) xMax = v.x;
			if (v.y > yMax) yMax = v.y;
			if (v.z > zMax) zMax = v.z;
			return *this;
		};
		
		inline Bounds& addPoint(float x, float y, float z) {
			if (x < xMin) xMin = x;
			if (y < yMin) yMin = y;
			if (z < zMin) zMin = z;

			if (x > xMax) xMax = x;
			if (y > yMax) yMax = y;
			if (z > zMax) zMax = z;
			return *this;
		};

		inline Bounds& addPoint(double x, double y, double z) {
			if (x < xMin) xMin = x;
			if (y < yMin) yMin = y;
			if (z < zMin) zMin = z;

			if (x > xMax) xMax = x;
			if (y > yMax) yMax = y;
			if (z > zMax) zMax = z;
			return *this;
		};

		inline Bounds& addPoint(const Pasta::Vector3& p) {
			if (p.x < xMin) xMin = p.x;
			if (p.y < yMin) yMin = p.y;
			if (p.z < zMin) zMin = p.z;

			if (p.x > xMax) xMax = p.x;
			if (p.y > yMax) yMax = p.y;
			if (p.z > zMax) zMax = p.z;
			return *this;
		};

		inline Bounds& add(const r3::Bounds& b) {
			if (b.xMin < xMin) xMin = b.xMin;
			if (b.xMax > xMax) xMax = b.xMax;

			if (b.yMin < yMin) yMin = b.yMin;
			if (b.yMax > yMax) yMax = b.yMax;

			if (b.zMin < zMin) zMin = b.zMin;
			if (b.zMax > zMax) zMax = b.zMax;
			return *this;
		};

		inline bool contains(const Pasta::Vector3& p) {
			return p.x >= xMin && p.x < xMax&& p.y >= yMin && p.y < yMax&& p.z >= zMin && p.z < zMax;
		};

		inline bool contains(float px, float py, float pz) {
			return px >= xMin && px < xMax&& py >= yMin && py < yMax && pz >= zMin && pz < zMax;
		};

		inline bool intersects(const Bounds& b) const {
			return !(xMin > b.xMax || xMax < b.xMin || yMax < b.yMin || yMin > b.yMax || zMin >= b.zMax || zMax <= b.zMin);
		};

		inline bool intersectsStrict(const Bounds& b) const {
			return !(xMin >= b.xMax || xMax <= b.xMin || yMin >= b.yMax || yMax <= b.yMin || zMin >= b.zMax || zMax <= b.zMin);
		}; 
		
		inline bool includes(const Vector3 & b) const {
			return !(xMin > b.x || xMax < b.x || yMin > b.y || yMax < b.y || zMin > b.z || zMax < b.z);
		};
		
		inline bool includesStrict(const Vector3 & b) const {
			return !(xMin >= b.x || xMax <= b.x || yMin >= b.y || yMax <= b.y || zMin >= b.z || zMax <= b.z);
		};

		inline r::Vector3 getSize() {
			return r::Vector3(getWidth(), getLength(),getHeight());
		};

		inline Bounds operator+(const Bounds& b) const {
			Bounds r;
			r.xMin = std::min<double>(xMin, b.xMin);
			r.xMax = std::max<double>(xMax, b.xMax);
			r.yMin = std::min<double>(yMin, b.yMin);
			r.yMax = std::max<double>(yMax, b.yMax);
			r.zMin = std::min<double>(zMin, b.zMin);
			r.zMax = std::max<double>(zMax, b.zMax);
			return r;
		};

		inline Bounds operator+(const Vector3& v) const {
			Bounds b;
			b.xMin = xMin + v.x;
			b.xMax = xMax + v.x;
			b.yMin = yMin + v.y;
			b.yMax = yMax + v.y;
			b.zMin = zMin + v.z;
			b.zMax = zMax + v.z;
			return b;
		};

		inline Bounds& operator+=(const Bounds& b) {
			xMin = std::min<double>(xMin, b.xMin);
			xMax = std::max<double>(xMax, b.xMax);
			yMin = std::min<double>(yMin, b.yMin);
			yMax = std::max<double>(yMax, b.yMax);
			zMin = std::min<double>(zMin, b.zMin);
			zMax = std::max<double>(zMax, b.zMax);
			return *this;
		};

		inline Bounds& operator+=(const Vector3& v) {
			xMin += v.x;
			xMax += v.x;
			yMin += v.y;
			yMax += v.y;
			zMin += v.z;
			zMax += v.z;
			return *this;
		};

		inline Bounds operator-(const Vector3& v) const {
			Bounds b;
			b.xMin = xMin - v.x;
			b.xMax = xMax - v.x;
			b.yMin = yMin - v.y;
			b.yMax = yMax - v.y;
			b.zMin = zMin - v.z;
			b.zMax = zMax - v.z;
			return b;
		};

		inline Bounds& operator-=(const Vector3& v) {
			xMin -= v.x;
			xMax -= v.x;
			yMin -= v.y;
			yMax -= v.y;
			zMin -= v.z;
			zMax -= v.z;
			return *this;
		};

		inline Bounds operator*(const double& d) const {
			Bounds b;
			b.xMin = xMin * d;
			b.xMax = xMax * d;
			b.yMin = yMin * d;
			b.yMax = yMax * d;
			b.zMin = zMin * d;
			b.zMax = zMax * d;
			return b;
		};

		inline Bounds operator*(const Vector3& v) const {
			Bounds b;
			b.xMin = xMin * v.x;
			b.xMax = xMax * v.x;
			b.yMin = yMin * v.y;
			b.yMax = yMax * v.y;
			b.zMin = zMin * v.z;
			b.zMax = zMax * v.z;
			return b;
		};

		inline Bounds& operator*=(const double& d) {
			xMin *= d;
			xMax *= d;
			yMin *= d;
			yMax *= d;
			zMin *= d;
			zMax *= d;
			return *this;
		};

		inline Bounds& operator*=(const Vector3& v) {
			xMin *= v.x;
			xMax *= v.x;
			yMin *= v.y;
			yMax *= v.y;
			zMin *= v.z;
			zMax *= v.z;
			return *this;
		};

		void		im();
		r2::Bounds	toBounds2() const;
		std::string toString() const;
	};

}
