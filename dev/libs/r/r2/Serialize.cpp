#include "stdafx.h"

#include "r/Types.hpp"
#include "Serialize.hpp"
#include "rd/JSerialize.hpp"
#include "rd/TileLib.hpp"
#include "r2/im/TilePicker.hpp"
#include "rd/AudioEmitter2D.hpp"
#include "filter/Layer.hpp"
#include "filter/Blur.hpp"
#include "filter/Glow.hpp"
#include "filter/Bloom.hpp"
#include "filter/ColorMatrix.hpp"

using namespace std;
using namespace r2;

#define SUPER r2::Node
function<r2::Node* (NodeType nt)> r2::Serialize::customTypeResolver;

template<> void Pasta::JReflect::visit(NodeType& c, const char* name) {
	if (name) visitObjectBegin(name);
	visitInt((int&)c, "nodeType");
	if (name) visitObjectEnd(name);
};

template<> void Pasta::JReflect::visit(pair<u64,NodeType>& c, const char* name) {
	if (name) visitObjectBegin(name);
	visit(c.first,"uid");
	visit(c.second,"nodeType");
	if (name) visitObjectEnd(name);
};

static r2::Node* emptyNode(NodeType nt) {
	switch (nt)	{
		case NodeType::NT_NODE:			return new r2::Node();
		case NodeType::NT_TEXT:			return new r2::Text();
		case NodeType::NT_BATCH: {
			auto b = new r2::Batch();
			b->name = "Deserialized Batch " + to_string(b->uid);
			return b;
		}
		case NodeType::NT_GRAPHICS:		return new r2::Graphics();
		case NodeType::NT_INTERACT:		return new r2::Interact();
		case NodeType::NT_SCENE:		return new r2::Scene();
		case NodeType::NT_SCISSOR:		return new r2::Scissor();
		case NodeType::NT_BMP:			return new r2::Bitmap();
		case NodeType::NT_ABMP:			return new rd::ABitmap();
		case NodeType::NT_AUDIO2D:		return new rd::AudioEmitter2D();
		default:						return r2::Serialize::customTypeResolver(nt);
	}
}

void r2::Node::serializeChildren(Pasta::JReflect& jr) {
	std::vector<r2::Node*> serializedChildren;
	if (!jr.isReadMode()) {
		for (auto& c : children)
			if (!c->isUtility() && !(c->nodeFlags & NF_SKIP_SERIALIZATION)) 
				serializedChildren.push_back(c);
	}
	//else s'okay will be handled later
	u32 nbChildren = serializedChildren.size();
	jr.visit(nbChildren, "nbChildren");
	if (jr.isReadMode()) serializedChildren.resize(nbChildren);
	std::vector<int> ctypes;
	if (jr.isReadMode())
		ctypes.resize(nbChildren);
	else
		for (auto& c : serializedChildren)
			ctypes.push_back((int)c->getType());
	jr.visit(ctypes, "children_types");
	if (jr.visitArrayBegin("children", nbChildren)) {
		for (Pasta::u32 i = 0; i < nbChildren; ++i) {
			jr.visitIndexBegin(i);
			jr.visitObjectBegin(nullptr);

			if (i == 124) {
				int nop = 0;
			}
			r2::Node* n = nullptr;
			NodeType nt = NodeType::NT_NODE;
			if (jr.isReadMode()) {
				if (i < ctypes.size())
					nt = (NodeType)ctypes[i];
				n = emptyNode(nt);
			}
			else {
				n = serializedChildren[i];
				nt = n->getType();
			}

			//if (!jr.isReadMode())
			//	jr.visit(i, "idx");

			if (n)
				jr.visit(*n, nullptr);
			#if _DEBUG
			else
				cout << "hollow children list?" << "\n";
			#endif

			if (jr.isReadMode())
				serializedChildren[i] = n;

			jr.visitObjectEnd(nullptr);
			jr.visitIndexEnd();
			//cout << i << " " << nt << " " << n->uid << "\n";
		}
	}
	jr.visitArrayEnd("children");

	if (jr.isReadMode())
		for (auto c : serializedChildren) {
			if(!c) continue;//avoid mis serialized node errors

			r2::Node* prev = findByUID(c->uid);
			if (prev)
				prev->destroy();
			addChild(c);
		}
}

