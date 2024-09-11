#pragma once
#include "r3/Sprite3D.hpp"

namespace r3 {
	class Cube3D : public Sprite3D {
	public:
						Cube3D(r2::Node* parent);

		virtual void	drawSubmitGeometry(rs::GfxContext* _g) override;
	};

	class Icosphere3D : public Sprite3D {
		float*			vertexData = nullptr; 
		int				vertexCount;
		Pasta::u32*		indexData = nullptr;  
		int				indexCount;

		void			makeIcosphere(int order = 0);
	public:
						Icosphere3D(r2::Node* parent, int order = 0);
		virtual			~Icosphere3D();

		virtual void	drawSubmitGeometry(rs::GfxContext* _g) override;
	};

	class Arrow3D : public Sprite3D {
		eastl::vector<float>	buff;
		eastl::vector<u32>		indices;

		void			makeArrow();
	public:
						Arrow3D(r2::Node* parent);

		virtual void	drawSubmitGeometry(rs::GfxContext* _g) override;
	};
}