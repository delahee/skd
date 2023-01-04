#include "stdafx.h"

#include "1-graphics/geo_vectors.h"
#include "1-graphics/Graphic.h"
#include "1-texts/TextMgr.h"
#include "1-graphics/Texture.h"
#include "PastaAssert.h"

#include "Text.hpp"

#include "rd/Pools.hpp"

#ifdef _DEBUG
#include "ri18n/T.hpp"
#endif

using namespace std;
using namespace r2;
using namespace Pasta;



#define SUPER Batch
Text::Text(rd::Font * _fnt, const char * text, r2::Node * parent ) : SUPER ( parent ){
	bgColor = r::Color(0, 0, 0, 0);
	this->fnt = _fnt;
	if (fnt == nullptr) fnt = r2::GpuObjects::defaultFont;
	if(fnt)
		this->shader = fnt->getShader();
	setText(text);
	this->setTextColor(r::Color(1.f, 1.f, 1.f));
#ifdef _DEBUG
	name = "Text#" + to_string(uid);
#endif
	blendmode = Pasta::TransparencyType::TT_ALPHA;
}

r2::Text::Text(FontSetup fnt, const char* txt, r2::Node* parent) : r2::Text(fnt.font, txt, parent){
	setFontSize(fnt.size);
};

r2::Text::~Text() {
	dispose();
};

void r2::Text::reset() {
	SUPER::reset();
	colors.clear();
	if (dropShadow)		delete dropShadow;		dropShadow = nullptr;
	if (outline)		delete outline;			outline = nullptr;
	clearContent();
	italicBend = std::nullopt;
	beforeDrawFlush = nullptr;
	fnt = nullptr;
	setText("");
	blockAlign = 0;
	curPrio = 0;
	bgColor = r::Color(0, 0, 0, 0);
	curLogicalCharacter = 0;
	forceColor = std::nullopt;
	maxLineWidth = -1;
	originalMaxLineWidth = -1;
}

void r2::Text::dispose() {
	SUPER::dispose();
	colors.clear();
	if (dropShadow)		delete dropShadow;		dropShadow = nullptr;
	if (outline)		delete outline;			outline = nullptr;
	italicBend = std::nullopt;
	beforeDrawFlush = nullptr;
	clearContent();
	fnt = nullptr;
	setText("");
	blockAlign = 0;
	curPrio = 0;
	bgColor = r::Color(0, 0, 0, 0);
	curLogicalCharacter = 0;
	forceColor = std::nullopt;
	maxLineWidth = -1;
	originalMaxLineWidth = -1;
}

const char* r2::Text::getFontName() const {
	if (!fnt)return 0;
	return rd::FontManager::get().getFontName(fnt);
}

void r2::Text::setFont(rd::Font * fnt){
	this->fnt = fnt;
	if (this->fnt == nullptr) this->fnt = r2::GpuObjects::defaultFont;
	this->shader = this->fnt->getShader();
	cache();
}

std::string & r2::Text::setText(const char * str){
	if (str == nullptr) {
		text = "";
		return text;
	}

	text = str;
	cache();
	return text;
}

void r2::Text::setBgColor(r::Color c){
	bgColor = c;
	cache();
}

const char* r2::Text::pushLineStraight(const char* str, float* _x, float* _y, int count /*= -1*/) {
	if (count == -1)
		count = 1024 * 1024;

	float savedX = *_x;
	float savedY = *_y;
	float w = 0.0;
	getLineWidth(str, &w,count);

	for (; *str != 0;){
		if (*str == '\n') {

			auto ox = *_x;

			*_x = 0;
			*_y += getLineHeight() + lineSpacingOffset;

			setLineInfos(curLogicalLine, { ox, *_y });

			curLogicalCharacter++;
			curLogicalLine++;
			return str + 1;
		}

		int incr;
		int charId, nextCharId;

		getTextChar(str, 0, &incr, &charId, &nextCharId);
		pushChar(charId, nextCharId, curLogicalCharacter, _x, _y);

		if (incr == 0) //ill formed utf8?
			return 0;

		str += incr;
		curLogicalCharacter++;
		count--;
		if (count == 0) {
			break;
		}
	}
	return str;
}

