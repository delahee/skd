#include "stdafx.h"
#include "Node.hpp"
#include "Interact.hpp"
#include "Scene.hpp"

using namespace std;
using namespace r2;
using namespace rs;

Interact::Interact(r2::Node* p) : Super(nullptr){
	//Cannot really find satisfying values for interact size 
	//let's use a reasonably stupid yes observable value
	rectWidth = 16;
	rectHeight = 16;
	if (p)//c++ won't call inherited members from within SUPER constructor
		p->addChild(this);
	setName("Interact");
}

Interact::Interact(float w,float h, r2::Node * p ) : Super(nullptr), rectWidth(w), rectHeight(h){
	if (p)//c++ won't call inherited members from within SUPER constructor
		p->addChild(this);
	setName("Interact");
}

r2::Interact::~Interact(){
	//traceNode("~inter",this);
	r2::Scene* s = getScene();
	if (s) 
		s->removeEventTarget(this);
	dispose();
}

void r2::Interact::dispose(){
	//remove all data
	onMouseButtonDowns.clear();
	onMouseMoves.clear();
	onMouseOvers.clear();
	onMouseButtonUps.clear();
	onMouseOuts.clear();

	onFocuses.clear();
	onFocusLosts.clear();

	onKeyUps.clear();
	onKeyDowns.clear();

	//process
	r2::Scene * s = getScene();
	if (s) {
		if (s->getCurrentFocus() == this)
			s->setCurrentFocus(nullptr);

		if (s->getCurrentOver() == this)
			s->setCurrentOver(nullptr);
	}
	Super::dispose();
}

void r2::Interact::ensureScene() {
	auto scene = getScene();
	if (scene != assignedScene ) {
		if (assignedScene)  assignedScene->removeEventTarget(this);
		assignedScene = 0;
		if (scene) {
			scene->addEventTarget(this);
			assignedScene = scene;
		}
	}
}

void r2::Interact::onEnterHierarchy() {
	Super::onEnterHierarchy();
	auto scene = getScene();
	if (scene) {
		scene->addEventTarget(this);
		assignedScene = scene;
	}
}

void r2::Interact::onExitHierarchy() {
	Super::onExitHierarchy();
	Scene* scene = getScene();
	if (scene) 
		scene->removeEventTarget(this);
}

bool r2::Interact::hasFocus(){
	r2::Scene * sc = this->getScene();
	if (!sc) return false;
	return sc != nullptr && sc->getCurrentFocus() == this;
}

void r2::Interact::handleEvent(InputEvent & ev){
	if (!enabled)
		return;

	switch (ev.kind)
	{
		case InputEventKind::EIK_Out:			
			computeOut(ev);  break;
		case InputEventKind::EIK_Over:
			{
				if (ev.button == rs::MouseButton::BUTTON_NONE)
					isMouseDown = false;

				if (!wasInside) {
					auto evEnter = ev;
					evEnter.kind = InputEventKind::EIK_Enter;
					computeEnters(evEnter);
				}

				if (!isMouseDown)	computeOver(ev);
				else				computeMove(ev);

				wasInside = true;
				break;
			}

		case InputEventKind::EIK_Move:
			computeMove(ev); break;

		case InputEventKind::EIK_Push:			computePush(ev); break;

		case InputEventKind::EIK_Release:		computeRelease(ev);  break;

		case InputEventKind::EIK_Wheel:			computeWheel(ev);  break;

		case InputEventKind::EIK_KeyUp:
			computeKeyUp(ev); 
			break;

		case InputEventKind::EIK_KeyDown:
			computeKeyDown(ev); 
			break;

		case InputEventKind::EIK_Focus:			computeFocus(ev); break;
		case InputEventKind::EIK_FocusLost:		computeFocusLost(ev); break;

		case InputEventKind::EIK_Character:		computeChars(ev); break;

		default:
			break;
	}
}


Bounds r2::Interact::getMyLocalBounds(){
	if (boundlessInteract) 
		return Bounds().empty();
	
	auto b = Bounds();
	b.xMin = 0;
	b.xMax = rectWidth;
	b.yMin = 0;
	b.yMax = rectHeight;
	b = b.getTransformed( getLocalMatrix() );
	return b;
}

r2::Bounds Interact::getMeasures(bool forFilters) {
	if (forFilters) return Bounds().empty();

	return Super::getMeasures(false);
}

void r2::Interact::computeFocusLost(rs::InputEvent & ev){
	for (auto& e : onFocusLosts)
		e(ev);
}

void r2::Interact::computeFocus(rs::InputEvent & ev){
	for (auto& e : onFocuses)
		e(ev);
}

void r2::Interact::computePush(InputEvent & ev){
	r2::Interact * foc = nullptr;
	r2::Scene * sc = getScene();

	if (sc) foc = sc->getCurrentFocus();
	if (foc != this) sc->setCurrentFocus(this,ev);

	tmp = onMouseButtonDowns;
	for (auto& e : tmp)
		e(ev);
	tmp.clear();

	if (ev.stopPropagation != false)//if it's true or unset
		ev.stopPropagation = true;

	isMouseDown = true;
	mouseClickedDuration = 0.0f;
}

