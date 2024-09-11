#pragma once

namespace r {
	struct Vector2d {
	public:
		double x = 0.0;
		double y = 0.0;

		Vector2d() {

		};

		Vector2d(double _x, double _y)
			:x(_x), y(_y)
		{

		};

		Vector2d operator-() const
		{
			return Vector2d(-x, -y);
		}

		Vector2d operator-(const Vector2d& v) const
		{
			return Vector2d(x - v.x, y - v.y);
		}

		Vector2d operator-(int k) const{
			return Vector2d(x - k, y - k);
		}

		Vector2d& operator-=(const Vector2d& v){
			x -= v.x;
			y -= v.y;
			return *this;
		}

		Vector2d operator+(const Vector2d& v) const{
			return Vector2d(x + v.x, y + v.y);
		}

		Vector2d operator+(int k) const{
			return Vector2d(x + k, y + k);
		}

		Vector2d& operator+=(const Vector2d& v)
		{
			x += v.x;
			y += v.y;
			return *this;
		}

		Vector2d operator*(int k) const{
			return Vector2d(x * k, y * k);
		}

		Vector2d operator*(const Vector2i& _rhs) const{
			return Vector2d(x * _rhs.x, y * _rhs.y);
		}

		Vector2d& operator*=(int k){
			x *= k;
			y *= k;
			return *this;
		}

		Vector2d operator/(int k) const{
			return Vector2d(x / k, y / k);
		}

		auto operator/(const Vector2d& _rhs) const {
			return Vector2d(x / _rhs.x, y / _rhs.y);
		}

		auto& operator/=(int k)
		{
			x /= k;
			y /= k;
			return *this;
		}

		bool operator==(const Vector2d& v) const { return (x == v.x) && (y == v.y); }
		bool operator!=(const Vector2d& v) const { return !(*this == v); }
	};

	typedef r::Vector2d					vec2d;
}
 
