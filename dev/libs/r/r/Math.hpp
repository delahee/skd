#pragma once

#include "r/Types.hpp"
	
namespace r {
	class Math {
	public:
		static constexpr float pi = 3.14159265359f;
		static constexpr float goldenRatio = 1.618033988749f;
		static constexpr float invGoldenRatio = 1.0f / 1.618033988749f;

		static inline double normAngle(double x) {
			x = fmod(x, 2 * pi);
			if (x < 0)
				x += 2 * pi;
			return x;
		}

		static inline float rad2Deg(float x) {
			return x * (180.f / PASTA_PI);
		};

		static inline float deg2Rad(float x) {
			return x * (PASTA_PI / 180.f);
		};

		static inline float normAngle(float x) {
			x = fmodf(x, 2.0f * pi);
			return x;
		}

		static inline float normAnglePos(float x) {
			x = fmodf(x, 2.0f * pi);
			if (x < 0)
				x += 2.0f * pi;
			return x;
		}

		static inline bool isZero(float a, float epsilon = 1e-6f){
			return fabsf(a) <= epsilon;
		};

		
		static inline bool approximatelyEqual(float a, float b, float epsilon = 1e-6f)
		{
			return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
		};

		static inline bool approximatelyEqual(const Vector3& a, const Vector3& b, float eps = 1e-6f){
			return approximatelyEqual(a.x, b.x, eps)
				&& approximatelyEqual(a.y, b.y, eps)
				&& approximatelyEqual(a.z, b.z, eps);
		};

		static inline bool essentiallyEqual(float a, float b, float epsilon = 1e-6f)
		{
			return fabs(a - b) <= ((fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * epsilon);
		};

		static inline bool definitelyGreaterThan(float a, float b, float epsilon = 1e-6)
		{
			return (a - b) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
		};

		static inline bool definitelyLessThan(float a, float b, float epsilon = 1e-6)
		{
			return (b - a) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
		};

		static inline float clamp(float v, float mn = 0.0f, float mx = 1.0f) {
			if (v < mn) return mn;
			else if (v > mx) return mx;
			else return v;
		};

		static inline float lerp(float a, float b, float ratio) {
			return a + ratio * (b - a);
		};
		
		static inline double lerp(double a, double b, double ratio) {
			return a + ratio * (b - a);
		};

		static inline void swap(float& a, float& b) {
			float tmp = a;
			a = b;
			b = tmp;
		};

		template<typename T>
		static inline T moveTowards(T current, T target, float maxDistanceDelta) {
			T toVector = target - current;
			float sqdist = current.getNormSquared();
			if ((sqdist == 0) || ( (maxDistanceDelta >= 0) && (sqdist <= maxDistanceDelta * maxDistanceDelta)))
				return target;
			auto dist = sqrtf(sqdist);
			return current + toVector / dist * maxDistanceDelta;
		};

		static inline int umod(int val, int rad) {
			if (rad == 0)
				return 0;
			int res = val % rad;
			while (res < 0)
				res += rad;
			return res;
		};

		static bool isFloat(float f, float thresh = 1e-6f);
	};

	struct PASTA_EXPORT Vector2s {
		r::s16 x = 0;
		r::s16 y = 0;

		Vector2s() {};
		Vector2s(r::s16 x, r::s16 y) { this->x = x; this->y = y; }
		Vector2s(const Vector2s& o) { x = o.x; y = o.y; }

		explicit Vector2s(int _xy) : x(_xy), y(_xy) {}
		explicit Vector2s(const Vector2i&_xy) : x(_xy.x), y(_xy.y) {}

		inline Vector2s abs() const {
			return Vector2s(x > 0 ? x : -x, y > 0 ? y : -y);
		};

		inline Vector2s operator-() const {
			return Vector2s(-x, -y);
		};

		inline Vector2s operator-(const Vector2s& v) const {
			return Vector2s(x - v.x, y - v.y);
		}

		inline Vector2s operator-(int k) const {
			return Vector2s(x - k, y - k);
		}

		inline Vector2s& operator-=(const Vector2s& v) {
			x -= v.x;
			y -= v.y;
			return *this;
		}

		inline Vector2s operator+(const Vector2s& v) const {
			return Vector2s(x + v.x, y + v.y);
		}

		inline Vector2s operator+(int k) const {
			return Vector2s(x + k, y + k);
		}

