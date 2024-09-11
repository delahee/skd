#include "stdafx.h"

#include "1-graphics/Graphic.h"
#include "1-graphics/GraphicContext.h"
#include "1-graphics/FrameBuffer.h"
#include "1-graphics/geo_vectors.h"
#include "1-time/Profiler.h"
#include "2-application/Application.h"

#include "Scene.hpp"

#include "../rs/InputEvent.hpp"
#include "../rs/GfxContext.hpp"

using namespace std;
using namespace r2;
using namespace rs;
using namespace Pasta;

Scene::Scene( r2::Node * parent ) : Super(parent){
	clearColor = r::Color(0, 0, 0, 1);
	setDepthRange(1 << 15); // -32k; +32k
	rs::Sys::addEventListener(this);

#ifdef R_HAS_PROCESS_EXPLORER
	tools::ProcessExplorer::registry.push_back(&al);
#endif

	setName("r2::Scene");
}

Scene::~Scene(){
	dispose();

#ifdef R_HAS_PROCESS_EXPLORER
	auto & reg = tools::ProcessExplorer::registry;
	auto pos = find(reg.begin(), reg.end(), &al);
	if(pos!=reg.end() ) tools::ProcessExplorer::registry.erase(pos);
#endif
	rs::Sys::removeEventListener(this);
}

void r2::Scene::removeEventManagement() {
	rs::Sys::removeEventListener(this);
}

void r2::Scene::dispose(){
	al.dispose();//agents will be released first
	//cleanup whatever mess is in the final tree
	Super::dispose();
}

void r2::Scene::overrideDimensions(float w, float h){
	areDimensionsOverriden = true;
	sceneWidth = w;
	sceneHeight = h;
}

void Scene::stdMatrix(rs::GfxContext* ctx, int w, int h) {
	if (w == -1) w = windowWidth();
	if (h == -1) h = windowHeight();

	Matrix44 trans = Matrix44::identity;
	r::Vector3 pos = cameraPos;
	pos.x = round(pos.x * cameraScale.x) / cameraScale.x;
	pos.y = round(pos.y * cameraScale.y) / cameraScale.y;
	trans.translate(-pos);
	Matrix44 sc = Matrix44::identity;
	sc.setScale(cameraScale);
	viewMatrix = sc * trans * rotationMatrix;

	if (ctx) {
		ctx->loadViewMatrix(viewMatrix);
		ctx->loadModelMatrix(Matrix44::identity);
		ctx->loadProjMatrix(projMatrix=Matrix44::ortho(0, w, h, 0, ctx->zMin, ctx->zMax));
	}
}

Matrix44 r2::Scene::getCanonicalViewMatrix() const {
	Matrix44 trans = Matrix44::identity;
	r::Vector3 pos = cameraPos;
	pos.x = round(pos.x * cameraScale.x) / cameraScale.x;
	pos.y = round(pos.y * cameraScale.y) / cameraScale.y;
	trans.translate(-pos);
	Matrix44 sc = Matrix44::identity;
	sc.setScale(cameraScale);
	return sc * trans * rotationMatrix;
}


void Scene::render( Pasta::Graphic * g ) {
	if (!visible)
		return;

	for (RenderOp& ro : preRenderOps) ro(g);

	GraphicContext * gfxCtx = GraphicContext::GetCurrent();
	rs::GfxContext ctx(g);

	ctx.supportsEarlyDepth = false;
	ctx.push();

		for (RenderOpCtx & ro : preClearRenderOps) ro(&ctx);

		if (doClear) 
			ctx.clear(PASTA_CLEAR_COLOR | PASTA_CLEAR_DEPTH, clearColor, ctx.zMax );

		for (RenderOpCtx & ro : postClearRenderOps) ro(&ctx);
	
		{
			ctx.currentPass = rs::Pass::Basic;
			stdMatrix(&ctx);
			drawRec(&ctx);
		}

		for (RenderOpCtx & ro : postRenderOps) ro(&ctx);

	ctx.pop();
}