static r2::Filter* createFilter(r2::FilterType ft) {
	switch (ft)
	{
	case FilterType::FT_NONE:			break;
	case FilterType::FT_BASE:			return new r2::Filter();
	case FilterType::FT_LAYER:			return new r2::filter::Layer();
	case FilterType::FT_BLUR:			return new r2::filter::Blur();
	case FilterType::FT_GLOW:			return new r2::filter::Glow();
	case FilterType::FT_BLOOM:			return new r2::filter::Bloom();
	case FilterType::FT_COLORMATRIX:	return new r2::filter::ColorMatrix();
	case FilterType::FT_USER:			throw "oh please god no!";
	
	default:
		break;
	}
	return 0;
}

void r2::Node::serializeArray(Pasta::JReflect& jr, eastl::vector<r2::Node*>& arr, u32 size, const char* name) {
	u32 similarCount = 0;
	if (jr.visitArrayBegin(name, size)) {
		if (jr.isReadMode() && size)
			arr.resize(size);
		for (u32 i = 0; i < size; ++i) {
			jr.visitIndexBegin(i);
			jr.visitObjectBegin(nullptr);
			if (jr.isReadMode()) {
				NodeType t;
				jr.visit((int&)t, "type");
				arr[i] = emptyNode(t);
			}
			jr.visit(*arr[i], nullptr);
			jr.visitObjectEnd(nullptr);
			if (jr.visitIndexEnd())
				++similarCount;
		}
	}
	jr.visitArrayEnd(name);
	if (!jr.isReadMode() && similarCount == size)
		jr.m_jvalue->EraseMember(name);
}

void r2::Node::serialize(Pasta::JReflect& jr, const char* _name) {
	if (rd::Bits::is(nodeFlags, NF_SKIP_SERIALIZATION))
		return;
	if (_name) jr.visitObjectBegin(_name);
	jr.visit(name, "name");
	jr.visit(uid, "uid");
	if (jr.isReadMode())
		rs::Sys::reserveUID(uid);

	NodeType t = getType();
	jr.visit((int&)t, "type");

	jr.visit(visible, "visible");
	jr.visit(nodeFlags, "nodeFlags");
	jr.visit(x, "x");
	jr.visit(y, "y");
	jr.visit(z, "z");
	jr.visit(rotation, "rot");
	jr.visit(scaleX, "scaleX");
	jr.visit(scaleY, "scaleY");

	bool hasFilter = filter != 0;
	jr.visit(hasFilter, "hasFilter ");
	if (hasFilter) {
		if (jr.isReadMode()) {
			FilterType ft = FilterType::FT_BASE;
			jr.visit((int&) ft, "filterType");
			filter = createFilter(ft);
		}
		else {
			jr.visit( (int&)filter->type, "filterType");
		}
	}
	if (hasFilter) {
		filter->serialize(jr, "filter");
	}

	vars.serialize(&jr, "vars");

	if (parent && !jr.m_read)
		jr.visit(parent->uid, "parentUID");

	if (!(nodeFlags & NF_SKIP_CHILD_SERIALIZATION))
		serializeChildren(jr);

	if (_name) jr.visitObjectEnd(_name);
	if (jr.m_read) 
		rs::Sys::reserveUID(uid);
}



