#include "stdafx.h"
#include "rs/Std.hpp"
#include "r2/BatchElem.hpp"
#include "r2/im/TilePicker.hpp"
#include "rd/JSerialize.hpp"
#include "RichText.hpp"
#include "T.hpp"
#include "Nar.hpp"

using namespace std;
using namespace r2;
using namespace rd;
using namespace ri18n;

static r2::Outline strongOutline;
static std::vector<r::opt<r2::Outline>> saveOutline;
static std::vector<r::opt<r::Color>>	saveColor;
static std::vector<bool>				saveItalic;

static void id_1(const char*) {

}

static void id_0() {

}

std::function<bool(const char*, ActionDisplayInfo&)>
RichText::getActionDisplayInfo;

RichText::RichText(rd::Font* _fnt , const char* t , r2::Node* parent) : RichText((AstNode*)nullptr, parent) {
	retainElementInfos = true;
	if (_fnt) {
		fonts.push_back(_fnt);
		fnt = _fnt;
		setFont(fnt);//sets the correct shader
	}
	setTree(t);
}

RichText::RichText(rd::Font* fnt, const std::string& t, r2::Node* parent) : RichText( fnt,t.c_str(), parent) {

}

RichText::RichText(rd::Font* fnt, const Str& t, r2::Node* parent) : RichText( fnt,t.c_str(), parent) {

}

void ri18n::RichText::updateStyle(){
	cache();
}

ri18n::RichText::RichText(AstNode* _tree, r2::Node* parent ) : Super( 0,0,parent){
	tree = _tree;
	onEmStart = [this]() {
		pushCurrentColor();
		forceColor = emColor;
	};
	onEmEnd = [this]() {
		popCurrentColor();
	};

	onStrongStart = [this]() {
		pushCurrentColor();
		forceColor = strongColor;
		pushCurrentOutline();
		if (strongOutlineColor) {
			strongOutline.col = *strongOutlineColor;
			outline = strongOutline;
		}
	};

	onStrongEnd = [this]() {
		popCurrentColor();
		popCurrentOutline();
	};
	onEvent = id_1;
	onTagStart = std::bind(&ri18n::RichText::stdTagStart, this, std::placeholders::_1, std::placeholders::_2);
	onTagEnd = std::bind(&ri18n::RichText::stdTagEnd, this, std::placeholders::_1);
	onTagFrom = id_1;

	onScript = onEvent = onCondEvent = onCondUniqueEvent =
	onUniqueEvent = onImportantEvent = id_1;

	onFirstChar = onLastChar = id_0;

#if 0
	onEmStart = []() { cout << "onEmStart " << endl; };
	onEmEnd = []() { cout << "onEmEnd "  << endl; };

	onStrongStart = []() { cout << "onStrongStart " << endl; };
	onStrongEnd = []() { cout << "onStrongEnd " << endl; };

	onEvent = [](const char* c) { cout << "onEvent " << c << endl; };
	onCondEvent = [](const char* c) { cout << "onCondEvent " << c << endl; };
	onCondUniqueEvent = [](const char* c) { cout << "onCondUniqueEvent " << c << endl; };
	onUniqueEvent = [](const char* c) { cout << "onUniqueEvent " << c << endl; };
	onImportantEvent = [](const char* c) { cout << "onImportantEvent " << c << endl; };

	onTagStart = [](const char* c,auto node) { cout << "onTagStart " << c << endl; };
	onTagEnd = [](const char* c) { cout << "onTagEnd " << c << endl; };

	onTagFrom = [](const char* c) { cout << "onTagFrom " << c << endl; };

	onFirstChar = []() { cout << "onFirstChar " << endl; };
	onLastChar = []() { cout << "onLastChar " << endl; };
	onScript = [](const char* c) { cout << "onScript " << c << endl; };
#endif

#ifdef _DEBUG
	name = "Rich Text#" + to_string(uid);
#endif

	cache();
}

RichText::~RichText() {
	dispose();
}