int	r2::Text::getNextCesurePoint(const char* text) {
	const char* str = text;
	for (; *str != 0;) {
		if (*str == '\n') 
			return (str + 1) - text;
		
		int incr;
		int charId, nextCharId;
		getTextChar(str, 0, &incr, &charId, &nextCharId);
		if (incr == 0) 
			return 0;
		if (canBeCesured(charId))
			return (str + incr) - text;
		str += incr;
	}
	return str - text;
}

void r2::Text::onBeforeDrawFlush() {
	if (!italicBend)
		return;

	const int nbElems = this->nbElems;
	const int nbVertPerElem = 4;
	const int stride = 9;

	auto getX = [](float* vert) -> float& {
		return vert[0];
	};
	auto getY = [](float* vert) -> float &{
		return vert[1];
	};

	//ideally we should pump out a shear matric with an angle but come on.
	int fSize = fnt->getSize();
	for (int i = 0; i < nbElems; ++i) {
		float* tl = &(fbuf.data()[ i * nbVertPerElem * stride + 0]);
		float* tr = &(fbuf.data()[ i * nbVertPerElem * stride + stride * 1]);
		float* bl = &(fbuf.data()[ i * nbVertPerElem * stride + stride * 2]);
		float* br = &(fbuf.data()[ i * nbVertPerElem * stride + stride * 3]);
		float& tlx = getX(bl);
		float& trx = getX(br);
		float bend = *italicBend;
		float h = getY(tl) - getY(bl);
		float coef = h / fSize;
		getX(tl)+= bend * coef;
		getX(tr) += bend * coef;
	}
}

void r2::Text::setLineInfos(int idx, LineInfo li) {
	if (idx >= lineInfos.size())
		lineInfos.resize(idx + 1);
	lineInfos[idx] = li;
}

void r2::Text::applyAlign() {
	if (blockAlign == 0) return;
	auto sz = getSize();
	float xOffset = 0;
	float yOffset = 0;
	auto el = head;
	auto& h = lineInfos.back().bottom;
	if ((blockAlign & ALIGN_VCENTER) == ALIGN_VCENTER)
		yOffset -= h / 2;
	else if ((blockAlign & ALIGN_BOTTOM) == ALIGN_BOTTOM)
		yOffset -= h;
	if (snapToPixel)
		y = floor(y);
	while (el) {
		auto lineIdx = elInfos[el->uid].lineIdx;
		auto& lineInfo = lineInfos[lineIdx];
		xOffset = 0;
		if ((blockAlign & ALIGN_HCENTER) == ALIGN_HCENTER)
			xOffset -= lineInfo.width * 0.5f;
		else if ((blockAlign & ALIGN_RIGHT) == ALIGN_RIGHT)
			xOffset -= lineInfo.width;
		if (snapToPixel)
			xOffset = floor(xOffset);
		el->x += xOffset;
		el->y += yOffset;
		el = el->next;
	}
	
}

void r2::Text::setItalicBend(r::opt<int> point) {
	italicBend = point;
	if (point.has_value()) {
		beforeDrawFlush = [this]() {
			onBeforeDrawFlush();
		};
	}
	else
		beforeDrawFlush = nullptr;
	cache();
}

int	r2::Text::getAnteCesurePoint(const char* text, float beforeW) {
	const char* str = text;
	int lastCesure = 0;
	int nbChar = 0;
	std::string sub;
	for (; *str != 0;){
		if (*str == '\n')
			return (str + 1) - text;
		//evaluate next char
		int incr =  0;
		int charId = 0; int nextCharId = 0;
		getTextChar(str, 0, &incr, &charId, &nextCharId);
		if (incr == 0)
			return 0;
		nbChar++;
		
		//is it a valid cesure point
		if (canBeCesured(charId))
			lastCesure = nbChar;

		sub.assign(str, incr);
		float lw = 0.0;
		getLineWidth(sub.c_str(), &lw, 1);
		beforeW -= lw;
		//ok continue BUt ony if we improved overall so that we grind space anyway, important for large words in smaller columns
		if (beforeW < 0.0 && lastCesure != 0)
			return lastCesure;
		str += incr;
	}
	return lastCesure;
}

