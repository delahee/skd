#include "stdafx.h"

#include "rui/Button.hpp"

rui::Button::Button(const char* label, rd::Font * fnt,r2::Node * _parent ) : r2::Node(_parent) {
	this->label = new r2::Text(fnt, label, this);

	name = std::string("bt:") + label;

	this->label->setTextColor(activeColor);

	r2::Bounds labelBnd = this->label->getBounds(this);
	this->inter = new r2::Interact(labelBnd.right(), labelBnd.bottom(),this);
	
	al = new AgentList();

	inter->onFocuses.push_back([this](rs::InputEvent & ev) {
		sigOnFocus.trigger();
	});

	inter->onFocusLosts.push_back([this](rs::InputEvent & ev) {
		sigOnBlur.trigger();
	});

	inter->onMouseEnters.push_back([this](rs::InputEvent & ev) {
		sigOnEnter.trigger();
	});

	inter->onMouseOuts.push_back([this](rs::InputEvent & ev) {
		sigOnOut.trigger();
	});

	inter->onKeyDowns.push_back([this](rs::InputEvent & ev) {
		r2::Scene * scene = getScene();
		if (scene->getCurrentFocus() == inter && focused) {
			if (ev.native == Pasta::PAD_CROSS ) 
				sigOnClick.trigger();
		}
	});

	inter->onMouseButtonUps.push_back([this](rs::InputEvent & ev) {
		sigOnClick.trigger();
	});

	sigOnFocus.add([this]() {
	});
	
	sigOnBlur.add([this]() {
	});

	sigOnEnter.add([this]() {
		hovered = true;
	});

	sigOnOut.add([this]() {
		hovered = false;
	});

	sigOnClick.add([this]() {
	});

	sigOnDispose.add([this]() {
	});

	auto pAsMenu = dynamic_cast<rui::Menu*>(parent);
	if(pAsMenu) 
		pAsMenu->buttons.push_back(this);
}

void rui::Button::pack(){
	if (filter)
		filter->invalidate();
	r2::Bounds labelBnd = this->label->getBounds(this);
	inter->rectWidth = labelBnd.right();
	inter->rectHeight = labelBnd.bottom();
}

rui::Button::~Button() {

	if (label)
		delete label;
	if (inter)
		delete inter;
	if (al)
		delete al;
}

void rui::Button::dispose() {
	//r2::Node::dispose();

	sigOnEnter.clear();
	sigOnOut.clear();
	sigOnFocus.clear();
	sigOnBlur.clear();
	sigOnDispose.clear();
	sigOnClick.clear();

	if (label){
		label->dispose();
		label = nullptr;
	}
	if (inter) {
		inter->dispose();
		inter = nullptr;
	}
}

bool rui::Button::isEnabled() {
	return enabled;
}

void rui::Button::setEnabled(bool _onOff) {
	inter->enabled = enabled = _onOff;
}

void rui::Button::setGreyed(bool onoff)
{
	greyed = onoff;
}

bool rui::Button::isVisible()
{
	return visible;
}

void rui::Button::setVisible(bool _onOff)
{
	visible = _onOff;
}

bool rui::Button::isGreyed()
{
	return greyed;
}

bool rui::Button::hasParent()
{
	return parent;
}

bool rui::Button::canInteract()
{
	return enabled;
}

void rui::Button::onMouseOut(rs::InputEvent &)
{
	sigOnOut.trigger();
}

void rui::Button::onMouseOver()
{
	sigOnEnter.trigger();
}

void rui::Button::onFocus()
{
	sigOnFocus.trigger();
}

void rui::Button::onFocusLost()
{
	sigOnBlur.trigger();
}

void rui::Button::doClick()
{
	sigOnClick.trigger();
}

Vector2 rui::Button::getPosition()
{
	return Vector2(x,y);
}

Vector2 rui::Button::getSize()
{
	return Vector2(width(),height());
}

void rui::Button::update(double dt)
{
	r2::Node::update(dt);
	al->update(dt);
}

void rui::Button::setActiveColor(int rgb, float alpha)
{
	activeColor.load(rgb, alpha);
	setActiveColor(activeColor);
}

void rui::Button::setGreyedColor(int rgb, float alpha)
{
	greyedColor.load(rgb, alpha);
	setGreyedColor(greyedColor);
}

void rui::Button::setActiveColor(r::Color col)
{
	activeColor = col;
	if (!greyed)
		label->setTextColor(activeColor);
}

void rui::Button::setGreyedColor(r::Color col)
{
	greyedColor = col;
	if (greyed)
		label->setTextColor(greyedColor);
}

void rui::Button::setDebug(bool onOff)
{
	//r2::Graphics::fromBounds(label->getLocalBounds(), rd::ColorLib::get(0xff007f, 0.5), this);
	r2::Graphics::fromBounds(inter->getLocalBounds(), rd::ColorLib::get(0xff007f, 0.5), this);
}

bool rui::Button::isHovered()
{
	return hovered;
}

void rui::Button::setHovered(bool _onOff)
{
	hovered = _onOff;
	sigOnEnter.trigger();
}

bool rui::Button::isFocused()
{
	return focused;
}

void rui::Button::setFocused(bool _onOff)
{
	focused = _onOff;
	inter->focus(true);
}

void rui::Button::setBackgroundColor(r::Color col){
	if (bg)
		bg->destroy();
	bg = r2::Graphics::fromPool(this);
	bg->toBack();
}