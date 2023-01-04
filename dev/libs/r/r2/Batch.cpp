#include "stdafx.h"

#include <EASTL/vector.h>
#include <vector>
#include <algorithm>

#include "platform.h"
#include "1-graphics/geo_vectors.h"
#include "1-graphics/Graphic.h"
#include "1-graphics/GraphicContext.h"
#include "1-graphics/GraphicEnums.h"
#include "1-graphics/ShaderProgram.h"
#include "1-graphics/ShaderParam.h"

#include "Sprite.hpp"
#include "Batch.hpp"
#include "BatchElem.hpp"

using namespace r2;
using namespace Pasta;

#define SUPER Sprite

Batch::Batch( Node * parent ) : SUPER(parent) {
	name = "Batch#" + std::to_string(uid);
}

Batch::~Batch() {
	dispose();
}

void r2::Batch::dispose() {
	destroyAllElements();
	SUPER::dispose();
	beforeDrawFlush = nullptr;
}

r2::BatchElem* r2::Batch::getElement(int idx) {
	if ( head == nullptr)
		return nullptr;

	BatchElem * nth = head;
	while (idx > 0) {
		idx--;
		nth = nth->next;
		if (nth == nullptr)
			return nullptr;
	}

	return nth;
}

r2::BatchElem* r2::Batch::getElementByName(const char* name) {
	BatchElem* cur = head;
	while (cur) {
		if (rd::String::equals(name, cur->name.c_str()))
			return cur;
		cur = cur->next;
	}
	return nullptr;
}

std::vector<r2::BatchElem*> r2::Batch::getAllElements() {
	std::vector<r2::BatchElem*> e;
	for (BatchElem * it = head; it != nullptr; it = it->next) 
		e.push_back(it);
	return e;
}

int r2::Batch::getNbElements() {
	return nbElems;
}

void r2::Batch::changePriority(r2::BatchElem * be, double nprio) {
	if (!be) return;

	be->priority = nprio;

	if (head == be) {//nothing to be done, we were the head, reinserting is safer
		remove(be);
		add(be);
		return;
	}

	if (be->batch != this) {//ok not even here
		add(be);
		return;
	}

	//todo make dual
	if (be->prev->getPriority() <= nprio ) {//get on top of all under or equal to mirror the same in the other order
		BatchElem * prev = be->prev;
		be->remove();

		while (prev->getPriority() <= nprio) {//the equal is important, it mimics perfectly the old remove/add
			prev = prev->prev;
			if (prev == nullptr) {
				remove(be);
				add(be);//we should be head, fast escape
				return;
			}
		}

		BatchElem * nnext = prev->next;
		if (nnext)
			nnext->prev = be;
		be->next = nnext;

		prev->next = be;
		be->prev = prev;
		
		be->batch = this;//don't forget to tody
		nbElems++;
	}
	else {//be->prev->getPriority() > nprio
		BatchElem * next = be->next;
		if (next == nullptr) return;

		be->remove();

		while (next->getPriority() > nprio) {//the order is still preserved like a remove/add call pair
			if (next->next == nullptr) {
				next->next = be;
				be->prev = next;

				be->batch = this;
				nbElems++;
				return;
			}
			else 
				next = next->next;
		}

		BatchElem * nprev = next->prev;
		if (nprev) {
			nprev->next = be;
			be->prev = nprev;
		}
		be->next = next;
		next->prev = be;

		be->batch = this;
		nbElems++;
	}

}

void r2::Batch::bindTile(rs::GfxContext*g, r2::Tile* t){
	currentRenderingTile = t;
	if (t) 
		bindTexture(g, t->getTexture());
	else
		bindTexture(g, nullptr);
}

void r2::Batch::setShader(rs::GfxContext* ctx) {
	Pasta::Graphic* g = ctx->gfx;
	bindShader(ctx);

	if (ctx->currentPass == rs::Pass::Picking) {
		Pasta::ShaderParam *param = g->getShader()->getParam("uColor");
		if (param)
			param->setValue(Pasta::Color::White.ptr());
	}
}

Tile * r2::Batch::getPrimaryTile() {
	return currentRenderingTile;
}

