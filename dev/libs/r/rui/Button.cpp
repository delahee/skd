#include "stdafx.h"

#include "r2/Text.hpp"
#include "rui/Button.hpp"

#include "rd/JSerialize.hpp"
#include "rd/Style.hpp"


rui::Button::Button(const char* _label, rd::Font * fnt,r2::Node * _parent ) : r2::Node(_parent) {
	label = r2::Text::fromPool(fnt, _label, activeColor,this);
	name = std::string("bt:") + _label;
	label->setTextColor(activeColor);
	label->setBlockAlign(r2::Text::ALIGN_VCENTER);
	label->x = 2;
	label->y += label->height() * 0.5f;

	r2::Bounds labelBnd = label->getBounds(this);
	this->inter = new r2::Interact(labelBnd.right(), labelBnd.bottom(),this);

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

void rui::Button::setupInter() {
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

void rui::Button::setupSig(){
	//sigOnFocus.add([this]() { });

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

	//sigOnClick.add([this]() { });
	//sigOnDispose.add([this]() { });
}

void rui::Button::setFontSize(int sz){
	label->setFontSize(sz);
	pack();
}

void rui::Button::im() {
	using namespace ImGui;
	bool chg = false;
	chg = DragInt("interact Padding", &interactPadding, 1);
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
	if(ImGui::Button("Repack interact zone"))
		pack();
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
		SameLine();

		if(!isGreyed() && ImGui::Button("Set to grey"))
			setGreyed(true);
		if (isGreyed() && ImGui::Button("Ungrey"))
			setGreyed(false);
		TreePop();
		NewLine();
	}
	r2::Im::nodeButton(bg); SameLine();
	r2::Im::nodeButton(label); SameLine();
	r2::Im::nodeButton(inter);
	chg |= ColorEdit3("activeColor", activeColor);
	chg |= ColorEdit3("focusedColor", focusedColor);
	chg |= ColorEdit3("greyedColor", greyedColor);
	if( TreeNode("Text##ruibutton")){
		label->textIm();
		TreePop();
	}
	Super::im();
	if(chg) 
		pack();
}

void rui::Button::setFont(rd::Font * fnt){
	label->setFont(fnt);
}

void rui::Button::pack(){
	if (!label) return;
	if (filter)
		filter->invalidate();
	r2::Bounds labelBnd = label->getBounds(this);
	inter->x = std::rint(labelBnd.left() - interactPadding);
	inter->y = std::rint(labelBnd.top() - interactPadding);
	inter->rectWidth = std::rint(labelBnd.width() + interactPadding*2);
	inter->rectHeight = std::rint(labelBnd.height() + interactPadding*2);
	if (greyed)
		label->setTextColor(greyedColor);
	else if (focused)
		label->setTextColor(focusedColor);
	else 
		label->setTextColor(activeColor);

	r2::Graphics* lbg = dynamic_cast<r2::Graphics*>(bg);
	if (lbg) {
		lbg->clear();
		lbg->setColor(bgColor);
		lbg->drawRect(inter->x, inter->y, rint(inter->width() - interactPadding * 0.5), rint(inter->height()) - interactPadding);
	}
	onPack();
}

rui::Button::~Button() {
#if 0
	trace(Str256f("~bt name:%s id:%ld ptr:0x%x", name.c_str(), uid, this));
#endif
	al.dispose();

	if (label) {
		label->destroy();
		label = 0;
	}
	if (inter) {
		inter->destroy();
		inter = 0;
	}

	dispose();
}

void rui::Button::dispose() {
	//r2::Node::dispose();

	sigOnEnter.clear();
	sigOnOut.clear();
	sigOnFocus.clear();
	sigOnBlur.clear();
	sigOnDispose.clear();
	sigOnClick.clear();

	if (label)
		label->dispose();
	if (inter) 
		inter->dispose();
}

bool rui::Button::isEnabled() {
	return enabled;
}

void rui::Button::setEnabled(bool _onOff) {
	inter->enabled = enabled = _onOff;
}

void rui::Button::setGreyed(bool onoff){
	greyed = onoff;
	if (greyed)
		label->setTextColor(greyedColor);
	else 
		label->setTextColor(activeColor);
	sigOnGreyed();
}

bool rui::Button::isVisible(){
	return visible;
}

void rui::Button::setVisible(bool _onOff){
	visible = _onOff;
}

bool rui::Button::isGreyed() const{
	return greyed;
}

bool rui::Button::hasParent(){
	return parent;
}

bool rui::Button::canInteract(){
	return enabled;
}

void rui::Button::onMouseEnter() {
	sigOnEnter.trigger();
}

void rui::Button::onMouseOut(){
	sigOnOut.trigger();
}

void rui::Button::onMouseOver() {
	sigOnOver.trigger();
}

void rui::Button::onFocus(){
	sigOnFocus.trigger();
}

void rui::Button::onFocusLost(){
	sigOnBlur.trigger();
}

void rui::Button::doClick(){
	sigOnClick.trigger();
}

Vector2 rui::Button::getPosition(){
	return Vector2(x,y);
}

r::Vector2 rui::Button::getAbsolutePosition() { 
	return r2::Node::getBounds(getScene()).getTopLeft();
}

Vector2 rui::Button::getSize(){
	return Vector2(width(),height());
}

void rui::Button::update(double dt){
	r2::Node::update(dt);
	al.update(dt);
}

void rui::Button::setActiveColor(int rgb, float alpha){
	activeColor.load(rgb, alpha);
	setActiveColor(activeColor);
}

void rui::Button::setGreyedColor(int rgb, float alpha){
	greyedColor.load(rgb, alpha);
	setGreyedColor(greyedColor);
}

void rui::Button::setLabel(const char* lbl){
	r2::Text* txt = getText();
	if (!txt)return;
	txt->setText(lbl);
}

void rui::Button::setActiveColor(r::Color col){
	activeColor = col;
	if (!greyed)
		label->setTextColor(activeColor);
}

void rui::Button::setFocusedColor(r::Color col){
	focusedColor = col;
	if (focused && ! greyed)
		label->setTextColor(focusedColor);
}

void rui::Button::updateColor(){
	if(greyed)
		label->setTextColor(greyedColor);
	else if( focused )
		label->setTextColor(focusedColor);
	else 
		label->setTextColor(activeColor);
}

void rui::Button::setGreyedColor(r::Color col){
	greyedColor = col;
	if (greyed)
		label->setTextColor(greyedColor);
}

void rui::Button::setDebug(bool onOff){
	//r2::Graphics::fromBounds(label->getLocalBounds(), rd::ColorLib::get(0xff007f, 0.5), this);
	r2::Graphics::fromBounds(inter->getLocalBounds(), rd::ColorLib::get(0xff007f, 0.5), this);
}

bool rui::Button::isHovered(){
	return hovered;
}

void rui::Button::setHovered(bool _onOff){
	hovered = _onOff;
	sigOnEnter.trigger();
}

bool rui::Button::isFocused(){
	return focused;
}

void rui::Button::setFocused(bool _onOff){
	focused = _onOff;
	if (_onOff) {
		inter->focus(true);
		label->setTextColor(focusedColor);
	}
	else {
		//inter->blur(true);
		if(!greyed)
			label->setTextColor(activeColor);
	}
}

void rui::Button::setBackgroundColor(const r::Color &col){
	if (bg)
		bg->destroy();
	bg = r2::Graphics::fromPool(this);
	bgColor = col;
	bg->toBack();
	pack();
}

void rui::Button::serialize(Pasta::JReflect& f, const char* name){
	if (f.isReadMode()){
		destroyAllChildren();
		label = 0;
		inter = 0;
	}

	Super::serialize(f, name);

	f.visit(interactPadding, "interactPadding");
	f.visit(hovered, "hovered");
	f.visit(focused, "focused");
	f.visit(greyed, "greyed");
	f.visit(enabled, "enabled ");

	f.visit(activeColor, "activeColor");
	f.visit(focusedColor, "focusedColor");
	f.visit(greyedColor, "greyedColor");

	r::uid bgUid = bg ? bg->uid : 0;
	f.visit(bgUid, "bgUid");
	if (f.isReadMode()) bg = (r2::Node*)findByUID(bgUid);
	
	r::uid interUid = inter ? inter->uid : 0;
	f.visit(interUid, "interUid");
	if (f.isReadMode()) inter = (r2::Interact*)findByUID(interUid);

	r::uid labelUid = label ? label->uid : 0;
	f.visit(labelUid, "labelUid");
	if (f.isReadMode()) {
		auto nlabel = (r2::Text*)findByUID(labelUid);
		if( nlabel != label ){
			if(label) label->destroy();
			label = nlabel;
		} 
		setupInter();
		setupSig();
	}

	if (label) label->tryTranslate();
	pack();
}

void rui::Button::applyStyle(const rd::Style& st){
	if(label) 
		label->applyStyle(st);
	
	if (st.has("background-color"))
		setBackgroundColor(st.getColor("background-color"));

	if (st.has("color:active"))
		activeColor = st.getColor("color:active");
	if (st.has("color:greyed"))
		greyedColor = st.getColor("color:greyed");
	if (st.has("color:focused"))
		focusedColor = st.getColor("color:focused");

	pack(); 
}