void ri18n::RichText::stdTagStart(const char* t, AstNode* node) {
	rd::XmlData data;
	rd::String::parseXMLTags(t, data);
	rd::Font* cur = fonts[fonts.size() - 1];

	//WIP
	if (rs::Std::exists(data.attrs, "class"))
		classes.push_back(data.attrs["class"]);

	if (data.tag == "em") onEmStart();
	if (data.tag == "strong") onStrongStart();

	if (	data.tag == "action"
		||	data.tag == "actionPrompt") {

		if (rs::Std::exists(data.attrs, "id") && getActionDisplayInfo){
			ActionDisplayInfo info;
			info.fontSize = textSize.empty() ? cur->getSize() : textSize.back();
			if( rs::Std::exists(data.attrs, "playerId"))
				info.playerId = atoi(data.attrs["playerId"].c_str());
			RichText::getActionDisplayInfo(data.attrs["id"].c_str(), info);
			addAction(info);
		}
	}

	if (data.tag == "img") {
		const char* tileName = 0;

		if( rs::Std::exists( data.attrs ,"tile")) tileName = data.attrs["tile"].c_str();
		if (rs::Std::exists(data.attrs, "id")) tileName = data.attrs["id"].c_str();
		if (rs::Std::exists(data.attrs, "grp")) tileName = data.attrs["grp"].c_str();

		if (!tileName) return;

		const char* libName = 0;
		if (rs::Std::exists(data.attrs, "lib")) libName = data.attrs["lib"].c_str();
		if (!libName) return;

		bool animated = rs::Std::exists(data.attrs, "animated") && (data.attrs["animated"] == "true");

		float px = 0;
		float py = 0;

		float ofsX = 0;
		float ofsY = 0;
		float paddingLeft = 0;
		float paddingRight = 0;

		bool filter = false;

		rd::TileLib* lib = r2::im::TilePicker::getOrLoadLib(libName);
		if (!lib) 
			return;

		if (rs::Std::exists(data.attrs, "filter")) filter = rd::String::parseBool(data.attrs["filter"].c_str());
		if (rs::Std::exists(data.attrs, "pivotX")) px = atof( data.attrs["pivotX"].c_str());
		if (rs::Std::exists(data.attrs, "pivotY")) py = atof( data.attrs["pivotY"].c_str());

		if (rs::Std::exists(data.attrs, "ofsX")) ofsX = atof(data.attrs["ofsX"].c_str());
		if (rs::Std::exists(data.attrs, "ofs-x")) ofsX = atof(data.attrs["ofs-x"].c_str());
		if (rs::Std::exists(data.attrs, "offset-x")) ofsX = atof(data.attrs["offset-x"].c_str());

		if (rs::Std::exists(data.attrs, "ofsY")) ofsY = atof(data.attrs["ofsY"].c_str());
		if (rs::Std::exists(data.attrs, "ofs-y")) ofsY = atof(data.attrs["ofs-y"].c_str());
		if (rs::Std::exists(data.attrs, "offset-y")) ofsY = atof(data.attrs["offset-y"].c_str());

		if (rs::Std::exists(data.attrs, "paddingLeft")) paddingLeft = atof(data.attrs["paddingLeft"].c_str());
		if (rs::Std::exists(data.attrs, "padding-left")) paddingLeft = atof(data.attrs["padding-left"].c_str());

		if (rs::Std::exists(data.attrs, "paddingRight")) paddingRight = atof(data.attrs["paddingRight"].c_str());
		if (rs::Std::exists(data.attrs, "padding-right")) paddingRight = atof(data.attrs["padding-right"].c_str());

		float constraintHeight = -1.0f;
		float constraintWidth = -1.0f;
		if (rs::Std::exists(data.attrs, "constraintHeight")) constraintHeight = atof(data.attrs["constraintHeight"].c_str());
		else if (rs::Std::exists(data.attrs, "constraintWidth")) constraintWidth = atof(data.attrs["constraintWidth"].c_str());
		else {
			if (rs::Std::exists(data.attrs, "height") && data.attrs["height"] == "auto") {
				constraintHeight = getFontSize();
			}
		}

		bool measures = true;
		if (rs::Std::exists(data.attrs, "measures")) measures = rd::String::parseBool(data.attrs["measures"].c_str());
		
		curX += paddingLeft;

		rd::ABitmap * e = rd::Pools::abitmaps.alloc();
		auto fd = lib->getFrameData(tileName);
		e->setCenterRatio(px, py);
		e->set(lib);
		e->playAndLoop(tileName);
		if (constraintWidth > 0) e->constraintHeight(constraintWidth);
		if (constraintHeight > 0) e->constraintHeight(constraintHeight);
		if (!measures)
			e->nodeFlags |= NodeFlags::NF_SKIP_MEASURES_FOR_BOUNDS;
		e->x = std::lrint(curX + ofsX);
		e->y = std::lrint(curY + ofsY + cur->getBase() - e->height());
		e->vars.set("r2::Text::lineIndex", curLogicalLine);
		e->vars.set("r2::Text::cx", curX);
		e->vars.set("r2::Text::cy", curY);
		e->vars.set("r2::Text::inlined", true);
		if (filter) e->texFilterLinear();
		addChild(e);
		curX += e->width()+ paddingRight;
		if (!animated) 
			e->stop();
	}
	else if (data.tag == "font") {
		if (rs::Std::exists(data.attrs, "name")) {
			auto f = FontManager::get().getFont( data.attrs["name"].c_str());
			fonts.push_back(fnt);
			fnt = f;
		}

		if (rs::Std::exists(data.attrs, "size")) {
			int sz = atoi(data.attrs["size"].c_str());
			textSize.push_back(sz);
		}

		if (rs::Std::exists(data.attrs, "color")) {
			pushCurrentColor();
			Color col = rd::String::readColor(data.attrs["color"].c_str());
			forceColor = col;
		}

	}
	else if (data.tag == "color") {
		pushCurrentColor();
		if( rs::Std::exists(data.attrs,"val")){
			forceColor = rd::String::readColor(data.attrs["val"].c_str());
		}
		else {
			if (rd::String::contains(t,"0x")) {
				t = rd::String::skip(t,"=");
				t = rd::String::skipWhitespace(t);
				if (*t == '\'')
					t++;
				else if (*t == '"')
					t++;
				if (rd::String::startsWith(t, "0x")) {
					
					forceColor = rd::String::readColor(t);
				}
			}
			else {
				trace("unable to read color literal");
			}
		}
	}
	else if (data.tag == "loc") {
		if (node) {
			auto& s = node->a0Data->strData;
			if(ri18n::T::has(s))
				node->repl = ri18n::T::get(s);
			else {
				#ifdef _DEBUG
				node->repl = "~"s + s;
				#endif
			}
		}
	}
	else if (data.tag == "i" || data.tag == "italic") {
		//TODO
		///pushItalic();
	}
}