void r2::Batch::setTransparency(rs::GfxContext* ctx, BatchElem * elem) {
	auto g = ctx->gfx;
	switch (ctx->currentPass) {
	case rs::Pass::EarlyDepth:
		g->setBlendState(r2::GpuObjects::opaqueNoColor);
		return;
	case rs::Pass::Picking:
		g->setTransparencyType(TransparencyType::TT_OPAQUE);
		break;
	default:
		if (elem->blendmode < TransparencyType::TRANSPARENCY_TYPE_COUNT)
			g->setTransparencyType(elem->blendmode);
		else
			g->setTransparencyType(blendmode);
		break;
	}
	g->applyContextTransparency();
}

void r2::Batch::draw(rs::GfxContext* ctx) {
	if (!shouldRenderThisPass(ctx)) return;

	Pasta::Graphic* g = ctx->gfx;
	
	//prepare sizing
	auto nbVisible = 0;
	for (BatchElem * it = head; it != nullptr; it = it->next)
		if (it->shouldRender(ctx))
			nbVisible++;
	if (nbVisible == 0)
		return;
	int count = nbVisible;

	const int stride = 3 + 2 + 4 + (hasNormals ? 3 : 0);
	const int verts = 4; // doesn't take into account custom BatchElem
	const int tri = 2;	

	nbSubmit = 0;
	fbuf.clear();
	if(fbuf.capacity() < count * verts * stride)
		fbuf.reserve(count * verts * stride);
	ibuf.clear();
	if (ibuf.capacity() < count * tri * 2)
		ibuf.reserve(count * tri * 2);

	if (useExtraPass && ctx->currentPass == rs::Pass::ExtraPass0 && beforeExtraPass) beforeExtraPass(this);

	//prepare context
	g->pushContext();

	if (trsDirty && !(nodeFlags & NF_MANUAL_MATRIX)) syncMatrix();
	ctx->loadModelMatrix(mat);
	
	BatchElem * tHead = nullptr;
	for (auto it = head; it != nullptr; it = it->next) {
		if (it->shouldRender(ctx)) {
			tHead = it;
			break;
		}
	}

	if (!tHead) {
		g->popContext();
		return;
	}

	applyDepth(ctx);

	g->setVertexDeclaration(VD_POSITIONS | VD_TEXCOORDS | VD_COLORS | (hasNormals ? VD_NORMALS : 0));

	Pasta::Color c(color.r, color.g, color.b, color.a * alpha * ctx->alpha.back());
	g->setColor(c);
	g->setAlpha(c.a);
	if (additionnalTexture != nullptr)
		g->setTexture(Pasta::ShaderStage::Fragment, 1, additionnalTexture);
	
//------------------
#define SEND_BATCH() \
	bindTile(ctx, tHead->tile); \
	setShader(ctx); \
	setTransparency(ctx, tHead); \
	if( beforeDrawFlush) beforeDrawFlush(); \
	g->drawIndexed(PrimitiveType::Triangles, fbuf.data(), fbuf.size() / stride, ibuf.data(), ibuf.size()); \
	fbuf.clear(); \
	ibuf.clear(); \
	nbSubmit++;

//------------------

	int nbDrawn = 0;
	BatchElem * tI = tHead;
	while (tI) {
		if (!tI->shouldRender(ctx)) {
			tI = tI->next;
			continue;
		}

		if (tI->getTexture() != tHead->getTexture() || tI->blendmode != tHead->blendmode) {
			SEND_BATCH();
			tHead = tI;
		}

		if (ctx->currentPass == rs::Pass::Picking) {
			const r::Color oldColor = tI->color;
			const float oldAlpha = tI->alpha;
			tI->color.r = ((tI->uid      ) & 0x00FFFFFF);
			tI->color.g = ((tI->uid >> 24) & 0x00FFFFFF);
			tI->color.b = ((tI->uid >> 48) & 0x0000FFFF);
			tI->color.a = 1.0f;
			tI->alpha = 1.0f;
			tI->pushQuad(fbuf, ibuf);
			tI->color = oldColor;
			tI->alpha = oldAlpha;
		}
		else if (hasNormals) tI->pushQuadNormals(fbuf, ibuf);
		else tI->pushQuad(fbuf, ibuf);
		
		nbDrawn++;
		tI = tI->next;
	}
	SEND_BATCH();
#undef SEND_BATCH

#ifdef _DEBUG
	ctx->nbBatchElemDrawn += nbDrawn;
	ctx->nbDraw += nbSubmit;
#endif

	//post cleanup
	g->setVertexDeclaration(0);
	g->setTexture(ShaderStage::Fragment, 0, NULL);
	g->popContext();

	if (useExtraPass && ctx->currentPass == rs::Pass::ExtraPass0 && afterExtraPass) afterExtraPass(this);
}

