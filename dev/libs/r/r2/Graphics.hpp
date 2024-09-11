#pragma once

#include "Sprite.hpp"

namespace r2{
	struct Vertex {
		Vector4				pos;
		r::Color			col;
		Vector4				uv;//u0 v0 u1 v1

							Vertex() {};
							Vertex(float x, float y);
							Vertex(const r::vec2& xy);
	};

	struct Tri {
		Vector4				p0;
		Vector4				p1;
		Vector4				p2;

		r::Color			p0col;
		r::Color			p1col;
		r::Color			p2col;

							Tri() {};
							Tri(const Tri&);

		void				setP0(float x, float y, float z);
		void				setP1(float x, float y, float z);
		void				setP2(float x, float y, float z);

		void				setP0(float x, float y, float z, const r::Color & color);
		void				setP1(float x, float y, float z, const r::Color & color);
		void				setP2(float x, float y, float z, const r::Color & color);

		void				setP0(const r::Vector3& p) { setP0(p.x,p.y,p.z); };
		void				setP1(const r::Vector3& p) { setP1(p.x,p.y,p.z); };
		void				setP2(const r::Vector3& p) { setP2(p.x,p.y,p.z); };

		void				setP0(const r::Vector3 & p, const r::Color & color);
		void				setP1(const r::Vector3 & p, const r::Color & color);
		void				setP2(const r::Vector3 & p, const r::Color & color);

		void				setColor(const r::Color &color);

		float*				cooPtr() { return &p0.x; };
		float*				colorPtr() { return p0col.ptr(); };

		std::string			toString();

		Tri					unwind() const;
	};

	struct Quad {
		float				p0x = 0.f;
		float				p0y = 0.f;
		float				p0z = 0.f;

		float				p1x = 0.f;
		float				p1y = 0.f;
		float				p1z = 0.f;

		float				p2x = 0.f;
		float				p2y = 0.f;
		float				p2z = 0.f;

		float				p3x = 0.f;
		float				p3y = 0.f;
		float				p3z = 0.f;

		r::Color			p0col;
		r::Color			p1col;
		r::Color			p2col;
		r::Color			p3col;

		void				setP0(float x, float y, float z, const r::Color& color);
		void				setP1(float x, float y, float z, const r::Color& color);
		void				setP2(float x, float y, float z, const r::Color& color);
		void				setP3(float x, float y, float z, const r::Color& color);

		void				setP0(const r::Vector3& p);
		void				setP1(const r::Vector3& p);
		void				setP2(const r::Vector3& p);
		void				setP3(const r::Vector3& p);

		void				setP0(const r::Vector2& p);
		void				setP1(const r::Vector2& p);
		void				setP2(const r::Vector2& p);
		void				setP3(const r::Vector2& p);

		void				setP0(const r::Vector3& p, const r::Color& color);
		void				setP1(const r::Vector3& p, const r::Color& color);
		void				setP2(const r::Vector3& p, const r::Color& color);
		void				setP3(const r::Vector3& p, const r::Color& color);

		float*				cooPtr() { return &p0x; };
		float*				colorPtr() { return p0col.ptr(); };

		void				setColor(const r::Color& color);

		std::string			toString();

		Tri					tri0()const;
		Tri					tri1()const;
	};

	struct Arc {
		Vector3 pos;
		float	angleSize = r::Math::pi;
		float	thickness = 1.0f;
		float	radius = 16.0f;
		int		nbSegments = 0;//0 means pixel perfect count divided by 4 which is good enough for pixel games

		void	pixelPerfect();
	};

	enum class ShapeKind : int  {
		None,
		Quad,
		Circle,
		Disc,
		Triangle,
		Line,
		HollowRect,//not implemented here todo marion
		Arc,//not implemented here todo marion
	};

	struct Shape {
		ShapeKind	type{};
		Vector4		pos{};
		Vector4		sz{};
		r::Color	col;
	};

	//please keep it simple
	//for custom stuff, make an extension and use drawTriangles please
	class Graphics : public r2::Sprite {
		typedef r2::Sprite			Super;
	public:
		typedef r2::Arc				Arc;
	public:
		r::Color					geomColor;