const char* r2::Text::pushLineWithCesure(const char* str, float* _x, float* _y) {
	float savedX = *_x;
	float savedY = *_y;
	float w = 0.0;
	getLineWidth(str, &w);//eval cur line
	if( (*_x + w) > maxLineWidth ) { // read up until cesure is possible
		int nextCesurePoint = getAnteCesurePoint(str, maxLineWidth- *_x);
		if (nextCesurePoint == 0) {//we are already larger and next cesure point is...right now... it smells
			if (*_x != 0) {//otherwise line is already fresh...
				auto ox = *_x;
				*_x = 0;
				*_y += getLineHeight() + lineSpacingOffset;
				setLineInfos(curLogicalLine,{ ox, *_y });
				curLogicalLine++;
			}
			str = pushLineStraight(str, _x, _y);
		}
		else {
			int len = rd::String::strlenUTF8(str);
			if (nextCesurePoint >= len) 
				str = pushLineStraight(str, _x, _y);
			else {
				str = pushLineStraight(str, _x, _y, nextCesurePoint);
				//then line jump
				//if (_x >= (float)maxLineWidth) 
				{
					auto ox = *_x;
					*_x = 0;
					*_y += getLineHeight() + lineSpacingOffset;

					setLineInfos(curLogicalLine, { (double)ox,*_y });
					curLogicalLine++;
				}
			}
		}
	}
	else 
		str = pushLineStraight(str, _x, _y);

	return str;
}

const char * r2::Text::pushLine(const char * str, float *_x, float *_y){
	if (maxLineWidth <= 0) 
		str = pushLineStraight(str, _x, _y);
	else 
		str = pushLineWithCesure(str, _x, _y);
	return str;
}


