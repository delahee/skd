#include "stdafx.h"
#include "ri18n/T.hpp"
#include "ri18n/RichText.hpp"
#include "DropDown.hpp"

using namespace std::literals;
rui::DropDown::DropDown(r2::Node* parent) : Super( "dropdown",parent) {
	result = new rd::Promise();
	sigExit.add([this]() {
		if (result && !result->isSettled()) {
			yieldHierarchy();
			reject();
		}
	});

	onDispose.addOnce([this]() {
		if (result && !result->isSettled())
			reject();
	});
	
	flags |= ALLOW_ESC;

	sigBack.addOnce([this]() {
		if (!result->isSettled()) 
			reject();
	});

	style("background-color", r::Color::Black);
	style("color", r::Color::White);
	style("color:active", r::Color::White);
	style("color:focused", r::Color::Purple);
	style("vertical-spacing", 16);
	style("font-size", 26);
}

void rui::DropDown::dispose(){
	Super::dispose();
}

void rui::DropDown::simBack() {//do not destroy the dropdown, leave it to caller
	auto pm = getParentMenu();
	yieldHierarchy();
	sigBack();
}

rui::DropDown::~DropDown(){
	if (result)
		delete result;
	result = 0;
	dispose();
}

void rui::DropDown::im() {
	style.im();
	Super::im();
}

r2::Node* rui::DropDown::feedHelp(const char* lbl) {
	auto hfr = findByName("help_frame");
	if (hfr) hfr->destroy();
	 
	frame->syncAllMatrix();
	auto w = int(frame->width())-6;

	if (lbl && *lbl) {
		auto n = r2::Node::fromPool(this);
		n->name = "help_frame";
		auto txt = ri18n::RichText::fromPool(style.getFontFamily(), lbl, style.getColor("color") , n);
		txt->setMaxLineWidth(300);
		auto b = txt->getBounds();
		auto gfx = r2::Graphics::fromPool(0);
		gfx->setGeomColor(style.getColor("background-color"));
		gfx->drawRect(0, 0, b.width(), b.height());
		n->addChild(gfx);
		n->addChild(txt);
		n->x = w;
		n->trsDirty = true;
		n->syncAllMatrix();
		return n;
	}

	//when there is no help to display, display nothing
	return 0;
}

rui::Button* rui::DropDown::getButton(const char* entryLabel,const char* buttonLabel){
	auto bt = new rui::Button(ri18n::T::has(buttonLabel) ? TT(buttonLabel) : buttonLabel, 0, 0);
	if (enableHelpPanel) {
		bt->sigOnFocus.add([this,bt]() {
			if (helpReplacer) {
				const char* label = bt->vars.getString("label");
				auto h = feedHelp( helpReplacer(StrRef(label)).c_str() );
				if (h) {
					h->y = bt->y;
					h->trsDirty = true;
					h->syncAllMatrix();
				}
			}
			else
				traceWarning("No help panel content resolver set, not feeding the help pannel");
		});
	}
	return bt;
}


void rui::DropDown::setContent(const eastl::vector<Str>& _labels) {
	labels = _labels;
	build();
}

void rui::DropDown::setContent(const std::initializer_list<const char *>& _labels) {
	for (auto l : _labels)
		labels.push_back(Str(l));
	build();
}

void rui::DropDown::makeTestButtonInteractor() {
	for (auto n : frame->children) {
		auto bt = dynamic_cast<rui::Button*>(n);
		if (bt) {
			bt->sigOnFocus.add([=]() {
				bt->setActiveColor(r::Color::Magenta);
			});
			bt->sigOnBlur.add([=]() {
				bt->setActiveColor(r::Color::White);
			});
		}
	}
}

void rui::DropDown::makeDefaultButtonInteractor() {
	for (auto n : frame->children) {
		auto bt = dynamic_cast<rui::Button*>(n);
		if (bt) {

			bt->sigOnClick.add([=]() {
				if (!acceptsInput())
					return;
				yieldHierarchy();
				accept(bt->vars.getStr("label").c_str());
			});
		}
	}
}

void rui::DropDown::update(double dt) {
	Super::update(dt);
}

void rui::DropDown::build(){
	destroyAllChildren();

	root = r2::Node::fromPool(this);
	bg = r2::Graphics::fromPool(root);
	bg->color = style.getColor("background-color");
	bg->drawRect(0, 0, 4, 4);
	frame = new r2::Flow(root);
	frame->vertical();
	frame->verticalSpacing = style.getInt("vertical-spacing");

	int idx = 0;
	for (auto& b : labels) {
		Str n = b.c_str();
		if (getUIName)
			n = getUIName(b.c_str());
		rui::Button* bt = getButton( b.c_str(),n.c_str() );
		bt->applyStyle(style);
		if (bt) {
			frame->addChild(bt);
			addButton(bt);
		}
		bt->vars.set("idx", idx);
		bt->vars.set("label", b.c_str());
		idx++;
	}
	frame->reflow();
	auto b = frame->getBounds();
	bg->setPosVec2( b.getTopLeft() );
	bg->setSize( b.width(), b.height() );
	bg->toBack();

	root->setPos( -b.left(), -b.top() );

	makeDefaultButtonInteractor();
	navDirty = true;
	startNavigation(0);
}

void rui::DropDown::accept(const char*lbl) {
	trace("dd accepted"s + lbl);
	selected = lbl;
	onAccepted(selected.c_str());
	result->accept(lbl);
}

void rui::DropDown::reject(){
	trace("dd rejected");
	onRejected();
	result->reject(0);
}
