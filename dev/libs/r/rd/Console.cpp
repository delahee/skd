#include "stdafx.h"
#include "Console.hpp"
#include "1-input/InputEnums.h"

using namespace std;
using namespace rd;
using namespace r2;
using namespace Pasta;

static Console * _self = nullptr;

#define SUPER Node
Console::Console(rd::Font * _fnt, r2::Node * parent) : fnt(_fnt), Node(parent) {
	_self = this;
	name = "Console";
	producer = new r2::Text(fnt, nullptr);
	producer->name = "cons.prod";

	LH = fnt->getSize() + 2;

	if (getScene()) {
		Scene * sc = getScene();
		y = sc->height();
		LW = sc->width();
	}

	bg = new Graphics(this);
	producer->name = "cons.bg";
	InputMgr * mgr = InputMgr::getSingleton();
	mgr->addControllerListener(this);

	sb = new Batch(this);

	InputConf cnf(fnt);
	cnf.width = LW;
	cnf.bgColor = r::Color(0,0,0,1);
	cnf.textColor = r::Color(1,1,1,1);

	input = new InputField(cnf, this);
	input->name = "cons.input";
	input->y = -LH;

	input->deferValidation = true;
	input->onChangeCbk = [this](std::string & value) {
		runCommand(value);
		value = "";
	};

	init();
}

Console::~Console() {
#if _DEBUG
	printf("deleting console\n");
#endif

	deleteAllChildren();
	sb = nullptr;
	if (producer) {
		delete producer;
		producer = nullptr;
	}
	fnt = nullptr;
}

void rd::Console::init() {
	mkHost();
}

void rd::Console::release() {
	if (scriptingBackend == rd::ScriptLanguage::LUA) {
		hostLua.collect_garbage();
		hostLua = sol::state();
	}
}

void Console::notifyKeyPressed(Pasta::ControllerType ctrl, Pasta::Key key) {
	pokeFade();
}
void Console::onConnexionChange(Pasta::ControllerType ctrl, bool connected) {
	pokeFade();
}

void Console::notifyKeyReleased(Pasta::ControllerType ctrl, Pasta::Key key) {
	InputMgr * mgr = InputMgr::getSingleton();

	if (ctrl != CT_KEYBOARD) return;

	//unsigned char c = mgr->getLocalCharOfKey(key);
	//printf("%c \n", c);

	if (!listening) {
		if (key == KB_SLASH || key == KB_NUMPAD_DIVIDE || key == KB_GRAVE) {
			//focus !
			listening = true;
			pokeFade();
			input->focus();
		}
		return;
	}

	manageArchive(key);

	pokeFade();
}

void rd::Console::mkHost() {
	switch (scriptingBackend) {
	case rd::ScriptLanguage::LUA:
		hostLua.collect_garbage();
		hostLua = sol::state();
		hostLua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::io, sol::lib::string, sol::lib::table);
		hostLua.set_function("log", sol::overload(
			[this](const char* str) { log(str); },
			[this](std::string str) { log(str); }
		));
		break;
	}
}

void rd::Console::manageArchive(Pasta::Key key ) {
	if (archive.size() == 0) return;

	if (key == KB_UP) {
		archivePos--;
		if(archivePos < 0) archivePos = archive.size() - 1;
		input->setValue(archive[archivePos]);
	}

	if (key == KB_DOWN) {
		archivePos++;
		archivePos = archivePos % archive.size();
		input->setValue(archive[archivePos]);
	}
	pokeFade();
}

void rd::Console::update(double dt){
	SUPER::update(dt);

	timeToFade -= dt;

	if (timeToFade < 0) {
		if (visible) {
			if (alphaTip <= 0.0) {
				input->blur(true);
				visible = false;
				listening = false;
			}
			else {
				alphaTip -= dt;
				alpha = alphaTip;
			}
		}
	}
	else {
		visible = true;
	}

	if (!show) {
		timeToFade = -1.0;
		visible = false;
	}

	if (!input->hasFocus()) {
		listening = false;
	}
}

void rd::Console::pokeFade() {
	visible = true;
	timeToFade = 5.0;
	alphaTip = 1.0;
	alpha = 1.0;
}

void Console::dispose() {
	SUPER::dispose();

	if ( sb )		sb->dispose();
	if ( producer )	producer->dispose();
}

bool rd::Console::hasFocus(){
	return input->hasFocus();
}

void Console::help() {
	_self->log("Hello!");
}

void Console::log(const std::string & str) {
	log(str.c_str());
}

void Console::clear() {
	lines.clear();
	timeToFade = -1.0;
	visible = false;
}

void Console::log(const char * str) {
	CLine l;
	l.content = str;

	producer->setText(str);

	std::vector<r2::BatchElem*> ae = producer->getAllElements();
	for (int i = 0; i < ae.size(); i++) {
		r2::BatchElem * e = ae[i];
		l.spr.push_back( e );
		l.sprPos.push_back(Vector2(e->x, e->y));
		sb->add(e);
	}
	l.height = producer->getSize().y;
	producer->removeAllElements();

	lines.push_back(l);
	calcLineCoords();
	pokeFade();
}

void Console::calcLineCoords() {
	auto start = lines.begin();
	if (lines.size() > 10)
		start = lines.end() - 10;

	for (auto& l : lines) {
		for (r2::BatchElem * e : l.spr)
			e->visible = false;
	}

	int total = 0;
	for (auto iter = start; iter != lines.end(); iter++) {
		CLine & l = *iter;
		total += l.height;
	}

	int cy = 0;
	for (auto iter = start; iter != lines.end(); iter++) {
		CLine & l = *iter;
		int k = 0;
		for (r2::BatchElem * e : l.spr) {
			e->x = l.sprPos[k].x + 1;
			e->y = l.sprPos[k].y - total + cy - LH;
			e->visible = true;
			k++;
		}
		cy += l.height;
	}

	bg->clear();
	bg->setGeomColor(r::Color(0.5f, 0.3f, 0.3f, 0.2f));
	bg->drawQuad(0, - LH - total, LW, -LH);
}

void Console::runCommand(const std::string & str) {
	runCommand(str.c_str());
}

void Console::runCommand(const char * str){
	archive.push_back(str);
	archivePos = -1;

	try {
		log(str);
		switch (scriptingBackend) {
		case rd::ScriptLanguage::LUA:
			hostLua.unsafe_script(str);
			break;
		}
	}
	catch (const sol::error& e) {
		log( std::string("Lua Error\n" ) + e.what() );
	}
	catch (std::exception &e) {
		log( std::string("Std Error\n" ) + e.what() );
	}

}

#undef SUPER