void Scene::checkEvents() {
	for (InputEvent e : pendingEvents) 
		handleEvent(e);
	pendingEvents.erase(pendingEvents.begin(), pendingEvents.end());
}

void Scene::syncViewMatrix() {
	Matrix44 trans = Matrix44::identity;
	trans.translate(-cameraPos);
	Matrix44 sc = Matrix44::identity;
	sc.setScale(cameraScale);
	viewMatrix = sc * trans;
}

void Scene::update(double dt) {
	Matrix44 trans = Matrix44::identity;
	trans.translate(-cameraPos);
	Matrix44 sc = Matrix44::identity;
	sc.setScale(cameraScale);
	viewMatrix = sc * trans;

	al.update(dt);
	for (UpdateOp & ro : preUpdateOps) ro(dt);
	Node::update(dt);
	for (UpdateOp & ro : postUpdateOps) ro(dt);
}

Scene* Scene::getScene() {
	return this;
}

void Scene::drawInto(rs::GfxContext * _g , r2::Node * node, Pasta::Texture * t, Pasta::FrameBuffer * _fb, rs::Pass pass) {
	Graphic * g = _g->gfx;
	GraphicContext * ctx = GraphicContext::GetCurrent();
	if (t->getFlags() & PASTA_TEXTURE_RENDER_TARGET) {
		FrameBufferAttachment fba[1];
		fba[0].m_layer = 0;
		fba[0].m_mip = 0;
		fba[0].m_texture = t;
		Pasta::FrameBuffer * fb = (_fb) ? _fb : GraphicContext::CreateFrameBuffer(fba, 1, false);
		
		_g->push();
			_g->currentPass = pass;
				_g->pushTarget(fb);
				
					stdMatrix(_g, t->getWidth(),t->getHeight());

					if (pass == Pass::Picking) _g->clear(PASTA_CLEAR_COLOR | PASTA_CLEAR_DEPTH, r::Color::Black, _g->zMax);
					else if (doClear) _g->clear(PASTA_CLEAR_COLOR | PASTA_CLEAR_DEPTH, clearColor, _g->zMax);
				
					node->drawRec(_g);

				_g->popTarget();

				if(!_fb) GraphicContext::DestroyFrameBuffer(fb);
		_g->pop();
	}
	else {
		PastaLogDebug("not a RT");
	}
}

void r2::Scene::unregisterInputDependency(r2::Scene* sc) {
	for (auto iter = deps.begin(); iter != deps.end(); ++iter) {
		if ((*iter).sc == sc) {
			deps.erase(iter);
			break;
		}
	}
}

r2::InputDep& r2::Scene::registerInputDependency(r2::Scene * sc){
	rs::Sys::removeEventListener(sc);
	deps.push_back({ sc });
	return deps.back();
}

void r2::Scene::onEvent( InputEvent & ev ) {
	pendingEvents.push_back(ev);
}

InputEvent r2::Scene::transformEvent(InputEvent& ev) {
	//mouse does not support rotated scenes
	//get it to scene space
	float sceneX = (ev.relX - x) * (width() / windowWidth()) / scaleX; // in case of scene scaling for offscreen scenes
	float sceneY = (ev.relY - y) * (height() / windowHeight()) / scaleY;

	Matrix44 iView = viewMatrix.inverse();
	Vector2 pointi(sceneX, sceneY);
	pointi = iView * pointi;

	InputEvent evSub(ev);
	evSub.relX = pointi.x;
	evSub.relY = pointi.y;
	return evSub;
}


