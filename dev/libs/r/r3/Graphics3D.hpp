
#pragma once

#include "r3/Node3D.hpp"
#include "r3/Bounds.hpp"
#include "r3/Sprite3D.hpp"

namespace r3 {
	class Graphics3D : public Sprite3D {
		typedef Sprite3D			Super;
		typedef r2::Tri				Tri;//in case we want to superseed it easierly at some point
		typedef r2::Quad			Quad;
	public:
		r::Color					geomColor;

									Graphics3D(r2::Node* n);
		virtual						~Graphics3D() {
		};

		void						setGeomColor(const r::Color& c);
		void						setGeomColor(int col, float alpha = 1.0);

		virtual void				dispose() override;
		virtual void				reset() override;

		virtual r2::Bounds			getMyLocalBounds() override;
		virtual void				drawSubmitGeometry(rs::GfxContext* _g);

		const eastl::vector<r2::Tri>&	getTriangles() const;
		virtual	NodeType				getType()const override { return NodeType::NT_GRAPHICS3D; };

		void						saveState();
		void						restoreState(bool andDelete = true);
		void						clear();

		void						draw3DCircle(const r::Vector3& vtx, const r::Vector3& _up, const r::Vector3& _normal, float radius, const r::Color& col, int nbSegments, float thicc);
		void						drawLine(const Vector3& start, const Vector3& end, float lineThickness = 1.f);
		void						drawLine(float x0, float y0, float x1, float y1, float lineThickness = 1.f);

		void						drawQuad(const Quad& q);
		void						drawTriangle(float x0, float y0, float x1, float y1, float x2, float y2);
		void						drawTriangleDoubleSided(const Tri& tri);
		void						drawHollowRect(float x0, float y0, float x1, float y1, float lineThickness = 1);
		void						drawTriangle(const Tri& tri);

		virtual void				im() override;

	protected:
		eastl::vector<Tri>			triangles;
		eastl::vector<Tri>*			saved = nullptr;
		eastl::vector<float>		fbuf;

		virtual void serialize(Pasta::JReflect& jr, const char* _name) override;

	};
}