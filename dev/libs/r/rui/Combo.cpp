#include "stdafx.h"

#include "rui/Combo.hpp"
#include "rui/DropDown.hpp"

rui::Combo::Combo(Conf cnf, r2::Node* n) : Super(n) {
	conf = cnf;

	if (!conf.style) {
		style("background-color", r::Color::Black.mulAlpha(0.95f));
		style.setFontSize(26);
		conf.style = &style;
	}

	Str lbl;

	if( conf.activeId.empty()){
		if (!conf.content.empty()) {
			if (conf.getUIName)
				lbl = conf.getUIName(conf.content[0].c_str());
			else
				lbl = conf.content[0].c_str();
		}
		else 
			lbl =  name;
	}
	else {
		if (conf.getUIName)
			lbl = conf.getUIName(conf.activeId.c_str());
	}

	bt = new rui::Button( lbl.c_str(), conf.style->getFontFamily(), this);
	if(conf.style ) 
		bt->applyStyle( *conf.style );

	auto bnd = bt->getBounds();
	bt->x += -bnd.left();
	bt->y += -bnd.top();

	bt->sigOnClick.add([=]() {
		onDropDownCreation();
		dd = new rui::DropDown( this );

		if(conf.style)
			dd->style = *conf.style;

		if (!dd->style.has("color:focused"))
			dd->style("color:focused", r::Color::Pink);
		if (!dd->style.has("color:active"))
			dd->style("color:active", r::Color::White);

		dd->getUIName = conf.getUIName;
		dd->setContent( conf.content );
		dd->onAccepted.add([=](const char* l) {
			auto ph = bt;
			rui::Button* phBt = dynamic_cast<rui::Button*>(ph);
			if (phBt) {
				Str lbl = conf.getUIName ? conf.getUIName(l) : l;//todo finish this
				phBt->setLabel(lbl.c_str());
				phBt->show();
			}
			rd::Garbage::trash(dd);
		});
		dd->onRejected.add([=]() {
			int here = 0;
		});
		dd->useNewNavigation = true;
		dd->generateNavigation();
		dd->startNavigation();
		dd->onDispose.add([=]() {
			dd = 0;
		});
		afterDropDownCreation();
	});
}

rui::Combo::~Combo(){
}

void rui::Combo::activate() {
	if (isActive())
		return;
	bt->doClick();
}

bool rui::Combo::isActive(){
	return 0 != dd;
}

void rui::Combo::im() {
	style.im();
	Super::im();
}