bool r2::Scene::handleEvent(InputEvent & ev){
	if (!visible) return false;

	if (DEBUG_EVENT) {
		cout << "*******************************" <<"\n";
		cout << "*******************************" <<"\n";
		cout << "*******************************" <<"\n";
		cout << "handleEvent :" << ev.toString() << "\n";//endl generates a flush call that destroys perfs
	}

	//ev is in screen space
	InputEvent backup = ev;
	InputEvent evSub = transformEvent(ev);

	if (DEBUG_EVENT) cout << "processing deps befores " << name.cpp_str() << "\n";

	for (auto & dep : deps) {
		if (dep.before) {
			auto sc = dep.sc;
			auto & scName = sc->name;//for debug purpose
			if (evSub.kind == InputEventKind::EIK_Push)
				int retain = 0;
			bool handle = sc->handleEvent(evSub);
			if (handle && evSub.stopPropagation == true) {
				if (DEBUG_EVENT) cout << "exited in befores " << "\n";
				return handle;
			}
		}
	}
	
	if (DEBUG_EVENT) cout << "processing scene node" << "\n";

	bool handled = false;

	_tmpInteracts = interacts;
	std::reverse(_tmpInteracts.begin(), _tmpInteracts.end());

	for (auto i = _tmpInteracts.rbegin(); i != _tmpInteracts.rend(); ++i) {

		r2::Interact * it = *i;
		if (it == nullptr) //if an interactor deleted another interactor during loop
			continue;

		if (DEBUG_EVENT) cout << "inspecting " << it->name.cpp_str() << "\n";
		
		//allow all controllers to get key and pad events
		bool isGeoEvent = ev.isGeometricEvent();
		if (!isGeoEvent) {
			if (DEBUG_EVENT) cout << "a geo event hence direct transmission"<<"\n";//should add a keyboard focus probably?

			it->handleEvent(ev);
			continue;
		} else { //this is a mouse thing
			bool cancel = false;

            if (DEBUG_EVENT) cout << "evSub " << evSub.toString() << " #" << it->uid << "\n";

			if (!it->boundlessInteract) {
				Bounds b = it->getBounds(this);
				if (!b.contains(evSub.relX, evSub.relY)) {
					cancel = true;
					if (DEBUG_EVENT) cout << "out of bound" << "\n";
				}
			}

			if (!computeNodeVisibility(it)) {//not visible
				if (DEBUG_EVENT) cout << "not visible" << "\n";
				if (DEBUG_EVENT) cout << "current sprite is not visible checking for current hovered" << "\n";
				cancel = true;
			}

			if (DEBUG_EVENT) 
				cout << "we have a client,node in area of interest" << "\n";

			Vector2 r = it->globalToLocal(Vector2(evSub.relX, evSub.relY));
			ev.relX = r.x;
			ev.relY = r.y;

			if(!it->doesAcceptEvent(ev)){
				cancel = true;
				if (DEBUG_EVENT) cout << "interact decided it did not wanted to accept event" << "\n";
			}

			if (cancel) {
				if (it == getCurrentOver())//checkout if we matter
					setCurrentOver(nullptr);
				if (DEBUG_EVENT) cout << "cancelled" << "\n";
				continue;
			}

			if (DEBUG_EVENT) cout << "it handling " << ev.toString() << " #" << it->uid << "\n";

			it->handleEvent(ev);

			if (DEBUG_EVENT) cout << "ev state " << ev.toString() << " #" <<it->uid << "\n";

			handled = true;

			ev.relX = backup.relX;
			ev.relY = backup.relY;
		}
		if (handled && (ev.stopPropagation == true) )
			break;
	}

	if (ev.kind == InputEventKind::EIK_Push && !handled)
		setCurrentFocus(nullptr);

	if (DEBUG_EVENT) cout << "processing deps afters" << "\n";

	if (!handled || (ev.stopPropagation==false )) {//experimental ( aka not tested )
		for (auto& dep : deps) {
			if (!dep.before) {
				auto sc = dep.sc;
				bool handle = sc->handleEvent(evSub);
				if (handle && (evSub.stopPropagation == true) ) {
					return handle;
				}
			}
		}
	}

	return handled && !(ev.stopPropagation == false);
}

bool r2::Scene::computeNodeVisibility(r2::Node * node) {
	bool isVisible = true;
	r2::Node * p = node;
	while (p != nullptr) {
		if (!p->visible) {
			isVisible = false;
			return isVisible;
		}
		p = p->parent;
	}
	return isVisible;
}