void Text::pushChar(int charId, int nextCharId, int charPos, float *_x, float *_y){
	char c = charId;
	if (!fnt) 
		return;
	const CharDescr *ch = fnt->getCharDescr(charId);
	if (!ch) 
		return;

	float w = float(ch->srcW);
	float h = float(ch->srcH);
	float ox = float(ch->xOff);
	float oy = float(ch->yOff);
	float u1 = ch->srcX;
	float v1 = ch->srcY;
	float u2 = u1 + ch->srcW;
	float v2 = v1 + ch->srcH;
	int page = ch->page;
	Texture * texture = fnt->getTexture(page);
	if (!texture) 
		return;
	BatchElem * q = rd::Pools::elems.alloc();

#if _DEBUG
	//q->name = std::string({ (char)(charId),(char)0 });
#endif

	rd::Bits::unset(q->beFlags, NF_ENGINE_HELPER);
	q->x = *_x + ox;
	q->y = *_y + oy;
	
	float tw = texture->getWidth();
	float th = texture->getHeight();
	q->getTile()->setTexture(texture, u1/tw, v1/th, u2/tw, v2/th);
	q->setSize(w, h);
	q->z = 0.0;
	q->setPriority( - curPrio + (page<<16) );//group rendering by pages but keep order
	q->userdata = (void*)(charId);
	if (retainElementInfos) {
		auto& ei = elInfos[q->uid];
		ei.charcode = charId;
		ei.lineIdx = curLogicalLine;
		ei.charPos = curLogicalCharacter;
	}

	if(!forceColor)
		for (TextColorRange& c : colors) {
			int s = c.start;
			int e = c.end;
			if (e == -1) e = 1024 * 1024;//we don't really care if we exceed text size but text size can mean so much things, better not use it

			if (charPos >= c.start && charPos < e) {
				if (c.multiply) {
					q->color.r *= c.col.r;
					q->color.g *= c.col.g;
					q->color.b *= c.col.b;
					q->color.a *= c.col.a;
				}
				else 
					q->color = c.col;
			}
		}
	else {
		q->color = *forceColor;
	}

	if (dropShadow != nullptr) {
		BatchElem * qShadow = rd::Pools::elems.alloc();
		qShadow->beFlags |= NF_ENGINE_HELPER;
		qShadow->x = q->x + dropShadow->dx;
		qShadow->y = q->y + dropShadow->dy;
		qShadow->z = q->z - 1e-6f;

		qShadow->getTile()->setTexture(texture, u1 / tw, v1 / th, u2 / tw, v2 / th);

		qShadow->setSize(q->width(), q->height());
		qShadow->z = q->z;
		qShadow->setPriority(q->getPriority() + 20);

		qShadow->color = dropShadow->col; // Alpha inside color
		qShadow->userdata = q;

		if (retainElementInfos) {
			auto& ei = elInfos[qShadow->uid];
			ei.charcode = charId;
			ei.lineIdx = curLogicalLine;
			ei.charPos = curLogicalCharacter;
		}

		add(qShadow);
	}
	if (outline != nullptr) {
		for (int i = 0; i < 4; i++) {
			BatchElem * qShadow = rd::Pools::elems.alloc();
			qShadow->beFlags |= NF_ENGINE_HELPER;
			qShadow->x = q->x;
			qShadow->y = q->y;
			qShadow->z = q->z - 1e-6f;

			float delta = outline->delta;
			if (i == 0) qShadow->x+=delta;
			if (i == 1) qShadow->x-=delta;
			if (i == 2) qShadow->y+=delta;
			if (i == 3) qShadow->y-=delta;

			qShadow->getTile()->setTexture(texture, u1 / tw, v1 / th, u2 / tw, v2 / th);

			qShadow->setSize(q->width(), q->height());
			qShadow->z = q->z;
			qShadow->setPriority(q->getPriority() + 10);

			qShadow->color = outline->col; // Alpha inside color
			qShadow->userdata = q;
			if (retainElementInfos) {
				auto& ei = elInfos[qShadow->uid];
				ei.charcode = charId;
				ei.lineIdx = curLogicalLine;
				ei.charPos = curLogicalCharacter;
			}
			add(qShadow);
		}
	}

	add(q);
	curPrio++;

	if (onPushChar)
		onPushChar(q);
	advanceCursorPos(_x, _y, ch, nextCharId);
}


void r2::Text::clearContent() {
	destroyAllElements();
	elInfos.clear();
	lineInfos.clear();
	italicBend = std::nullopt;
}

void Text::cache() {
	clearContent();

	if (fnt == nullptr) 
		return;
	
	float x = 0.0;
	float y = 0.0;
	curPrio = 0;
	curLogicalCharacter = 0;
	curLogicalLine = 0;
	const char *str = text.c_str();
	while (str && *str != 0) {
		str = pushLine(str, &x, &y);
	}
	setLineInfos(curLogicalLine, { x, y+getLineHeight() });

	curLogicalCharacter = 0;
	curLogicalLine = 0;
	
	applyAlign();
	addBg();

	onTextChanged();
}

void Text::addBg(){
	if (text.size() && bgColor.a > 0.f) {
		r2::Bounds b;
		r2::Bounds tmp;
		auto el = head;
		while (el) {
			tmp.empty();
			el->getLocalBounds(tmp);
			b.add(tmp);
			el = el->next;
		}

		BatchElem* le = firstElem();
		BatchElem* e = rd::Pools::elems.alloc();
		e->x = -bgExtent.x + b.left();
		e->y = -bgExtent.y + b.top();
		e->tile->copy(*GpuObjects::whiteTile);
		e->color = bgColor;
		e->setSize(b.width() + bgExtent.z * 2, b.height() + bgExtent.w * 2);
		e->setPriority(le->getPriority() + 20);
		add(e);
	}
}
//not accurate 
Pasta::Vector2 Text::getTextSize(const char * text){
	Pasta::Vector2 ret;
	if (fnt == nullptr)
		return ret;

	float w = 0;
	float x = 0;
	int nbLines = 0;

	while (*text != 0){
		auto next = getLineWidth(text, &w);
		if (maxLineWidth >= 0 && w > maxLineWidth) {
			int nextCesurePoint = getAnteCesurePoint(text, maxLineWidth);
			int len = rd::String::strlenUTF8(text);
			if (nextCesurePoint > 0 && nextCesurePoint < len) {
				w = maxLineWidth;
				next = text + nextCesurePoint;
			}
		}
		text = next;
		ret.x = Max(ret.x, w);
		nbLines++;
	}
	nbLines = Max(nbLines, 1);
	ret.y = (getLineHeight()+ lineSpacingOffset) * nbLines;

	return ret;
}