void Batch::remove(BatchElem * e ) {
	if (e->batch != this) return;

	if (e->prev == nullptr) {
		if (head == e) {
			head = e->next;
			e->prev = nullptr;
		}
	}
	else
		e->prev->next = e->next;

	if (e->next != nullptr)
		e->next->prev = e->prev;

	e->prev = nullptr;
	e->next = nullptr;
	e->batch = nullptr;
	nbElems--;
}

struct PrioCompare {
	constexpr bool operator()(const BatchElem* lhs, const BatchElem * rhs) const {
		return lhs->getPriority() > rhs->getPriority();
	}
};

void Batch::add(BatchElem * e) {
	if (e->batch && e->batch != this) e->batch->remove(e);

	e->batch = this;

	if (head == nullptr) { //cool !
		e->prev = nullptr;
		e->next = nullptr;
		head = e;
		nbElems++;
		return;
	}

	BatchElem * cur = head;
	double nprio = e->getPriority();

	while (cur->getPriority() > nprio) {
		if (cur->next == nullptr) {//insert in place no need to wait
			e->prev = cur;
			e->next = nullptr;
			cur->next = e;
			nbElems++;
			return;
		}
		else
			cur = cur->next;
	}

	if (head == cur) //mod head
		head = e;

	BatchElem * oprev = cur->prev;

	//process prev
	if (oprev) {
		e->prev = oprev;
		oprev->next = e;
	}

	//set next
	e->next = cur;
	cur->prev = e;
	nbElems++;
}

bool r2::Batch::sanityCheck() {
	int c = 0;
	BatchElem * cur = head;
	BatchElem * prev = nullptr;
	while (cur) {
		if (prev != cur->prev) 
			return false;
		if (prev && (prev->getPriority() < cur->getPriority()))
			return false;
		if (cur->batch != this)
			return false;
		prev = cur;
		cur = cur->next;
		c++;
	}
	if (c != nbElems)
		return false;
	return true;
}

BatchElem * r2::Batch::alloc(Tile * _tile, double _priority) {
	BatchElem * elem = rd::Pools::elems.alloc(); 
	if (_tile != nullptr) {
		elem->setTile(_tile);
		elem->ownsTile = false;
	}
	elem->setPriority(_priority);
	add(elem);
	return elem;
}

void r2::Batch::free(BatchElem * e) {
	remove(e);
	e->destroy();
}

void r2::Batch::removeAllElements(){
	BatchElem * cur = head;
	head = nullptr;
	nbElems = 0;
}

void r2::Batch::deleteAllElements() {
	BatchElem * cur = head;
	while (cur) {
		BatchElem * n = cur->next;
		delete cur;
		cur = n;
	}
	head = nullptr;
	nbElems = 0;
}

void r2::Batch::poolBackAllElements() {
	BatchElem * cur = head;
	while (cur) {
		BatchElem * n = cur->next;
		
		cur->dispose();
		rd::Pools::free(cur);

		cur = n;
	}
	head = nullptr;
	nbElems = 0;
}

void r2::Batch::destroyAllElements() {
	BatchElem* cur = head;
	while (cur) {
		BatchElem* n = cur->next;

		cur->destroy();

		cur = n;
	}
	head = nullptr;
	nbElems = 0;
}

void r2::Batch::traverseElements(std::function<void(BatchElem*)> visit) {
	BatchElem * cur = head;
	while (cur) {
		visit(cur);
		cur = cur->next;
	}
}