void ri18n::RichText::stdTagEnd(const char* t) {
	rd::XmlData data;
	rd::String::parseXMLTags(t, data);
	if (data.tag == "font") {
		if (rs::Std::exists(data.attrs, "name")) {
			fnt = fonts.back();
			fonts.pop_back();
		}
		if (rs::Std::exists(data.attrs, "size")) {
			int sz = atoi(data.attrs["size"].c_str());
			textSize.pop_back();
		}
		if (rs::Std::exists(data.attrs, "color")) {
			popCurrentColor();
		}
	}
	if (rs::Std::exists(data.attrs, "class")) {
		Str& cl = data.attrs[StrRef("class")];
		auto pos = std::find(classes.begin(), classes.end(), cl);
		if (pos != classes.end()) {
			classes.erase(pos);
		}
	}
	if (data.tag == "em") onEmEnd();
	if (data.tag == "strong") onStrongEnd();
	if (data.tag == "color") {
		popCurrentColor();
	}
	else if ( data.tag == "i" || data.tag == "italic") {
		//TODO
		//setItalicBend(std::nullopt,false);
		//popItalic();
	}
}

r::Color ri18n::RichText::getTextColor(int idx){
	if (!colors.size())
		return r::Color::White;

	if( idx == -1)
		return colors.back().col;

	for(auto& c : colors){
		if (idx >= c.start && idx < c.end)
			return c.col;
	}

	return r::Color::White;
}

void ri18n::RichText::setTextColor(const r::Color& c) {
	pushCurrentColor();
	forceColor = c;
	r2::Text::setTextColor(c);
	popCurrentColor();
}

void ri18n::RichText::im(){
	using namespace ImGui;
	if (CollapsingHeader(ICON_MD_RTT " RichText")) {
		Indent();
		ImGui::Text("tree");
		tree->im();
		if (conf.mapCharactersToAst && TreeNode("Char Map To Nodes")) {
			if (TreeNode("Starts")) {
				ImGui::Text("warning it maps C chars not utf8");
				int idx = 0;
				for (auto & l : charsToNodeStarts) {
					ImGui::Text("pos : %d | char %c | 0x%x", idx, text[idx],text[idx]);
					for (AstNode* ev : l) {
						ev->im();
					}
					idx++;
				}
				TreePop();
			}
			TreePop();
		}
		Unindent();
	}
	r2::Text::im();
}