void r2::Scene::addEventTarget(r2::Interact * inter){
	auto pos = std::find(interacts.begin(), interacts.end(), inter);
	if (pos != interacts.end()) return; // already added

	auto scene = inter->getScene();
	if (scene != this) return; // not in this scene

	int depthInter = inter->getDepthInHierarchy();
	for (int i = 0; i < interacts.size(); i++) {
		r2::Node* refInsert = inter;
		r2::Node* refCurrent = interacts[i];
		int depthInsert = depthInter;
		int depthCurrent = refCurrent->getDepthInHierarchy();
		r2::Node* parentInsert = refInsert->parent;
		r2::Node* parentCurrent = refCurrent->parent;
		// If the interact to insert is deeper in the hierarchy than the current we go up
		while (depthInsert > depthCurrent) {
			refInsert = parentInsert;
			parentInsert = parentInsert->parent;
			depthInsert--;
		}
		// Same the other way around
		while (depthCurrent > depthInsert) {
			refCurrent = parentCurrent;
			parentCurrent = parentCurrent->parent;
			depthCurrent--;
		}
		// Now the two are on the same level so as long as the parent don't match we go up until we find a match
		while (parentInsert != parentCurrent) {
			refInsert = parentInsert;
			parentInsert = parentInsert->parent;
			refCurrent = parentCurrent;
			parentCurrent = parentCurrent->parent;
		}
		// Now we have a common reference and we can just compare the 2 child indices
		if (parentInsert->getChildIndex(refInsert) > parentCurrent->getChildIndex(refCurrent)) {
			interacts.insert(interacts.begin() + i, inter);
			return;
		}
	}
	interacts.push_back(inter);
}

void r2::Scene::removeEventTarget(r2::Interact * inter){

	if (currentOver == inter) currentOver = 0;
	if (currentFocus== inter) currentFocus = 0;

	auto pos = std::find(interacts.begin(), interacts.end(), inter);
	if (pos == interacts.end()) return; // already removed

	interacts.erase(pos);

	auto tpos = std::find(_tmpInteracts.begin(), _tmpInteracts.end(), inter);
	if (tpos == _tmpInteracts.end()) return;
	*tpos = nullptr;
}

void r2::Scene::setCurrentFocus(r2::Interact * inter) { 
	bool triggerCbk = true;
	if (currentFocus && currentFocus != inter && triggerCbk) {
		InputEvent ev(InputEventKind::EIK_FocusLost, -1, -1);
		currentFocus->handleEvent(ev);
	}

	currentFocus = inter; 

	if (currentFocus && triggerCbk) {
		InputEvent ev(InputEventKind::EIK_Focus, -1, -1);
		currentFocus->handleEvent(ev);
	}
}

void r2::Scene::setCurrentFocus(r2::Interact* inter, const InputEvent& src){
	bool triggerCbk = true;
	if (currentFocus && currentFocus != inter && triggerCbk) {
		InputEvent ev(InputEventKind::EIK_FocusLost, -1, -1);
		ev.origin = src.origin;
		currentFocus->handleEvent(ev);
	}

	currentFocus = inter;

	if (currentFocus && triggerCbk) {
		InputEvent ev(InputEventKind::EIK_Focus, -1, -1);
		ev.origin = src.origin;
		currentFocus->handleEvent(ev);
	}
}

void r2::Scene::setCurrentOver(r2::Interact * inter) {
	bool triggerCbk = true;
	if (currentOver && (currentOver != inter) && triggerCbk) {
		InputEvent ev(InputEventKind::EIK_Out, -1, -1);
		currentOver->handleEvent(ev);
	}
	currentOver = inter;
}

double Scene::width() {
	if (areDimensionsOverriden)
		return sceneWidth;
	return windowWidth();
}

double Scene::height() {
	if (areDimensionsOverriden)
		return sceneHeight;
	return windowHeight();
}


#include "1-device/DeviceMgr.h"
float Scene::windowWidth() {
	Pasta::DeviceMgr * dev = Pasta::DeviceMgr::getSingleton();
	return dev->getDevicePixelWidth();
}

