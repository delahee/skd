#pragma once

namespace r3 {
	namespace im {
		r2::Graphics*						gizmo(const r::vec3& g);

		extern std::function<r::vec3(const r::vec3&)>		transform;
		extern std::function<r::vec3(const r::vec3&)>		untransform;
	}
}