void r2::Interact::computeRelease(InputEvent & ev){
	tmp = onMouseButtonUps;
	for (auto& e : tmp)
		e(ev);
	tmp.clear();

	if( ev.stopPropagation != false)
		ev.stopPropagation = true;
	isMouseDown = false;
	mouseClickedDuration = -1;
}

//avoid weird reentrancy exceptions
static r2::EventHandlers s_tmpOnMouseMoves;
// clicked and overed
void r2::Interact::computeMove(InputEvent & ev){
	tmp = onMouseMoves;
	for (auto& e : tmp)
		e(ev);
	tmp.clear();

	if ( ev.stopPropagation != false)
		ev.stopPropagation = true;
}

void r2::Interact::computeOut(InputEvent & ev){
	tmp = onMouseOuts;
	for (auto& e : tmp)
		e(ev);
	tmp.clear();

	isMouseDown = false;
	mouseClickedDuration = -1;
	wasInside = false;
}

void r2::Interact::computeOver(InputEvent & ev){
	if (getScene()) getScene()->setCurrentOver(this);
	for (auto& e : onMouseOvers) e(ev);
	if (ev.stopPropagation != false )
		ev.stopPropagation = true;
}

// clicked and overed
void r2::Interact::computeWheel(InputEvent & ev) {
	for (auto& e : onMouseWheels)
		e(ev);
}

void r2::Interact::computeKeyUp(InputEvent & ev){
	for (int i = 0; i < onKeyUps.size(); ++i)
		onKeyUps[i](ev);
}

void r2::Interact::computeKeyDown(InputEvent & ev){
	for (int i = 0; i < onKeyDowns.size(); ++i)
		onKeyDowns[i](ev);
}

void r2::Interact::computeChars(rs::InputEvent & ev) {
	for (auto& e : onChars)
		e(ev);
}

void r2::Interact::computeEnters(rs::InputEvent & ev){
	for (auto& e : onMouseEnters) e(ev);
	if ( (ev.stopPropagation != false))
		ev.stopPropagation = true;
}

void r2::Interact::update(double dt){
	Super::update(dt);
	if(isMouseDown) mouseClickedDuration += dt;
}

void r2::Interact::blur(bool callEvent) {
	r2::Scene * sc = this->getScene();
	if (!sc) return;

	if (sc->getCurrentFocus() == this)
		sc->setCurrentFocus(nullptr);
}

void r2::Interact::focus(bool callEvents){
	r2::Scene * sc = this->getScene();
	if (!sc) return;

	if (sc->getCurrentFocus() == this) return;

	if (sc->getCurrentFocus() != this)
		sc->setCurrentFocus(this);
}

void r2::Interact::im(){
	using namespace ImGui;
	auto op = ImGuiTreeNodeFlags_DefaultOpen;
    if (TreeNodeEx("Interact",op)) {
        Checkbox("Enabled", &enabled);
        Checkbox("Boundless", &boundlessInteract);
		if (!boundlessInteract) {
			DragFloat("Width", &rectWidth);
			DragFloat("Height", &rectHeight);
		}
		if (TreeNodeEx("Sim",ImGuiTreeNodeFlags_DefaultOpen)) {
			rs::InputEvent ev(rs::InputEventKind::EIK_Simulated, 0, 0);
			if (Button("Focus"))
				for (auto& e : onFocuses) e(ev);
			if (Button("Blur"))
				for (auto& e : onFocusLosts) e(ev);
			if (Button("Enter")) 
				for (auto& e : onMouseEnters) e(ev);
			if (Button("Exit"))
				for (auto& e : onMouseOuts) e(ev);
			if (Button("Click"))
				for (auto& e : onMouseButtonDowns) e(ev);
			if (Button("Released"))
				for (auto& e : onMouseButtonUps) e(ev);
			TreePop();
		}
        TreePop();
    }
	Super::im();

}

void r2::Interact::fitToParent(){
	if (!parent) return;
	parent->syncAllMatrix();
	auto sz = parent->getSize();
	auto bnd = parent->getMyLocalBounds();
	if (bnd.isEmpty())
		bnd.zero();
	x = bnd.left() - parent->x;
	y = bnd.top() - parent->y;
	rectWidth = sz.x;
	rectHeight = sz.y;
}

Vector2 r2::Interact::getRelativeToParent(const Vector2& rel){
	if (!parent) return {};
	return Vector2(rel.x + x, rel.y + y);
}

bool r2::Interact::doesAcceptEvent(const rs::InputEvent& ev){
	if (doAcceptEventFunc)
		return doAcceptEventFunc(ev);
	return true;
}

r2::Interact* r2::Interact::fromPool(r2::Node* parent, int w, int h) {
	auto inter = rd::Pools::interacts.alloc();
	inter->rectWidth = w;
	inter->rectHeight = h;
	if (parent)
		parent->addChild(inter);
	return inter;
}