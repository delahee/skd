#pragma once

#include "Sprite.hpp"
#include "Graphics.hpp"

namespace r2{

	//please keep it simple
	//for sophisticated stuff, see Graphics
	//this is an emulation for sf::vertexArray which is easy to mock & port
	class VertexArray : public r2::Sprite {
		typedef r2::Sprite			Super;
	public:
									VertexArray( r2::Node * parent  = nullptr);
		virtual						~VertexArray();

		virtual void				dispose() override;
		void						clear();
		virtual void				reset() override;

		virtual Node*				clone(Node* newInst) override;

		void						add(const r2::Vertex & vtx);

		virtual Bounds				getMyLocalBounds() override;

		virtual bool				drawPrepare(rs::GfxContext * _g);
		virtual void				drawSubmitGeometry(rs::GfxContext * _g);

		virtual void				im() override;

		void						saveState();
		void						restoreState( bool andDelete = true);

		const eastl::vector<r2::Vertex>&	
									getVertices() const;
		virtual	NodeType			getType() const override { return NodeType::NT_GRAPHICS; };

		virtual Tile*				getPrimaryTile() override;
		Pasta::PrimitiveType::Enum	primType{ Pasta::PrimitiveType::Triangles };
		Pasta::Texture*				tex = 0;
		Tile						tile;
	protected:
		eastl::vector<r2::Vertex>	vertices;
		eastl::vector<r2::Vertex>*	saved = nullptr;
		eastl::vector<float>		fbuf;
		
		virtual void serialize(Pasta::JReflect & jr, const char * _name) override;
	};

}//end namespace r2

