#pragma once 

#include "Node.hpp"
#include "Rect.hpp"
#include "Bounds.hpp"

//warning getLocalBounds and getMeasure are probably inaccurate here
namespace r2{
class Scissor : public Node {
public:
							Rect rect;
	bool					isRelative = true;//still wonder if it serves a purpose otherwise
public:
			
							Scissor(Node * parent  = nullptr);
							Scissor(Rect r, Node * parent = nullptr);
	

	void					drawRec(rs::GfxContext * g);

	virtual Bounds			getMyLocalBounds() override;
	virtual	NodeType		getType() const override { return NodeType::NT_SCISSOR; };
private:
	
};
}