#include "stdafx.h"

#include "rui/FakeButton.hpp"
#include "rd/JSerialize.hpp"

rui::FakeButton::FakeButton(r2::Node * _parent ) : r2::Node(_parent) {
	setName("FakeButton");

	auto bnd = getBounds();
	this->inter = new r2::Interact(bnd.right(), bnd.bottom(),this);

	setupInter();
	setupSig();

	rui::Menu* pAsMenu = menu = dynamic_cast<rui::Menu*>(rui::Menu::getMenu(parent));
	if (menu) {
		menu->addButton(this);
		onDispose.addOnce([=]() {
			menu->removeButton(this);
		});
	}
}

void rui::FakeButton::setupInter() {
	if (!inter)
		return;

	inter->onFocuses.push_back([this](rs::InputEvent& ev) {
		this->ev = ev;
		sigOnFocus.trigger();
	});

	inter->onFocusLosts.push_back([this](rs::InputEvent& ev) {
		this->ev = ev;
		sigOnBlur.trigger();
	});

	inter->onMouseEnters.push_back([this](rs::InputEvent& ev) {
		this->ev = ev;
		sigOnEnter.trigger();
	});

	inter->onMouseOuts.push_back([this](rs::InputEvent& ev) {
		this->ev = ev;
		sigOnOut.trigger();
	});

	inter->onKeyDowns.push_back([this](rs::InputEvent& ev) {
		this->ev = ev;
		r2::Scene* scene = getScene();
		if (scene->getCurrentFocus() == inter && focused) {
			if (ev.native == Pasta::PAD_CROSS)
				sigOnClick.trigger();
		}
	});

	inter->onMouseButtonUps.push_back([this](rs::InputEvent& ev) {
		this->ev = ev;
		sigOnClick.trigger();
	});

	inter->onMouseButtonDowns.push_back([this](rs::InputEvent& ev) {
		this->ev = ev;
		sigOnMouseButtonDown.trigger();
	});

	inter->onMouseButtonUps.push_back([this](rs::InputEvent& ev) {
		this->ev = ev;
		sigOnMouseButtonUp.trigger();
	});
}

void rui::FakeButton::setupSig(){
	sigOnBlur.add([this]() {
		if (!greyed && focused)
			setFocused(false);
	});

	sigOnEnter.add([this]() {
		hovered = true;
	});

	sigOnOut.add([this]() {
		hovered = false;
	});
}

void rui::FakeButton::im() {
	using namespace ImGui;
	bool chg = false;
	if( chg |= Checkbox("enabled", &enabled))
		setEnabled(enabled);
	SameLine();
	if (chg |= Checkbox("hovered", &hovered))
		setHovered(hovered);
	SameLine();
	if( chg |= Checkbox("focused",&focused) )
		setFocused(focused);
	SameLine();
	if (chg |= Checkbox("greyed", &greyed))
		setGreyed(greyed);
	SameLine();
	
	if (TreeNodeEx("Sim", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::Button("Click"))
			sigOnClick();
		SameLine();
		if (ImGui::Button("Focus"))
			sigOnFocus();
		SameLine();
		if (ImGui::Button("Blur"))
			sigOnBlur();
		SameLine();
		if (ImGui::Button("Mouse Enter"))
			sigOnEnter();
		SameLine();
		if (ImGui::Button("Mouse Exit"))
			sigOnOut();
		TreePop();
		NewLine();
	}
	r2::Im::nodeButton(inter);
	Super::im();
	if(chg) 
		pack();
}

//here for learning purpose, you will probably want to do all of this by yourself with a fake
void rui::FakeButton::pack(){
	if (filter)
		filter->invalidate();
	r2::Bounds bnd = getBounds(this);
	inter->x = std::rint(bnd.left());
	inter->y = std::rint(bnd.top());
	inter->rectWidth = std::rint(bnd.width());
	inter->rectHeight = std::rint(bnd.height());
	onPack();
}

rui::FakeButton::~FakeButton() {
#ifdef _DEBUG
	trace(Str256f("~fkbt name%s id:%ld ptr0x%x", name.c_str(), uid, this));
#endif
	dispose();

	al.dispose();
	
	if (inter) {
		inter->destroy();
		inter = 0;
	}
}

void rui::FakeButton::dispose() {
	sigOnEnter.clear();
	sigOnOut.clear();
	sigOnFocus.clear();
	sigOnBlur.clear();
	sigOnDispose.clear();
	sigOnClick.clear();

	if (inter) 
		inter->dispose();
}

bool rui::FakeButton::isEnabled() {
	return enabled;
}

void rui::FakeButton::setEnabled(bool _onOff) {
	inter->enabled = enabled = _onOff;
}

void rui::FakeButton::setGreyed(bool onoff){
	greyed = onoff;
}

bool rui::FakeButton::isVisible(){
	return visible;
}

void rui::FakeButton::setVisible(bool _onOff){
	visible = _onOff;
}

bool rui::FakeButton::isGreyed() const{
	return greyed;
}

bool rui::FakeButton::hasParent(){
	return parent;
}

bool rui::FakeButton::canInteract(){
	return enabled;
}

void rui::FakeButton::onMouseEnter() {
	sigOnEnter.trigger();
}

void rui::FakeButton::onMouseOut(){
	sigOnOut.trigger();
}

void rui::FakeButton::onMouseOver() {
	sigOnOver.trigger();
}

void rui::FakeButton::onFocus(){
	sigOnFocus.trigger();
}

void rui::FakeButton::onFocusLost(){
	sigOnBlur.trigger();
}

void rui::FakeButton::doClick(){
	sigOnClick.trigger();
}

Vector2 rui::FakeButton::getPosition(){
	return Vector2(x,y);
}

r::Vector2 rui::FakeButton::getAbsolutePosition() {
	return r2::Node::getBounds(getScene()).getTopLeft();
}

Vector2 rui::FakeButton::getSize(){
	return Vector2(width(),height());
}

void rui::FakeButton::update(double dt){
	r2::Node::update(dt);
	al.update(dt);
}

void rui::FakeButton::setDebug(bool onOff){
#ifdef PASTA_FINAL
	return;
#endif

	if (onOff) {
		auto dbg = r2::Graphics::fromBounds(inter->getLocalBounds(), rd::ColorLib::get(0xff007f, 0.5), this);
		if(dbg) dbg->name = "dbg";
	}
	else {
		auto dbg = findByName("dbg");
		if (dbg) dbg->destroy();
	}
}

bool rui::FakeButton::isHovered(){
	return hovered;
}

void rui::FakeButton::setHovered(bool _onOff){
	hovered = _onOff;
	sigOnEnter.trigger();
}

bool rui::FakeButton::isFocused(){
	return focused;
}

void rui::FakeButton::setFocused(bool _onOff){
	focused = _onOff;
	if (_onOff) {
		inter->focus(true);
	}
	else {
	}
}

void rui::FakeButton::serialize(Pasta::JReflect& f, const char* name){
	if (f.isReadMode()){
		destroyAllChildren();
		inter = 0;
	}

	Super::serialize(f, name);

	f.visit(hovered, "hovered");
	f.visit(focused, "focused");
	f.visit(greyed, "greyed");
	f.visit(enabled, "enabled ");
	
	r::uid interUid = inter ? inter->uid : 0;
	f.visit(interUid, "interUid");
	if (f.isReadMode()) inter = (r2::Interact*)findByUID(interUid);

	pack();
}