void r2::Scene::serialize(Pasta::JReflect & jr, const char * _name) {
	if (rd::Bits::is(nodeFlags, NF_SKIP_SERIALIZATION))
		return;
	if (_name) jr.visitObjectBegin(_name);
	r2::Node::serialize(jr, nullptr);
	jr.visit(fixedSize, "fixedSize");
	jr.visit(doClear, "doClear");
	jr.visit(clearColor, "clearColor");
	jr.visit(cameraScale, "cameraScale");
	jr.visit(areDimensionsOverriden, "areDimensionsOverriden");
	jr.visit(sceneWidth, "sceneWidth");
	jr.visit(sceneHeight, "sceneHeight");
	if (_name) jr.visitObjectEnd(_name);
};

void r2::EarlyDepthScene::serialize(Pasta::JReflect & jr, const char * _name) {
	if (rd::Bits::is(nodeFlags, NF_SKIP_SERIALIZATION))
		return;
	if (_name) jr.visitObjectBegin(_name);
	r2::Scene::serialize(jr, nullptr);
	jr.visit(skipBasic, "skipBasic");
	jr.visit(skipClip, "skipClip");
	if (_name) jr.visitObjectEnd(_name);
};

void r2::Interact::serialize(Pasta::JReflect& jr, const char* _name) {
	if (rd::Bits::is(nodeFlags, NF_SKIP_SERIALIZATION))
		return;
	if (_name) jr.visitObjectBegin(_name);
	r2::Node::serialize(jr, nullptr);
	jr.visit(rectWidth, "rectWidth");
	jr.visit(rectHeight, "rectHeight");
	jr.visit(enabled, "enabled");
	if (_name) jr.visitObjectEnd(_name);
}

void r2::Sprite::serialize(Pasta::JReflect& jr, const char* _name) {
	if (rd::Bits::is(nodeFlags, NF_SKIP_SERIALIZATION))
		return;
	if (_name) jr.visitObjectBegin(_name);
	r2::Node::serialize(jr, nullptr);
	jr.visit(useSimpleZ, "useSimpleZ");
	jr.visit(killAlpha, "killAlpha");
	jr.visit(depthRead, "depthRead");
	jr.visit(depthWrite, "depthWrite");
	jr.visit((int&)blendmode, "blendmode");
	jr.visit(zTopOffset, "zTop");
	jr.visit(zBottomOffset, "zBottom");
	jr.visit(color, "color");
	jr.visit((int&)texFiltering, "texFiltering");
	jr.visit((int&)shader, "shader");
	jr.visit(shaderFlags, "shaderFlags");

	if (shader != Shader::SH_Basic) {
		bool hasShaderValues = shaderValues.head;
		jr.visit(hasShaderValues, "hasShaderValues");
		if (hasShaderValues && jr.isReadMode())
			shaderValues.head = new Anon();
		if (hasShaderValues)
			jr.visit(*shaderValues.head, "shaderValues");
	}
	if (_name) jr.visitObjectEnd(_name);
}

void r2::Bitmap::serialize(Pasta::JReflect & jr, const char * _name) {
	if (rd::Bits::is(nodeFlags, NF_SKIP_SERIALIZATION))
		return;
	if (_name) jr.visitObjectBegin(_name);
	r2::Sprite::serialize(jr, nullptr);
	jr.visit(*tile, "tile");
	if (jr.isReadMode()) {
		if (vars.has("rd::TileGroup")) {
			//we have the group in case we need to fix the instance automatically?
			//const char* group = getMetadata("rd::TileGroup")->asString();

			const char* libName = vars.get("rd::TileLib")->asString();
			//todo manage multi page textures
			rd::TileLib * lib = r2::im::TilePicker::getOrLoadLib(libName);
			if(lib) tile->setTexture(lib->tex);
		}
	}
	if (_name) jr.visitObjectEnd(_name);
};

