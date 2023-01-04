#include "stdafx.h"
#include "Node.hpp"
#include "Scene.hpp"
#include "Bounds.hpp"
#include "../rs/ITweenable.hpp"
#include "rd/Pools.hpp"

using namespace rs;
using namespace r2;
using namespace Pasta;

Node::Node(Node* p) : parent(p) {
	visible = true;

	x = 0.0;
	y = 0.0;
	z = 0.0;

	scaleX = 1.0;
	scaleY = 1.0;

	rotation = 0.0;

	if (parent)
		parent->addChild(this);

	uid = rs::Sys::getUID();

#ifdef _DEBUG
	name = "Node#" + std::to_string(uid);
#endif
}

Node::~Node() {
	if (sigOnResize) {
		delete sigOnResize;
		sigOnResize = nullptr;
	}
	onDeletion.trigger();
	destroyAllChildren();
	dispose();
}

Node* Node::dummy = new Node(nullptr);

//the full glyphs size whereas getSize() will ignore the pixels under the baseline.
Pasta::Vector2 r2::Node::localToGlobal(Pasta::Vector2 pt) {
	syncMatrix();
	Pasta::Vector4 res(pt.x, pt.y, 0, 1);
	res = mat * res;
	return Pasta::Vector2(res.x, res.y);
}

//input is in node coord
//output is in scene coord
Pasta::Vector2 r2::Node::globalToLocal(Pasta::Vector2 pt) {
	syncMatrix();
	Pasta::Vector4 res(pt.x, pt.y, 0, 1);
	Pasta::Matrix44 tmat = Pasta::Matrix44(mat);
	Pasta::Matrix44 tmati = tmat.inverse();
	res = tmati * res;
	return Pasta::Vector2(res.x, res.y);
}

Bounds r2::Node::getMyLocalBounds() {
	Bounds b;
	b.empty();
	return b;
}

Bounds r2::Node::getLocalBounds() {
	if (filter && (filter->mode == FilterMode::FM_Frozen) && (filter->nbRender > 0) ) {
		syncMatrix();
		Matrix44 mat = getLocalMatrix();
		Bounds l = filter->getMyLocalBounds();
		l.transform(mat);
		return l;
	}
	else {
		return getMeasures(false);
	}
}

Bounds r2::Node::getMeasures(bool forFilters) {
	syncMatrix();
	Bounds mine = getMyLocalBounds();
	Matrix44 mat = getLocalMatrix();
	for (auto it = children.begin(); it != children.end(); it++) {
		if (*it) {
			Node* n = (*it);
			Bounds b = n->getMeasures(forFilters);
			if (!b.isEmpty()) {
				b.transform(mat);
				mine.add(b);
			}
		}
	}
	return mine;
}

Bounds r2::Node::getBounds(r2::Node* relativeTo) {
	if (!relativeTo)
		return getLocalBounds();

	if (relativeTo == this)
		return getLocalBounds();

	if (parent) {
		Matrix44 diff = parent->getRelativeMatrix(relativeTo);
		Bounds b = getLocalBounds();
		b.transform(diff);
		return b;
	}

	Matrix44 lmat = relativeTo->getGlobalMatrix().inverse();
	Bounds b = getLocalBounds();
	b.transform(lmat);
	return b;
}

Bounds r2::Node::getScreenBounds() {
	Scene* sc = getScene();
	r2::Bounds bnd = getBounds(sc);
	bnd.transform(sc->viewMatrix);
	return bnd;
}

Bounds r2::Node::getMyBounds(r2::Node* relativeTo) {
	if (!relativeTo)
		return getMyLocalBounds();

	if (relativeTo == this)
		return getMyLocalBounds();

	if (parent) {
		Matrix44 diff = parent->getRelativeMatrix(relativeTo);
		Bounds b = getMyLocalBounds();
		b.transform(diff);
		return b;
	}

	Matrix44 lmat = relativeTo->getGlobalMatrix();
	Matrix44 lmati = lmat.affineInverse();
	Bounds b = getMyLocalBounds();
	b.transform(lmati);
	return b;
}

void r2::Node::traverse(std::function< void(r2::Node*)> visit) {
	_traverse(visit);
}

void r2::Node::_traverse(std::function< void(r2::Node*)>& visit) {
	visit(this);
	for (auto it = children.begin(); it != children.end(); ++it) {
		(*it)->traverse(visit);
	}
}