		inline Vector2s& operator+=(const Vector2s& v) {
			x += v.x;
			y += v.y;
			return *this;
		}

		inline Vector2s operator*(int k) const {
			return Vector2s(x * k, y * k);
		}

		inline Vector2s operator*(const Vector2s& _rhs) const {
			return Vector2s(x * _rhs.x, y * _rhs.y);
		}

		inline Vector2s& operator*=(int k) {
			x *= k;
			y *= k;
			return *this;
		}

		inline Vector2s operator/(int k) const {
			return Vector2s(x / k, y / k);
		}

		inline Vector2s operator/(const Vector2s& _rhs) const {
			return Vector2s(x / _rhs.x, y / _rhs.y);
		}

		inline Vector2s& operator/=(int k) {
			x /= k;
			y /= k;
			return *this;
		}
		
		inline void translate( s16 _x, s16 _y){
			x += _x;
			y += _y;
		}

		inline bool operator==(const Vector2s& v) const { return (x == v.x) && (y == v.y); }
		inline bool operator!=(const Vector2s& v) const { return !(*this == v); }

		inline r::s16* ptr() { return &x; };
		inline const r::s16* ptr() const { return &x; };

		float		getNorm() const { return sqrt(getNormSquared()); }
		double		getNormSquared() const { return x * x + y * y; }
	};

	struct PASTA_EXPORT Vector3s {
		r::s16 x = 0;
		r::s16 y = 0;
		r::s16 z = 0;
		r::s16 _pad;

		Vector3s() {};
		Vector3s(r::s16 x, r::s16 y, r::s16 z) {
			this->x = x; this->y = y; this->z = z;
		};

		Vector3s(const Vector3s& o) {
			x = o.x; y = o.y; z = o.z;
		};

		explicit Vector3s(int _xy) : x(_xy), y(_xy), z(_xy) {}
		explicit Vector3s(const Vector3i& _xyz) : x(_xyz.x), y(_xyz.y), z(_xyz.z) {}

		inline Vector3s abs() const {
			return Vector3s(x > 0 ? x : -x, y > 0 ? y : -y, z > 0 ? z : -z);
		};

		inline Vector3s operator-() const {
			return Vector3s(-x, -y,-z);
		};

		inline Vector3s operator-(const Vector3s& v) const {
			return Vector3s(x - v.x, y - v.y, z - v.z);
		}

		inline Vector2s operator-(int k) const {
			return Vector2s(x - k, y - k);
		}

		inline Vector3s& operator-=(const Vector3s& v) {
			x -= v.x;
			y -= v.y;
			return *this;
		}

		inline Vector3s operator+(const Vector3s& v) const {
			return Vector3s(x + v.x, y + v.y, z + v.z);
		}

		inline Vector3s operator+(int k) const {
			return Vector3s(x + k, y + k, z + k);
		}

		inline Vector3s& operator+=(const Vector3s& v) {
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}

		inline Vector3s operator*(int k) const {
			return Vector3s(x * k, y * k, z * k);
		}

		inline Vector3s operator*(const Vector3s& _rhs) const {
			return Vector3s(x * _rhs.x, y * _rhs.y, z * _rhs.z);
		}

		inline Vector3s& operator*=(int k) {
			x *= k;
			y *= k;
			z *= k;
			return *this;
		}

		inline Vector3s operator/(int k) const {
			float d = 1.0f / k;
			return Vector3s(x*k,y*k,z*k);
		}

		inline Vector3s operator/(const Vector3s& _rhs) const {
			return Vector3s(x / _rhs.x, y / _rhs.y, z / _rhs.z);
		}

		inline Vector3s& operator/=(int k) {
			float d = 1.0f / k;
			x *= d;
			y *= d;
			z *= d;
			return *this;
		}

		inline void translate(s16 _x, s16 _y, s16 _z) {
			x += _x;
			y += _y;
			z += _z;
		};

		inline bool operator==(const Vector3s& v) const { return (x == v.x) && (y == v.y) && (z == v.z); }
		inline bool operator!=(const Vector3s& v) const { return !(*this == v); }

		inline r::s16* ptr() { return &x; };
		inline const r::s16* ptr() const { return &x; };

		float		getNorm() const { return sqrt(getNormSquared()); }
		double		getNormSquared() const { return x * x + y * y + z * z; }
	};

}


