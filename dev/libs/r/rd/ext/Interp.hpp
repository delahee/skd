#pragma once
#include "r/Types.hpp"

namespace rd {
	namespace ext {

		
		class Interp {

		public:

			static r::Vector2 lerp(r::Vector2 v0, r::Vector2 v1, float t) {
				float tx = (v1.x - v0.x) * t + v0.x;
				float ty = (v1.y - v0.y) * t + v0.y;
				return r::Vector2(tx, ty);
			}

			static float catmull(
				float p0,
				float p1,
				float p2,
				float p3, float t) {

				float q = 2.0 * p1;
				float t2 = t * t;

				q += (-p0 + p2) * t;
				q += (2.0 * p0 - 5.0 * p1 + 4 * p2 - p3) * t2;
				q += (-p0 + 3 * p1 - 3 * p2 + p3) * t2 * t;

				return 0.5 * q;
			};

			static Vector2 c2(
				Vector2 p0,
				Vector2 p1,
				Vector2 p2,
				Vector2 p3, float t)
			{
				return Vector2(
					catmull(p0.x, p1.x, p2.x, p3.x, t),
					catmull(p0.y, p1.y, p2.y, p3.y, t)
				);
			};

			static r::Vector3 c3(
				r::Vector3 p0,
				r::Vector3 p1,
				r::Vector3 p2,
				r::Vector3 p3, float t)
			{
				return r::Vector3(
					catmull(p0.x, p1.x, p2.x, p3.x, t),
					catmull(p0.y, p1.y, p2.y, p3.y, t),
					catmull(p0.z, p1.z, p2.z, p3.z, t)
				);
			};

			static r::Vector4 c4(
				r::Vector4 p0,
				r::Vector4 p1,
				r::Vector4 p2,
				r::Vector4 p3, float t)
			{
				return r::Vector4(
					catmull(p0.x, p1.x, p2.x, p3.x, t),
					catmull(p0.y, p1.y, p2.y, p3.y, t),
					catmull(p0.z, p1.z, p2.z, p3.z, t),
					catmull(p0.w, p1.w, p2.w, p3.w, t)
				);
			};
		};

		class CurveC1 {
		public:
			eastl::vector<float> data;

					CurveC1() {};
			float	plot(float t);

			void	load(const char* folder, const char* file);
			void	save(const char* folder, const char* file);

			virtual bool im();
		protected:
			float	get(int idx);
		};

		class CurveC2 {
		public:
			eastl::vector<Vector2> data;

					CurveC2() {};
			Vector2	plot(float t);

			int		size()const { return data.size(); };
			void	load(const char* folder, const char* file);
			void	save(const char* folder, const char* file);
			bool	im();
		protected:
			const Vector2& get(int idx);
		};

		class CurveC3 {
		public:
			eastl::vector<Vector3> data;

					CurveC3() {};
			Vector3	plot( float t );

			void	load(const char* folder, const char* file);
			void	save(const char* folder, const char* file);
			bool	im();
		protected:
			const Vector3& get(int idx);
		};
	}
}