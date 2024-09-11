#include "stdafx.h"
#include "rd/Agent.hpp"
#include "rd/Style.hpp"
#include "ri18n/Nar.hpp"
#include "ri18n/RichText.hpp"
#include "rui/xv/StyleSheet.hpp"
#include "View.hpp"

using namespace rui;
using namespace rui::xv;

View* rui::xv::View::fromString(const Str& _style, const Str& _view, r2::Node* par){
	return fromString(_style.c_str(),_view.c_str(),par);
}

View* rui::xv::View::fromString(const char* _style, const char* _view, r2::Node* par){
	auto v = new View(_style,_view,par);
	return v;
}

View* rui::xv::View::fromFiles(const char* _stylePath, const char* _viewPath, r2::Node* par){
	char * st = rd::RscLib::getText(_stylePath);
	char* v = rd::RscLib::getText(_viewPath);
	
	auto vRes = rui::xv::View::fromString(st, v,par);

	rd::RscLib::releaseText(st);
	rd::RscLib::releaseText(v);
	return vRes;
}

rui::xv::View::View(const char* _style, const char* _view, r2::Node* par) : View(par) {
	attachStyleSheet(_style);
	attachView(_view);
}

rui::xv::View::View(r2::Node* par) : Super(par){
}

void rui::xv::View::attachStyleSheet(const char* stylesheet) {
	lastStyleRead = stylesheet;
	auto ss = new StyleSheet();
	bool ok = ss->parse(lastStyleRead.c_str());
	if (ok) {
		styleSheet = ss;
	}
	else {
		delete ss;
	}
}

void rui::xv::View::appendStyleSheet(const char* stylesheet) {
	lastStyleRead += stylesheet;
	auto ss = new StyleSheet();
	bool ok = ss->parse(lastStyleRead.c_str());
	if (ok) 
		styleSheet->append( *ss );
	delete ss;
}

void View::generateNode(ri18n::AstNode* node) {
	using namespace ri18n;
	using ri18n::AstNodeType;
	if (!node)
		return;

	auto mkControlNodeVertical = [&]() -> r2::Flow* {
		auto fl = new r2::Flow(tip);
		fl->vertical();
		tip = fl;
		return fl;
	};

	if (node->type == AstNodeType::Seq) {
		auto fl = mkControlNodeVertical();
		if(node->a0Data) generateNode(node->a0Data);
		if(node->a1Data) generateNode(node->a1Data);
		fl->reflow();
		tip = this;
		return;
	}

	if ( node->type == AstNodeType::Tag ) {
		rd::XmlData data;
		rd::String::parseXMLTags( node->strData.c_str(), data);
		auto assignName = [&](r2::Node*n) {
			if(n)
			if (rs::Std::exists(data.attrs, "id"))
				n->name= data.attrs["id"];
		};
		auto assignClasses = [&](r2::Node* n) {
			if (n)
				if (rs::Std::exists(data.attrs, "class"))
					n->vars.set("class" , data.attrs["class"].c_str() );
		};
		if (data.tag == "stylesheet") {
			appendStyleSheet(node->strData.c_str());
		}
		if (data.tag == "node") {
			tip = r2::Node::fromPool(tip);
			assignName(tip);
			generateNode(node->a0Data);
		}
		else if (data.tag == "flow") {
			auto fl = new r2::Flow(tip);
			fl->horizontal();
			if ( rs::Std::exists(data.attrs, "horizontal"))
				fl->horizontal();
			if ( rs::Std::exists(data.attrs, "vertical"))
				fl->vertical();
			tip = fl;
			assignName(tip);
			generateNode(node->a0Data);
			fl->reflow();
		}
		tip = this;
		return;
	}
	if (node->type == AstNodeType::Sentence) {
		if(!node->strData.empty())
			ri18n::RichText::mk( node->strData.c_str(), tip);
		return;
	}

	traceError("do not understand tag?");
}

void rui::xv::View::applyStylesheet(rui::xv::StyleSheet* sheet, r2::Node* node){
	if (!sheet)
		return;

	Str& id = node->name;
	StrRef classes = node->vars.getStr("classes");

	Selector sel;
	sel.id = id;
	sel.setClasses(classes.c_str());

	eastl::vector<rui::xv::StyleSheetEntry*> rules;
	rules = sheet->infer(sel);
	for (auto r : rules) {
		r->st->apply(*node);
	}
}

void View::attachView(const char* view) {
	lastViewRead = view;
	tree = ri18n::Nar().make(view);
	tip = this;
	generateNode(tree);
	if( styleSheet )
		traverse([this](r2::Node * n) {
			applyStylesheet(styleSheet, n);
		});
}

void View::im() {
	using namespace ImGui;
	if (TreeNode("View")){
		if (TreeNode("Styles")) {
			if (styleSheet) styleSheet->im();
			TreePop();
		}
		if (TreeNode("Nodes")) {
			if (tree) tree->im();
			TreePop();
		}
		r2::Im::nodeButton(this);
		TreePop();
	}
	Super::im();
}