void r2::Batch::disposeAllElements() {
	BatchElem * cur = head;
	while (cur) {
		BatchElem * n = cur->next;

		cur->dispose();

		cur = n;
	}
	head = nullptr;
	nbElems = 0;
}

Bounds r2::Batch::getMyLocalBounds() {
	Bounds b;
	b.empty();
	syncAllMatrix();

	Pasta::Matrix44 local = getLocalMatrix();
	Pasta::Matrix44 eLocal;

	for (auto it = head; it != nullptr; it = it->next) {
		BatchElem * e = it;

		if (!e->getTile()) continue;
		if (!e->getTexture()) continue;

		Tile * t = e->getTile();

		eLocal = Matrix44::identity;
		eLocal.setScale(e->scaleX, e->scaleY, 1.0f);
		eLocal.rotateZ(PASTA_RAD2DEG(e->rotation));
		eLocal.translate(Pasta::Vector3(e->x, e->y, 0.0));

		eLocal = local * eLocal;

		Pasta::Vector3 topLeft		= eLocal * Pasta::Vector3(t->dx,			t->dy,				0.0);
		Pasta::Vector3 topRight		= eLocal * Pasta::Vector3(t->dx + t->width, t->dy,				0.0);
		Pasta::Vector3 bottomLeft	= eLocal * Pasta::Vector3(t->dx,			t->dy + t->height,	0.0);
		Pasta::Vector3 bottomRight	= eLocal * Pasta::Vector3(t->dx + t->width, t->dy + t->height,	0.0);

		b.addPoint(topLeft.x, topLeft.y);
		b.addPoint(topRight.x, topRight.y);
		b.addPoint(bottomLeft.x, bottomLeft.y);
		b.addPoint(bottomRight.x, bottomRight.y);
	}
	
	return b;
}

r2::BatchElem* r2::Batch::firstElem() {
	return head;
}

r2::BatchElem* r2::Batch::getByUID(r::uid _uid) {
	auto el = head;
	while(el){
		if (el->uid == _uid)
			return el;
		el = el->next;
	}
	return nullptr;
}

Node * Batch::clone(Node * n) {
	if (!n) n = new Batch();

	SUPER::clone(n);

	Batch * s = dynamic_cast<Batch*>(n);
	if (s) {//we may sometime want to transfer only parent!
		int nbElements = nbElems;
		BatchElem * cur = head;
		while (cur) {
			BatchElem * c = cur->clone();
			s->add(c);
			cur = cur->next;
		}
	}
	return n;
}

void r2::Batch::findByUID(r::u64 uid, r2::Node *& node, r2::BatchElem *& elem) {
	SUPER::findByUID(uid, node, elem);
	if (node || elem) return;
	
	BatchElem * cur = head;
	while (cur) {
		if (cur->uid == uid) {
			elem = cur;
			return;
		}
		cur = cur->next;
	}
}

Batch* r2::Batch::fromPool(Node* parent) {
	auto a = rd::Pools::batches.alloc();
	if (parent) parent->addChild(a);
	return a;
}