void ri18n::RichText::pushCurrentColor() {
	if(forceColor)
		saveColor.push_back(*forceColor);
}

void ri18n::RichText::popCurrentColor() {
	if (saveColor.size()) {
		forceColor = saveColor[saveColor.size() - 1];
		saveColor.pop_back();
	}
	else
		forceColor = std::nullopt;
}

void ri18n::RichText::pushItalic() {
	if (!italicMask) italicMask = new BitArray();
	markItalicCharacters = true;
}

void ri18n::RichText::popItalic() {
	markItalicCharacters = false;
}

void ri18n::RichText::pushCurrentOutline() {
	saveOutline.push_back(outline);
}

void ri18n::RichText::popCurrentOutline() {
	if (saveOutline.size()) {
		outline = saveOutline[saveOutline.size() - 1];
		saveOutline.pop_back();
	}
	else
		outline = nullopt;
}


void ri18n::RichText::serialize(Pasta::JReflect& jr, const char* _name) {
	if (rd::Bits::is(nodeFlags, NF_SKIP_SERIALIZATION))
		return;
	if (_name) jr.visitObjectBegin(_name);
	r2::Text::serialize(jr, 0);
	jr.visit(srcTree, "srcTree");
	jr.visit(emColor, "emColor");
	jr.visit(strongColor, "strongColor");
	jr.visit(strongOutlineColor, "strongOutlineColor");

	if (jr.isReadMode() && translationKey.empty() && !srcTree.empty() )
		setTree(srcTree.c_str());
	if (jr.isReadMode())
		cache();
	if (_name) jr.visitObjectEnd(_name);
}

void RichText::ensureCharMaps(){
	if (conf.mapCharactersToAst) {
		if (curLogicalCharacter>= charsToNodeStarts.size())
			charsToNodeStarts.resize(curLogicalCharacter+1);
		if (curLogicalCharacter>=charsToNodeEnd.size())
			charsToNodeEnd.resize(curLogicalCharacter+1);
	}
}


void RichText::setTree(const char* _tree) {
	if (tree) {
		delete tree;
		tree = 0;
	}

	if (!_tree) {
		cache();
		return;
	}

	srcTree = _tree;

	auto n = Nar().make(_tree);
	if (n) {
		tree = n;
		cache();
	}
	syncAllMatrix();
}

void ri18n::RichText::dispose() {
	al.dispose();
	if (tree) {
		delete tree;
		tree = 0;
	}
	classes.clear();
	charsToNodeStarts.clear();
	charsToNodeEnd.clear();
	fonts.clear();
	textSize.clear();
	curX = 0;
	curY = 0;
	r2::Text::dispose();
}

const char* ri18n::RichText::setText(const char* str) {
	setTree(Nar().make(str));
	return text.c_str();
}

void ri18n::RichText::setTree(AstNode* _tree){
	if (tree) {
		delete tree;
		tree = 0;
	}
	tree = _tree;
	cache();
}

void ri18n::RichText::cache(){
	clearContent();

	curX = 0;
	curY = 0;
	text.clear();
	fonts.clear();

	if (!tree)return;

	if (fnt == nullptr) 
		fnt = rd::FontManager::get().dflt;
	fonts.push_back(fnt);

	int align = blockAlign;
	Vector2 size = getSize();

	float &x = curX;
	float &y = curY;
	curPrio = 0;
	curLogicalCharacter = 0;
	curLogicalLine = 0;
	onFirstChar();
	blockAlign = 0;
	renderNode(tree, &x, &y);
	setLineInfos(curLogicalLine, { x, y + getLineHeight() });
	blockAlign = align;
	onLastChar();
	curLogicalCharacter = 0;
	curLogicalLine = 0;

	if (tree && bgColor.a > 0.f) {
		BatchElem* le = getFirstElement();
		BatchElem* e = rd::Pools::elems.alloc();
		e->x = 0;
		e->y = 0;
		e->getTile()->setTexture(GpuObjects::whiteTex);
		e->color = bgColor;
		e->setSize(size.x, size.y);
		e->setPriority(le->getPriority() + 20);
		e->blendmode = le->blendmode;
		add(e);
		auto& ei = elInfos[e->uid];
		ei.charcode = 0;
		ei.lineIdx = 0;
		ei.charPos = 0;
	}
	applyAlign();
	addBg();
	onTextChanged();
}