void rd::ABitmap::serialize(Pasta::JReflect & jr, const char * _name) {
	if (rd::Bits::is(nodeFlags, NF_SKIP_SERIALIZATION))
		return;
	if (_name) jr.visitObjectBegin(_name);
	r2::Sprite::serialize(jr, nullptr);

	Pasta::JReflect & functor = jr;
	JREFLECT(destroyed);
	JREFLECT(flippedX);
	JREFLECT(flippedY);
	
	if (jr.m_read) {
		std::string libName;
		jr.visit(libName, "libName");
		if (libName.size()) {
			lib = r2::im::TilePicker::getOrLoadLib(libName);
			if (lib == nullptr) {
				vars.set("WARNING", string("Cannot find back library ") + libName);
			}
		}
		else {
			//tile was from other origin
		}
	}
	else{
		if (lib) {
			jr.visit(lib->name, "libName");
		}
		else {
			//tile was from other origin
		}
	}
	
	JREFLECT(groupName);

	TileAnim * anm = player.getLastAnim();
	bool hasAnm = anm != nullptr;
	jr.visitBool(hasAnm, "hasAnim");

	if (jr.m_read) {
		set(lib, groupName);
		if (groupName.size()) {
			player.play(groupName.c_str());
		}
		anm = player.getLastAnim();
	}
	else {

	}
	if (hasAnm) {
		if(anm)
			jr.visit(*anm,"lastAnim");
		if (!jr.m_read) {
			if(lib)
				anm->libName = lib->name.c_str();
			anm->groupName = groupName.c_str();
		}
	}
	JREFLECT(player);
	JREFLECT(frame);
	JREFLECT(usePivot);
	JREFLECT(pivotX);
	JREFLECT(pivotY);
	syncTile();

	if (_name) jr.visitObjectEnd(_name);

	
};

void r2::Graphics::serialize(Pasta::JReflect & jr, const char * _name) {
	if (rd::Bits::is(nodeFlags, NF_SKIP_SERIALIZATION))
		return;
	if (_name) jr.visitObjectBegin(_name);
	r2::Sprite::serialize(jr, nullptr);
	jr.visit(geomColor, "geomColor");

	Pasta::u32 nbTri = triangles.size();
	jr.visit(nbTri, "nbTri");
	if (jr.m_read&&nbTri) triangles.resize(nbTri);

	//because visitArray doesn't like nested anymous objects
	if (jr.visitArrayBegin("triangles", nbTri)){
		for (Pasta::u32 i = 0; i < nbTri; ++i){
			jr.visitIndexBegin(i);
			jr.visitObjectBegin(nullptr);
			jr.visit(triangles[i], nullptr);
			jr.visitObjectEnd(_name, true);
			jr.visitIndexEnd();
		}
	}
	jr.visitArrayEnd("triangles");

	if (_name) jr.visitObjectEnd(_name,true);
};


void r2::BatchElem::serialize(Pasta::JReflect& jr, const char* _name) {
	if (rd::Bits::is(beFlags, NF_SKIP_SERIALIZATION))
		return;
	if (_name) jr.visitObjectBegin(_name);
	jr.visit(name, "name");
	jr.visit(uid, "uid");

	if (jr.isReadMode())
		rs::Sys::reserveUID(uid);

	BeType t = getType();
	jr.visit((int&)t, "type");
	jr.visit((int&)blendmode, "blend");

	jr.visit(useSimpleZ, "useSimpleZ");
	jr.visit(destroyed, "destroyed");
	if (jr.isReadMode())
		destroyed = false;
	jr.visit(ownsTile, "ownsTile");
	jr.visit(visible, "visible");
	jr.visit(beFlags, "beFlags");
	jr.visit(x, "x");
	jr.visit(y, "y");
	jr.visit(z, "z");
	jr.visit(rotation, "rot");
	jr.visit(scaleX, "scaleX");
	jr.visit(scaleY, "scaleY");
	jr.visit(zTopOffset, "zTopOffset");
	jr.visit(zBottomOffset, "zBottomOffset");
	jr.visit(priority, "priority");
	jr.visit(color, "color");
	jr.visit(alpha, "alpha");

	if (jr.m_read)
		rs::Sys::reserveUID(uid);
	if (_name) jr.visitObjectEnd(_name);
}