bool r2::Text::canBeCesured(int charId) {
	switch (charId)
	{
	case  0x00A0:
	case '\r':
	case '\t':
	case '\n':
	case ' ':
		return true;
	default:
		return false;
	};
};

r::Vector2 r2::Text::getTextExtent(rd::Font* fnt, int fontSize, const char* txt){
	auto t = rd::Pools::texts.alloc();
	t->setFont(fnt);
	t->setFontSize(fontSize);
	t->setText(txt);
	auto sz = t->getSize();
	return sz;
}

r::Vector2 r2::Text::getTextSimulatedSize(rd::Font* fnt, int fontSize, const char* txt){
	auto t = rd::Pools::texts.alloc();
	t->setFont(fnt);
	t->setFontSize(fontSize);
	t->setText(txt);
	return r::Vector2(t->width(), t->height());
}

const char * r2::Text::getLineWidth(const char *text, float *w, int count /*= -1*/)
{
	if (fnt == nullptr) {
		*w = 0.f;
		return text;
	}
	float _x = 0;
	float _y = 0;
	if (count == -1) count = 1024 * 1024;

	for (; *text != 0;)	{
		if (*text == '\n'){
			*w = _x;
			return text + 1;
		}
		int nextPos;
		int charId, nextCharId;
		getTextChar(text, 0, &nextPos, &charId, &nextCharId);
		const CharDescr *ch = fnt->getCharDescr(charId);

#ifdef _DEBUG
		if( charId != '?' && ch == fnt->getCharDescr('?')){
			if(ri18n::T::onUnknownFontCharacterEncountered)
				ri18n::T::onUnknownFontCharacterEncountered(charId, fnt);
		}
#endif
		advanceCursorPos(&_x, &_y, ch, nextCharId);

		if (nextPos == 0) // whoaa dude, can arrive if fed garbage
			nextPos++;

		text += nextPos;
		count--;
		if(!count) 
			break;
	}

	*w = _x;
	return text;
}

void r2::Text::advanceCursorPos(float *_x, float *_y, const Pasta::CharDescr *ch, int nextCharId) 
{
	Vector2 advance = FontResource::getSpacing(ch, nextCharId);
	*_x += advance.x;
	*_y += advance.y;
}

void r2::Text::getTextChar(const char *text, int pos, int *nextPos, int *charId, int *nextCharId){
	*nextPos = pos;
	*charId = TextMgr::decode1UTF8(text, nextPos);
	int nextNextPos = *nextPos;
	*nextCharId = TextMgr::decode1UTF8(text, &nextNextPos);
}

float Text::getLineHeight(){
	if (fnt == nullptr) return 0.0;
	return fnt->getLineHeight();
}

void r2::Text::setLineSpacingOffset(float f) { 
	lineSpacingOffset = f; 
	cache();
}

void r2::Text::setMaxLineWidth(int v) { 
	originalMaxLineWidth = v;
	maxLineWidth = v / scaleX; 
	cache(); 
}

void r2::Text::setTextColor(const Pasta::Color & c) {
	setTextColor(r::Color(c.r, c.g, c.b, c.a));
}

void r2::Text::setTextColor( const r::Color & c ){
	colors.clear();
	TextColorRange r;
	r.col = c;
	r.start = 0;
	r.end = -1;
	colors.push_back(r);
	cache();
}