void r2::Node::traverseLastChildFirst(std::function<void(r2::Node*)>& visit) {
	for (auto it = children.rbegin(); it != children.rend(); ++it) {
		(*it)->traverseLastChildFirst(visit);
	}
	visit(this);
}


Node* r2::Node::findByName(const char* name) {
	if (StrRef(this->name) == StrRef(name))
		return this;
	for (auto it = children.begin(); it != children.end(); it++) {
		if (*it) {
			Node* n = (*it)->findByName(name);
			if (n)  return n;
		}
	}
	return nullptr;
}

Node* r2::Node::findByName(const std::string& name) {
	if (this->name == name)
		return this;
	for (auto it = children.begin(); it != children.end(); it++) {
		if (*it) {
			Node* n = (*it)->findByName(name);
			if (n)  return n;
		}
	}
	return nullptr;
}

void r2::Node::findByUID(r::u64 uid, r2::Node*& node, r2::BatchElem*& elem) {
	if (uid == 0) return;

	if (this->uid == uid) {
		node = this;
		elem = nullptr;
		return;
	}

	for (auto it = children.begin(); it != children.end(); it++) {
		if (*it) {
			(*it)->findByUID(uid, node, elem);
			if (node) return;
			if (elem) return;
		}
	}
}


r2::Node* r2::Node::findByUID(r::u64 uid) {
	if (!uid) return nullptr;
	if (this->uid == uid)
		return this;
	for (auto it = children.begin(); it != children.end(); it++) {
		if (*it) {
			Node* n = (*it)->findByUID(uid);
			if (n) 
				return n;
		}
	}
	return nullptr;
}


void r2::Node::destroyAllChildren() {
	while (children.size()) {
		int sz = children.size();
		children.back()->destroy();
		int nsz = children.size();
		if (nsz == sz) {
			int brk = 0;
			break;
		}
	}
	children.clear();
	trsDirty = true;
}


void r2::Node::disposeAllChildren() {
	while (children.size())
		children.back()->dispose();
	//may rest in hierarchy...
	trsDirty = true;
}

void r2::Node::deleteAllChildren() {
	while (children.size())
		children.back()->destroy();
	children.clear();
	trsDirty = true;
}

void r2::Node::removeAllChildren() {
	while (children.size())
		removeChild(children.back());
	children.clear();
	trsDirty = true;
}

void r2::Node::reset() {
	name = "";
	alpha = 1.0f;
	resetTRS();
	visible = true;
	onDeletion.clear();
	onDestruction.clear();
	if(sigOnResize) sigOnResize->clear();
	trsDirty = true;
};

TRS r2::Node::getTRS() {
	TRS transform;
	transform.xyz.x = x;
	transform.xyz.y = y;
	transform.xyz.z = z;
	transform.scaleXscaleYRotation.x = scaleX;
	transform.scaleXscaleYRotation.y = scaleY;
	transform.scaleXscaleYRotation.z = rotation;
	return transform;
}

void r2::Node::resetTRS() {
	x = 0; y = 0; z = 0;
	scaleY = scaleX = 1.0f;
	rotation = 0;

	trsDirty = true;
	syncMatrix();
}

void r2::Node::setTRS(double x, double y, double scaleX, double scaleY, double rotation, double z) {
	this->x = x;
	this->y = y;
	this->z = z;

	this->scaleX = scaleX;
	this->scaleY = scaleY;

	this->rotation = rotation;

	trsDirty = true;
	syncMatrix();
}

void r2::Node::setTRS(const TRS& trs) {
	this->x = trs.xyz.x;
	this->y = trs.xyz.y;
	this->z = trs.xyz.z;

	this->scaleX = trs.scaleXscaleYRotation.x;
	this->scaleY = trs.scaleXscaleYRotation.y;
	this->rotation = trs.scaleXscaleYRotation.z;

	trsDirty = true;
	syncMatrix();
}

r2::Node* r2::Node::clone(r2::Node* n) {
	if (n == nullptr)
		n = new Node();
	n->name = name;
	n->setTRS(getTRS());
	n->bhv = bhv;
	if (filter)
		n->filter = filter->clone();
	for (Node* c : children)
		n->addChild(c->clone());
	n->vars = vars;//deep cloning
	return n;
}