void r2::Batch::unitTest() {

	{
		Batch * b = new Batch();
		BatchElem * e = b->alloc();
		int i = 0;
		delete b;
	}

	{
		Batch * b = new Batch();
		BatchElem * e0 = b->alloc(nullptr);
		BatchElem * e1 = b->alloc(nullptr, -1);
		int i = 0;
		assert(b);
		assert(b->head == e0);

		assert(e0->next == e1);
		assert(e0->prev == nullptr);
		assert(e1->prev == e0);

		assert(b->sanityCheck());
		delete b;
	}

	{
		Batch * b = new Batch();
		BatchElem * e0 = b->alloc(nullptr);
		BatchElem * e1 = b->alloc(nullptr, 1);
		int i = 0;
		assert(b);
		assert(b->head == e1);

		assert(e0->next == nullptr);
		assert(e0->prev == e1);
		assert(e1->next == e0);

		assert(b->getElement(0) == e1);
		assert(b->getElement(1) == e0);

		assert(b->sanityCheck());
		delete b;
	}

	{
		Batch * b = new Batch();
		BatchElem * e0 = b->alloc(nullptr);
		BatchElem * e1 = b->alloc(nullptr, 1);
		BatchElem * e2 = b->alloc(nullptr, 2);
		assert(b);
		assert(b->head == e2);

		assert(b->getElement(0) == e2);
		assert(b->getElement(1) == e1);
		assert(b->getElement(2) == e0);

		assert(b->sanityCheck());
		b->poolBackAllElements();
		assert(b->getNbElements() == 0);
		assert(b->sanityCheck());
		delete b;
	}

	{
		Batch * b = new Batch();
		BatchElem * e0 = b->alloc(nullptr);
		BatchElem * e1 = b->alloc(nullptr, -1);
		BatchElem * e2 = b->alloc(nullptr, -2);
		assert(b);
		assert(b->head == e0);

		assert(b->getElement(0) == e0);
		assert(b->getElement(1) == e1);
		assert(b->getElement(2) == e2);

		assert(b->sanityCheck());
		b->poolBackAllElements();
		assert(b->getNbElements() == 0);
		assert(b->sanityCheck());
		delete b;
	}

	{
		Batch * b = new Batch();
		BatchElem * e0 = b->alloc(nullptr, 0);
		BatchElem * e1 = b->alloc(nullptr, -1);
		BatchElem * e2 = b->alloc(nullptr, 1);
		BatchElem * e3 = b->alloc(nullptr, 2);

		assert(b);

		assert(b->getElement(0) == e3);
		assert(b->getElement(1) == e2);
		assert(b->getElement(2) == e0);
		assert(b->getElement(3) == e1);

		assert(b->getElement(4) == nullptr);

		assert(b->sanityCheck());

		b->poolBackAllElements();
		assert(b->getNbElements() == 0);
		assert(b->sanityCheck());
		delete b;
	}

	{
		Batch * b = new Batch();
		BatchElem * e0 = b->alloc(nullptr, 0);
		BatchElem * e1 = b->alloc(nullptr, -1);
		BatchElem * e2 = b->alloc(nullptr, 1);
		BatchElem * e3 = b->alloc(nullptr, 2);

		assert(b);

		b->free(e1);
		assert(b->getElement(0) == e3);
		assert(b->getElement(1) == e2);
		assert(b->getElement(2) == e0);

		assert(b->sanityCheck());
		delete b;
	}

	{
		Batch * b = new Batch();
		BatchElem * e0 = b->alloc(nullptr, 0);
		BatchElem * e1 = b->alloc(nullptr, -1);
		BatchElem * e2 = b->alloc(nullptr, 1);
		BatchElem * e3 = b->alloc(nullptr, 2);

		assert(b);

		b->free(e3);
		assert(b->getElement(0) == e2);
		assert(b->getElement(1) == e0);
		assert(b->getElement(2) == e1);

		assert(b->sanityCheck());
		delete b;
	}

	{
		Batch * b = new Batch();
		BatchElem * e0 = b->alloc(nullptr, 0);
		BatchElem * e1 = b->alloc(nullptr, -1);
		BatchElem * e2 = b->alloc(nullptr, 1);
		BatchElem * e3 = b->alloc(nullptr, 2);

		assert(b);

		b->remove(e3);
		b->add(e3);

		assert(b->getElement(0) == e3);
		assert(b->getElement(1) == e2);
		assert(b->getElement(2) == e0);
		assert(b->getElement(3) == e1);

		assert(b->sanityCheck());
		delete b;
	}

	{
		Batch * b = new Batch();
		BatchElem * e0 = b->alloc(nullptr, 4);
		BatchElem * e1 = b->alloc(nullptr, 3);
		BatchElem * e2 = b->alloc(nullptr, 2);
		BatchElem * e3 = b->alloc(nullptr, 1);
		BatchElem * e4 = b->alloc(nullptr, 0);

		assert(b);

		assert(b->getElement(0) == e0);
		assert(b->getElement(1) == e1);
		assert(b->getElement(2) == e2);
		assert(b->getElement(3) == e3);
		assert(b->getElement(4) == e4);

		assert(b->sanityCheck());

		b->changePriority(e2, 5);

		assert(b->getElement(0) == e2);
		assert(b->getElement(1) == e0);
		assert(b->getElement(2) == e1);
		assert(b->getElement(3) == e3);
		assert(b->getElement(4) == e4);

		assert(b->sanityCheck());
		delete b;
	}

	{
		Batch * b = new Batch();
		BatchElem * e0 = b->alloc(nullptr, 4);
		BatchElem * e1 = b->alloc(nullptr, 3);
		BatchElem * e2 = b->alloc(nullptr, 2);
		BatchElem * e3 = b->alloc(nullptr, 1);
		BatchElem * e4 = b->alloc(nullptr, 0);

		assert(b);

		assert(b->getElement(0) == e0);
		assert(b->getElement(1) == e1);
		assert(b->getElement(2) == e2);
		assert(b->getElement(3) == e3);
		assert(b->getElement(4) == e4);

		assert(b->sanityCheck());

		b->changePriority(e2, 3.5);

		assert(b->getElement(0) == e0);
		assert(b->getElement(1) == e2);
		assert(b->getElement(2) == e1);
		assert(b->getElement(3) == e3);
		assert(b->getElement(4) == e4);

		assert(b->sanityCheck());
		delete b;
	}

	{
		Batch * b = new Batch();
		BatchElem * e0 = b->alloc(nullptr, 4);
		BatchElem * e1 = b->alloc(nullptr, 3);
		BatchElem * e2 = b->alloc(nullptr, 2);
		BatchElem * e3 = b->alloc(nullptr, 1);
		BatchElem * e4 = b->alloc(nullptr, 0);

		assert(b);

		assert(b->getElement(0) == e0);
		assert(b->getElement(1) == e1);
		assert(b->getElement(2) == e2);
		assert(b->getElement(3) == e3);
		assert(b->getElement(4) == e4);

		assert(b->sanityCheck());

		b->changePriority(e2, 0.5);

		assert(b->getElement(0) == e0);
		assert(b->getElement(1) == e1);
		assert(b->getElement(2) == e3);
		assert(b->getElement(3) == e2);
		assert(b->getElement(4) == e4);

		assert(b->sanityCheck());
		delete b;
	}

	{
		Batch * b = new Batch();
		BatchElem * e0 = b->alloc(nullptr, 4);
		BatchElem * e1 = b->alloc(nullptr, 3);
		BatchElem * e2 = b->alloc(nullptr, 2);
		BatchElem * e3 = b->alloc(nullptr, 1);
		BatchElem * e4 = b->alloc(nullptr, 0);

		assert(b);

		assert(b->getElement(0) == e0);
		assert(b->getElement(1) == e1);
		assert(b->getElement(2) == e2);
		assert(b->getElement(3) == e3);
		assert(b->getElement(4) == e4);

		assert(b->sanityCheck());

		b->changePriority(e2, -0.5);

		assert(b->getElement(0) == e0);
		assert(b->getElement(1) == e1);
		assert(b->getElement(2) == e3);
		assert(b->getElement(3) == e4);
		assert(b->getElement(4) == e2);

		assert(b->sanityCheck());
		delete b;
	}

	{
		Batch * b = new Batch();
		BatchElem * e0 = b->alloc(nullptr, 4);
		BatchElem * e1 = b->alloc(nullptr, 3);
		BatchElem * e2 = b->alloc(nullptr, 2);
		BatchElem * e3 = b->alloc(nullptr, 1);
		BatchElem * e4 = b->alloc(nullptr, 0);

		assert(b);

		assert(b->getElement(0) == e0);
		assert(b->getElement(1) == e1);
		assert(b->getElement(2) == e2);
		assert(b->getElement(3) == e3);
		assert(b->getElement(4) == e4);

		assert(b->sanityCheck());

		b->changePriority(e2, 3);

		assert(b->getElement(0) == e0);
		assert(b->getElement(1) == e2);
		assert(b->getElement(2) == e1);
		assert(b->getElement(3) == e3);
		assert(b->getElement(4) == e4);

		assert(b->sanityCheck());
		delete b;
	}

	int i = 0;

}


#undef SUPER