float Scene::windowHeight() {
	Pasta::DeviceMgr * dev = Pasta::DeviceMgr::getSingleton();
	return dev->getDevicePixelHeight();
}

float& Scene::getPanX() {
	return cameraPos.x;
}

float& Scene::getPanY() {
	return cameraPos.y;
}

void Scene::setPan(float x, float y) {
	cameraPos.x = x;
	cameraPos.y = y;
};

float& Scene::getZoomX() {
	return cameraScale.x;
}

float& Scene::getZoomY() {
	return cameraScale.y;
}

void Scene::setZoom(float xy) {
	cameraScale.x = xy;
	cameraScale.y = xy;
}

void Scene::setZoom(float x, float y) {
	cameraScale.x = x;
	cameraScale.y = y;
};

//camera depth now goes from -1/factor to 1/factor
void Scene::setDepthScale(float factor) {
	cameraScale.z = -factor;
};

float r2::Scene::getDepthRange() {
	return -1.0 / cameraScale.z;
}

void r2::Scene::setDepthRange(float range) {
	cameraScale.z = -1.0 / range;
}

void r2::EarlyDepthScene::render(Pasta::Graphic *g) {
	if (!visible) 
		return;

	for (RenderOp& ro : preRenderOps) ro(g);

	Pasta::GraphicContext * gfxCtx = Pasta::GraphicContext::GetCurrent();
	rs::GfxContext ctx(g);

	ctx.supportsEarlyDepth = true;
	ctx.push();
	
	for (RenderOpCtx & ro : preClearRenderOps) ro(&ctx);

	if (doClear)
		ctx.clear(PASTA_CLEAR_COLOR | PASTA_CLEAR_DEPTH, clearColor, ctx.zMax);

	for (RenderOpCtx & ro : postClearRenderOps) ro(&ctx);

	if (!skipClip) {
		{
			PASTA_CPU_GPU_AUTO_MARKER("EarlyDepth pass");
			ctx.currentPass = rs::Pass::EarlyDepth;
			stdMatrix(&ctx);
			drawRec(&ctx);
		}

		{
			PASTA_CPU_GPU_AUTO_MARKER("DepthEqWrite pass");
			ctx.currentPass = rs::Pass::DepthEqWrite;
			stdMatrix(&ctx);
			drawRec(&ctx);
		}
	}

	if (!skipBasic)	{
		PASTA_CPU_GPU_AUTO_MARKER("Basic pass");
		ctx.currentPass = rs::Pass::Basic;
		stdMatrix(&ctx);
		drawRec(&ctx);
	}

	for (RenderOpCtx & ro : postRenderOps) ro(&ctx);

	ctx.pop();
}

double r2::Scene::getValue(rs::TVar valType) {
	switch (valType){
	case VCamPosX:
		return cameraPos.x;
		break;
	case VCamPosY:
		return cameraPos.y;
		break;
	case VCamPosZ:
		return cameraPos.z;
		break;
	case VCamScaleX:
		return cameraScale.x;
		break;
	case VCamScaleY:
		return cameraScale.y;
		break;
	case VCamScaleZ:
		return cameraScale.z;
		break;
	default:
		return Super::getValue(valType);
		break;
	}
}

double r2::Scene::setValue(rs::TVar valType, double val) {
	switch (valType) {
	case VCamPosX:
		return cameraPos.x = val;
		break;
	case VCamPosY:
		return cameraPos.y = val;
		break;
	case VCamPosZ:
		return cameraPos.z = val;
		break;
	case VCamScaleX:
		return cameraScale.x = val;
		break;
	case VCamScaleY:
		return cameraScale.y = val;
		break;
	case VCamScaleZ:
		return cameraScale.z = val;
		break;
	default:
		return Super::setValue(valType,val);
		break;
	}
}

//void Scene::onResize(const Vector2& ns) {
//	Super::onResize(ns);
	//should apply those in user space code, not applying this can be intentionnal
	//if (areDimensionsOverriden) {
	//	sceneWidth = ns.x;
	//	sceneHeight = ns.y;
	//}
//}