void r2::Node::copyMatrix(r::Matrix44& _mat) {
	_mat = mat;
}

void r2::Node::overrideMatrix(const r::Matrix44& _mat) {
	mat = _mat;
	nodeFlags |= NF_MANUAL_MATRIX;
}

void r2::Node::restoreMatrix(const r::Matrix44& _mat) {
	mat = _mat;
	nodeFlags &= ~NF_MANUAL_MATRIX;
	trsDirty = true;
}

void r2::Node::setToLocalMatrix() {
	mat = getLocalMatrix();
}

Vector2 Node::getSize() {
	return getLocalBounds().getSize();
}

Vector2 r2::Node::getPos() {
	return Vector2(x, y);
}

void Node::syncChildrenMatrix() {
	for (auto c = children.begin(); c != children.end(); c++) {
		(*c)->trsDirty = true;
		(*c)->syncMatrix();
	}
}


void Node::syncMatrix() {
	if (!trsDirty) {//let's save a lot of ifs on doubles if we know answer already
		if (_x != x) { _x = x; trsDirty = true; }
		if (_y != y) { _y = y; trsDirty = true; }

		if (_scaleX != scaleX) { _scaleX = scaleX;	trsDirty = true; }
		if (_scaleY != scaleY) { _scaleY = scaleY;	trsDirty = true; }

		if (_rotation != rotation) { _rotation = rotation; trsDirty = true; }
		if (_z != z) { _z = z; trsDirty = true; }
	}

	if (trsDirty) {
		mat.setMatrix44(Pasta::Matrix44::identity);
		Pasta::Matrix44 local = getLocalMatrix();
		if (parent)
			mat.setMatrix44(parent->mat * local);
		else
			mat.setMatrix44(local);
		syncChildrenMatrix();
	}
	trsDirty = false;
}

void Node::syncAllMatrix() {
	if (trsDirty) {
		syncMatrix();
		for (auto c = children.begin(); c != children.end(); c++)
			(*c)->syncMatrix();
	}
}

Pasta::Matrix44 Node::getLocalMatrix() {
	Pasta::Matrix44 mat;
	mat.setScale(scaleX, scaleY, 1.0f);
	if (rotation) mat.rotateZ(PASTA_RAD2DEG(rotation));
	mat.translate(Pasta::Vector3(x, y, z));
	return mat;
}

Pasta::Matrix44 Node::getGlobalMatrix() {
	if (!parent) return getLocalMatrix();
	else {
		parent->syncMatrix();
		return parent->mat * getLocalMatrix();
	}
}

/**/
Pasta::Matrix44 r2::Node::getRelativeMatrix(r2::Node* to) {
	if (to == nullptr)
		return Pasta::Matrix44();
	auto myMat = getGlobalMatrix();
	auto toMat = to->getGlobalMatrix().inverse();
	return toMat * myMat;
}

bool Node::hasChild(Node* c) {
	auto idx = std::find(children.begin(), children.end(), c);
	return (idx != children.end());
}

bool Node::hasChildRec(Node* c) {
	if (this == c)
		return true;
	for (auto it = children.begin(); it != children.end(); it++) {
		if (*it) {
			bool hasChild = (*it)->hasChildRec(c);
			if (hasChild) return true;
		}
	}
	return false;
}

void r2::Node::detach() {
	if (parent) {
		parent->removeChild(this);
		parent = nullptr;
	}
}

void r2::Node::onEnterHierarchy() {
	for (auto c : children)
		c->onEnterHierarchy();
}

void r2::Node::onExitHierarchy() {
	for (auto c : children)
		c->onExitHierarchy();
}

bool Node::removeChild(Node* c) {
	if (children.size() == 0)
		return false;

	auto idx = eastl::find(children.begin(), children.end(), c);
	if (idx == children.end()) return false;

	onRemoveChild(c);

	c->onExitHierarchy();

	children.erase(idx);
	c->parent = nullptr;

	trsDirty = true;

	return true;
}

void r2::Node::setChildIndex(Node* c, int newPos) {
	int idx = getChildIndex(c);
	if (idx == -1)
		return;
	if (idx == newPos)
		return;
	if (newPos == -1)
		newPos = 0;

	removeChild(c);
	addChildAt(c, newPos);
}