void r2::Text::addTextColor( r::Color c, int start, int end /*=-1*/){
	TextColorRange r;
	r.col = c;
	r.start = start;
	r.end = end;
	colors.push_back(r);
	cache();
}


void r2::Text::addTextColor( Pasta::Color c, int start, int end /*=-1*/){
	TextColorRange r;
	r.col = c;
	r.start = start;
	r.end = end;
	colors.push_back(r);
	cache();
}

void r2::Text::replaceTextColor( Pasta::Color c, int start, int end /*=-1*/){
	replaceTextColor(r::Color(c.r, c.g, c.b, c.a),start,end);
}

void r2::Text::replaceTextColor(r::Color c, int start, int end /*=-1*/) {
	TextColorRange r;
	r.multiply = false;
	r.col = c;
	r.start = start;
	r.end = end;
	colors.push_back(r);
	cache();
}

// col manage the alpha
void r2::Text::addDropShadow(float dx, float dy, r::Color col) {
	if (!dropShadow) dropShadow = new r2::DropShadow();

	dropShadow->dx = dx;
	dropShadow->dy = dy;
	dropShadow->col = col;
	cache();
}

// col manage the alpha
void r2::Text::addOutline(r::Color col,float delta) {
	if (outline) delete outline;
	outline = new r2::Outline();
	outline->col = col;
	outline->delta = delta;
	cache();
}

void r2::Text::setBlockAlign(int align) {
	blockAlign = align;
	cache();
}

r2::Text* r2::Text::fromPool(rd::Font* fnt, const char* txt, r::Color color,  r2::Node* parent ) {
	auto t = rd::Pools::texts.alloc();
	t->setFont(fnt);
	t->setText(txt);
	t->setTextColor(color);
	if (parent)
		parent->addChild(t);
	return t;
}

r2::Node* r2::Text::clone(r2::Node* n) {
	if(!n) n = new r2::Text();
	SUPER::clone(n);

	Text * s = dynamic_cast<Text*>(n);
	if (s) {//we may sometime want to transfer only parent!
		s->snapToPixel = snapToPixel;
		s->bgColor = bgColor;
		s->blockAlign = blockAlign;
		s->fnt = fnt;
		s->text = text;
		s->curPrio = curPrio;
		if(s->dropShadow)
			s->dropShadow = new DropShadow(*dropShadow);
		if (s->outline)
			s->outline = new Outline(*outline);
		s->colors = colors;
		s->curLogicalCharacter = 0;
		s->curLogicalLine = 0;
		s->cache();
	}
	return n;
}

float r2::Text::getFontSize() {
	return scaleY * getLineHeight();
}

void r2::Text::setFontSize(std::optional<float> newHeight) {
	if (!newHeight ) {
		scaleY = scaleX = 1.0f;
		maxLineWidth = originalMaxLineWidth;
	}
	else {
		float lh = getLineHeight();
		float ratio = *newHeight / lh;
		scaleY = ratio;
		scaleX = scaleY;
		maxLineWidth = originalMaxLineWidth / scaleX;
	}
	cache();
	
}

void r2::Text::setBgExtent(const r::Vector4& ex) {
	bgExtent = ex;
	cache();
}

void r2::Text::autosize(double maxWidth, int targetFontSize, int minFontSize){
	if (!fnt) return;

	auto srcTarget = targetFontSize;
	auto srcMin = minFontSize;
	if(minFontSize<0){
		if( fnt->getShader() == r2::Shader::SH_MSDF){
			minFontSize = 24;
		}
		else {
			minFontSize = 8;
		}
	}

	if (targetFontSize < minFontSize)
		targetFontSize = minFontSize;

	for (int i = targetFontSize; i >= minFontSize; --i) {
		setFontSize(targetFontSize);
		if (width() <= maxWidth) {
			autoSizeTarget = srcTarget;
			autoSizeMin = minFontSize;
			autoSizeActual = targetFontSize;
			autoSizeWidth = maxWidth;
			return;
		}
		targetFontSize--;
	}
}

#undef SUPER