void ri18n::RichText::execute(std::function<void(AstNode*)>f) {
	if (tree&&f) f(tree);
}

void ri18n::RichText::renderNode(AstNode* tree, float* x, float* y) {
	if (!tree)
		return;

	bool doCharMaps = conf.mapCharactersToAst;

	if (doCharMaps) {
		ensureCharMaps();
		charsToNodeStarts[curLogicalCharacter].push_back(tree);
	}

	if (markItalicCharacters) 
		italicMask->set(curLogicalCharacter);

	switch (tree->type) {
	case AstNodeType::Seq: {
		renderNode(tree->a0Data, x, y);
		renderNode(tree->a1Data, x, y);
		break;
	}
	case AstNodeType::Sentence:{
		const char* str = tree->strData.c_str();
		text.append(str);
		while (str && *str != 0)
			str = pushLine(str, x, y);
		break;
	}
	case AstNodeType::Em: {
		onEmStart();
		renderNode(tree->a0Data, x, y);
		onEmEnd();
		break;
	}
	case AstNodeType::Strong: {
		onStrongStart();
		renderNode(tree->a0Data, x, y);
		onStrongEnd();
		break;
	}
	case AstNodeType::Event:
		onEvent(tree->strData.c_str());
		break;
	case AstNodeType::CondEvent:
		onCondEvent(tree->strData.c_str());
		break;
	case AstNodeType::UniqueEvent:
		onUniqueEvent(tree->strData.c_str());
		break;
	case AstNodeType::CondUniqueEvent:
		onCondUniqueEvent(tree->strData.c_str());
		break;
	case AstNodeType::ImportantEvent:
		onImportantEvent(tree->strData.c_str());
		break;
	case AstNodeType::Tag:
		onTagStart(tree->strData.c_str(), tree);
		if (!tree->repl.empty()){
			text += tree->repl;
			const char* str = tree->repl.c_str();
			while (str && *str != 0)
				str = pushLine(str, x, y);
		}
		else
			renderNode(tree->a0Data, x, y);
		onTagEnd(tree->strData.c_str());
		break;
	case AstNodeType::TagFrom:
		onTagFrom(tree->strData.c_str());
		break;
	case AstNodeType::Script:
		onScript(tree->strData.c_str());
		break;
	default:
		cout << "TODO" << to_string((int)tree->type) << endl;
		break;
	}

	if (doCharMaps) {
		ensureCharMaps();
		charsToNodeEnd[curLogicalCharacter].push_back(tree);
	}
}

RichText* RichText::mk(const char* txt, r2::Node* parent ) {
	auto n = Nar().make(txt);
	if (!n) return 0;
	return new RichText(n, parent);
}

RichText* RichText::mk(const char* txt, RichTextConf conf, r2::Node* parent)
{
	auto n = Nar().make(txt);
	if (!n) return 0;
	auto rc = new RichText(nullptr, parent);
	rc->conf = conf;
	rc->setTree(n);
	return rc;
}

void RichText::update(double dt){
	r2::Text::update(dt);
	al.update(dt);
}

void RichText::addInlinedNode(r2::Node * n) {
	n->x = curX;
	n->y = curY;
	n->vars.set("r2::Text::lineIndex", curLogicalLine);
	n->vars.set("r2::Text::cx", curX);
	n->vars.set("r2::Text::cy", curY);
	n->vars.set("r2::Text::inlined", true);
	curX += n->width();
	addChild(n);
}

void RichText::addAction(ActionDisplayInfo& info){
	int here = 0;
	if(info.raw)
		addInlinedNode(info.raw);
}


static RichText* sizer = 0;

r::Vector2 RichText::getTextExtent(rd::Font* fnt, int fontSize, const char* txt) {
	if (!sizer) sizer = new RichText();
	auto t = sizer;
	t->setFont(fnt);
	t->setFontSize(fontSize);
	t->setTree(txt);
	auto sz = t->getSize();
	t->dispose();
	return sz;
}

r::Vector2 RichText::getTextSimulatedSize(rd::Font* fnt, int fontSize, const char* txt) {
	if (!sizer) sizer = new RichText();
	auto t = sizer;
	t->setFont(fnt);
	t->setFontSize(fontSize);
	t->setTree(txt);
	auto sz = t->getSize();
	t->dispose();
	return sz;
}