void Node::addChildAt(Node* c, int pos) {
	c->detach();
	c->trsDirty = true;
	c->parent = this;
	if (pos > children.size())
		pos = children.size();
	if (pos < 0)
		pos = 0;
	children.insert(children.begin() + pos, c);
	c->onEnterHierarchy();
	onAddChild(c);
}

void r2::Node::putBehind(Node* el) {
	if (el->parent != nullptr) {
		int idx = el->parent->getChildIndex(el);
		el->parent->setChildIndex(this, idx);
	}
}

void r2::Node::putInFrontOf(Node* el) {
	if (el->parent != nullptr) {
		int idx = el->parent->getChildIndex(el);
		el->parent->setChildIndex(this, idx + 1);
	}
}

void r2::Node::toBack() {
	if (parent != nullptr)
		parent->setChildIndex(this, 0);
}

void r2::Node::backward(int nb) {
	if (parent != nullptr) {
		auto idx = parent->getChildIndex(this);
		parent->setChildIndex(this, idx - nb);
	}
}

void r2::Node::toFront() {
	if (parent != nullptr)
		parent->setChildIndex(this, parent->nbChildren() - 1);
}

void Node::addChild(Node* c) {
	if (!c)return;
	if (this==c)return;
	c->detach();
	c->trsDirty = true;
	c->parent = this;
	children.push_back(c);
	c->onEnterHierarchy();
	onAddChild(c);
}

int r2::Node::getChildIndex(Node* c) const{
	auto pos = std::find(children.cbegin(), children.cend(), c);
	if (pos == children.end()) return -1;
	return std::distance(children.cbegin(), pos);
}

int r2::Node::getDepthInHierarchy() {
	int depth = 0;
	r2::Node* parent = this->parent;
	while (parent != nullptr) {
		parent = parent->parent;
		depth++;
	}
	return depth;
}

void Node::setWidth(float w) {
	scaleX = 1.0f;
	trsDirty = true;
	float cw = width();
	scaleX = w / cw;
	trsDirty = true;
}

void Node::setHeight(float h) {
	scaleY = 1.0f; 
	trsDirty = true;
	float ch = height();
	scaleY = h / ch;
	trsDirty = true;
}

void r2::Node::constraintHeight(float height) {
	setHeight(height);
	scaleX = scaleY;
}

void r2::Node::constraintWidth(float w) {
	setWidth(w);
	scaleY = scaleX;
}

void r2::Node::setSizeVec2(const Vector2 & v) {
	//to better account for texture discretion and prevent error accum seems wise
	setScale(1.0, 1.0);
	setWidth(v.x);
	setHeight(v.y);
}

void r2::Node::setSize(float width, float height) {
	//to better account for texture discretion and prevent error accum seems wise
	setScale(1.0,1.0);
	setWidth(width);
	setHeight(height);
}

void Node::update(double dt) {
	syncMatrix();

	if (bhv) bhv(this);

	syncMatrix();

	for (int i = children.size() - 1; i >= 0; i--) {
		Node* n = children[i];
		if (n)
			n->update(dt);
	}

	syncMatrix();
}



void Node::onResize(const Vector2& _newScreenSize) {
	if (sigOnResize)
		(*sigOnResize)(_newScreenSize);
	for (int i = children.size() - 1; i >= 0; i--) {
		Node* n = children[i];
		if (n)
			n->onResize(_newScreenSize);
	}
}

void Node::dispose() {
	nodeFlags &= ~NF_UTILITY;

	vars.dispose();

	destroyAllChildren();

	if (parent) {
		parent->removeChild(this);
		parent = nullptr;
	}
	if (filter) {
		delete filter;
		filter = nullptr;
	}
}

void Node::draw(rs::GfxContext* ctx) {
	//draw self
}

void Node::drawContent(rs::GfxContext* ctx) {
	if (!visible) return;

	if (nodeFlags & NF_SKIP_DRAW) return;

	if (!(nodeFlags & NF_SKIP_HIERACHICAL_ALPHA)) 
		ctx->alpha.push_back(alpha * ctx->alpha.back());

		if (nodeFlags & NF_DRAW_CHILDREN_FIRST) {
			for (auto it = children.begin(); it != children.end(); it++)
				if ((*it) && (*it)->visible)
					(*it)->drawRec(ctx);
			draw(ctx);
		} else {
			draw(ctx);
			for (auto it = children.begin(); it != children.end(); it++)
				if ((*it) && (*it)->visible)
					(*it)->drawRec(ctx);
		}

	if (!(nodeFlags & NF_SKIP_HIERACHICAL_ALPHA))
		ctx->alpha.pop_back();
}