									Graphics( r2::Node * parent  = nullptr);
		virtual						~Graphics();

		virtual void				dispose() override;
		void						clear();
		virtual void				reset() override;

		virtual Node*				clone(Node* newInst) override;
		void						drawLine(const Vector3 & start, const Vector3& end, float lineThickness = 1.f);
		void						drawLine(float x0, float y0, float x1, float y1, float lineThickness = 1.f);
		void						drawCurve(rd::TType, float x0, float y0, float xMax, float lineThickness);
		void						drawTriangle(float x0, float y0, float x1, float y1, float x2, float y2);
		void						drawTriangle(const Tri & tri);
		void						drawTriangleDoubleSided(const Tri & tri);
		void						drawQuad(float x0, float y0, float x1, float y1 );
		void						drawQuad( const Vector2&tl, const Vector2& tr, const Vector2& br, const Vector2& bl);

		//does not work?
		void						drawQuad(const Quad& q);

		//calls drawquad of course
		void						drawRect(float x0, float y0, float x1, float y1 );
		void						drawRect( const r::Vector2 & tl, const r::Vector2&size );

		void						drawDisc(float x0, float y0, float radius, int nbSegments = 0);
		void						drawDisc(const Vector3& xy, float radius, int nbSegments = 0);
		void						drawCircle(float x0, float y0, float radius, float lineThickness = 1, int nbSegments = 0);

		void						drawCircularSector(const vec3& p, float radius, float angleStart, float angleEnd, int nbSegments = 0);

		void						drawArc( const Arc & c);
		void						drawCross(float x0, float y0, float radius = 10, float lineThickness = 1);
		void						drawHollowRect(float x0, float y0, float x1, float y1, float lineThickness = 1);
		
		void						drawHorizRectGradient(Vector2 tl, Vector2 dr, r::Color cstart, r::Color cEnd);

		void						setGeomColor(const r::Color & c);
		void						setGeomColor(int col, float alpha = 1.0);

		///beware adds a thin outline for readability
		static Graphics *			fromBounds(Bounds bnd, r::Color col, r2::Node * parent = nullptr);

		///beware adds a thin outline for readability
		static Graphics *			line(const Vector2&start, const Vector2& end , const r::Color& col, float thicc = 1.0, r2::Node * parent = nullptr);
		static Graphics *			linesFromBounds(const Bounds &bnd, const r::Color& col, float thicc = 1.0, r2::Node * parent = nullptr);
		static Graphics *			rect(
			const Vector2 &pos,
			const Vector2 &size,
			const r::Color &col, r2::Node * parent = nullptr);
		///beware adds a thin outline for readability
		static Graphics *			rectFromBounds(Bounds bnd, r::Color col, r2::Node * parent = nullptr);
		static Graphics *			fromPool(r2::Node * parent = nullptr);

		static Graphics*			fromTLDR(float x, float y, float xx, float yy, r::Color col, r2::Node* parent = nullptr);

		//flat rect
		static r2::Graphics*		rect(float x, float y, float width, float height, int color, float alpha, r2::Node * parent);

		virtual Bounds				getMyLocalBounds() override;

		virtual bool				drawPrepare(rs::GfxContext * _g);
		virtual void				drawSubmitGeometry(rs::GfxContext * _g);

		virtual void				im() override;

		void						saveState();
		void						restoreState( bool andDelete = true);

		const eastl::vector<Tri>&	getTriangles() const;
		virtual	NodeType			getType()const override { return NodeType::NT_GRAPHICS; };


		//please clone and don't delete
		static r2::Graphics*		getPerfectCircle360();
	protected:
		eastl::vector<Tri>		triangles;
		eastl::vector<Tri>*		saved = nullptr;
		eastl::vector<float>	fbuf;
		
		virtual void serialize(Pasta::JReflect & jr, const char * _name) override;

	};

	struct ShapeList {
		eastl::vector<Shape>	list;

		void apply(r2::Graphics* g, bool clear = true);
		bool imType(ShapeKind& k);
		bool im();

		void writeTo(rd::Vars& v);
		bool readFrom(rd::Vars& v);
	};
}//end namespace r2

namespace std {
	std::string to_string(r2::ShapeKind k);
}