void rd::ABatchElem::serialize(Pasta::JReflect& jr, const char* _name) {
	if (rd::Bits::is(beFlags, NF_SKIP_SERIALIZATION))
		return;

	if (_name) jr.visitObjectBegin(_name);

	r2::BatchElem::serialize(jr, nullptr);

	jr.visit(groupName,"group");

	std::string libName;
	if (lib) 
		libName = lib->name;
	jr.visit(libName, "libName");
	
	if( jr.isReadMode())
	if( r2::im::TilePicker::hasLib(libName.c_str())){
		auto lib = r2::im::TilePicker::getOrLoadLib(libName.c_str());
		set(lib, groupName.c_str());
	}

	jr.visit(player, "player");
	jr.visit(usePivot,"usePivot");
	jr.visit(pivotX,"pivotX");
	jr.visit(pivotY, "pivotY");

	if (jr.isReadMode())
		if (usePivot)
			setCenterRatio(pivotX, pivotY);
	
	if (_name) jr.visitObjectEnd(_name);
}

void r2::ColorMatrixControl::serialize(Pasta::JReflect* jr, const char* _name) {
	if (_name) jr->visitObjectBegin(_name);

	jr->visit(fresh, "fresh");
	jr->visit((int&)mode, "mode");
	jr->visit(hue, "hue");
	jr->visit(sat, "sat");
	jr->visit(val, "val");
	jr->visit(tint, "tint");
	jr->visit(ratioNew, "ratioNew");
	jr->visit(ratioOld, "ratioOld");
	jr->visit(mat, "mat");

	if (_name) jr->visitObjectEnd(_name);
}

void r2::Text::serialize(Pasta::JReflect& f, const char* _name) {
	if (rd::Bits::is(nodeFlags, NF_SKIP_SERIALIZATION))
		return;
	if (_name) f.visitObjectBegin(_name);
	r2::Batch::serialize(f, 0);
	f.visit(bgColor,"bgColor");
	f.visit(blockAlign,"blockAlign");

	bool hasFont = fnt != 0;
	f.visit(hasFont, "hasFont");
	if (hasFont) {
		string fontName;
		f.visit(fontName, "fontName");
		if(fontName.size())
			setFont(rd::FontManager::get().getFont(fontName));
	}

	f.visit(text, "text");

	bool hasDs = dropShadow != 0;
	f.visit(hasDs, "hasDropShadow");
	if (hasDs) {
		if (f.isReadMode())
			dropShadow = new DropShadow();
		dropShadow->serialize(&f, "dropShadow");
	}

	bool hasOutline = outline != 0;
	f.visit(hasOutline, "hasOutline");
	if (hasOutline) {
		if (f.isReadMode())
			outline = new Outline();
		outline->serialize(&f, "outline");
	}

	f.visit(colors, "colors");
	setText(text);
	if (_name) f.visitObjectEnd(_name);
}

void r2::DropShadow::serialize(Pasta::JReflect* f, const char* _name) {
	if (_name) f->visitObjectBegin(_name);
	f->visit(dx, "dx");
	f->visit(dy, "dy");
	f->visit(col, "col");
	if (_name) f->visitObjectEnd(_name);
}

void r2::Outline::serialize(Pasta::JReflect* f, const char* _name) {
	if (_name) f->visitObjectBegin(_name);
	f->visit(col, "col");
	if (_name) f->visitObjectEnd(_name);
}


template<> void Pasta::JReflect::visit(r2::TextColorRange & c, const char* name) {
	c.serialize(this, name);
};

void r2::TextColorRange::serialize(Pasta::JReflect* jr, const char* _name) {
	jr->visitObjectBegin(_name);
	jr->visit(start,"start");
	jr->visit(end,"end");
	jr->visit(col,"col");
	jr->visit(multiply,"multiply");
	jr->visitObjectEnd(_name);
}
#undef SUPER