void Node::drawFilteringResult(rs::GfxContext* ctx) {
	//inquire about this
	//ctx->gfx->pushContext();
	//ctx->gfx->setAlpha(alpha * ctx->alpha.back());
	r2::Im::draw(ctx, mat, filter->getResult());
	//ctx->gfx->popContext();
}

void Node::drawFiltered(rs::GfxContext* ctx) {
	if (ctx->currentPass != rs::Pass::Basic)
		return;

	bool hadEarlyDepth = ctx->supportsEarlyDepth;
	ctx->supportsEarlyDepth = false;
	{
		if (filter->shouldComputeFilter())
			filter->compute(ctx, this);
		if (filter->doRenderToBackbuffer)
			drawFilteringResult(ctx);
	}
	ctx->supportsEarlyDepth = hadEarlyDepth;
}

void Node::drawRec(rs::GfxContext* ctx) {
	if (filter) {
		if (filter->isEnabled())
			drawFiltered(ctx);
		else {
			filter->invalidate();
			drawContent(ctx);
		}
	}
	else 
		drawContent(ctx);
}

double Node::width() {
	return getBounds(parent).getWidth();
}

double Node::height() {
	return getBounds(parent).getHeight();
}

Scene* Node::getScene() {
	if (!parent) return nullptr;

	Scene* spar = dynamic_cast<Scene*>(parent);
	if (spar) return spar;

	return parent->getScene();
}

double r2::Node::getValue(TVar valType) {
	switch (valType) {
	case VX:		return x;
	case VY:		return y;
	case VZ:		return z;
	case VScaleX:	return scaleX;
	case VScaleY:	return scaleY;
	case VRotation:	return rotation;
	case VScale:	return (scaleX + scaleY) / 2.0f;

	case VWidth:	return width();
	case VHeight:	return height();
	case VAlpha:	return alpha;
	default:		return 0.0;
	}

}

double r2::Node::setValue(TVar valType, double val) {
	switch (valType) {
	case VX:		x = val; trsDirty = true;			break;
	case VY:		y = val; trsDirty = true;			break;
	case VZ:		z = val; trsDirty = true;			break;
	case VScaleX:	scaleX = val;	trsDirty = true;	break;
	case VScaleY:	scaleY = val;	trsDirty = true;	break;
	case VRotation:	rotation = val;	trsDirty = true;	break;
	case VScale:	scaleX = scaleY = val;	trsDirty = true; break;

	case VWidth:	setWidth(val);			break;
	case VHeight:	setHeight(val);			break;
	case VAlpha:	alpha = val;			break;
	default: {

	}
	};
	return val;
}

void r2::Node::drawTo(Pasta::Texture* t, Pasta::FrameBuffer* fb, Scene* _s) {
	Pasta::Graphic* gfx = Pasta::Graphic::getMainGraphic();
	rs::GfxContext g(gfx);

	bool needDispose = false;
	Scene* s = _s;
	if (!s) s = getScene();
	if (!s) {
		s = new r2::Scene(); needDispose = true;
	}

	s->drawInto(&g, this, t, fb);

	if (needDispose) {
		s->dispose();
		delete s;
	}
}

void r2::Node::toPool() {
	rd::Pools::free(this);
}

void r2::Node::destroy() {
	if (nodeFlags & NF_SKIP_DESTRUCTION) return;

	onDestruction.trigger();

	if (nodeFlags & NF_ORIGINATES_FROM_POOL)
		toPool();
	else
		delete this;
}

eastl::vector<r2::Node*> r2::Node::collect(r2::Node* tgt) {
	eastl::vector<r2::Node*> mine;
	_collect(tgt, &mine);
	return mine;
}

void r2::Node::_collect(r2::Node* tgt, eastl::vector<r2::Node*>* cur) {
	cur->push_back(tgt);
	for (auto c : tgt->children)
		_collect(c, cur);
}

r2::Node* r2::Node::fromPool(r2::Node * parent){
	auto n = rd::Pools::nodes.alloc();
	if (parent)
		parent->addChild(n);
	return n;
}