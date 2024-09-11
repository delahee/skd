#include "stdafx.h"

#include "1-input/InputEnums.h"
#include "rplatform/CrossPlatform.hpp"
#include "Console.hpp"


using namespace std;
using namespace rd;
using namespace r2;
using namespace Pasta;

Console * Console::me = nullptr;

eastl::vector<rd::Console*> rd::Console::ALL;

Console::Console(rd::Font * _fnt, r2::Node * parent) : fnt(_fnt), Node(parent) {
	me = this;
	name = "Console";
	producer = new r2::Text(fnt, nullptr);
	producer->name = "cons.prod";
	hostLua = new sol::state();
	LH = fnt->getSize() + 2;
	nodeFlags |= NF_ALPHA_UNDER_ZERO_SKIPS_DRAW;

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
	ALL.push_back(this);
}

Console::~Console() {
	if (me == this)
		me = 0;

	InputMgr* mgr = InputMgr::getSingleton();
	if(mgr) mgr->removeControllerListener(this);

	rs::Std::remove(ALL, this);
	if (ALL.size() == 0)
		rs::Sys::traceOverride = nullptr;

	destroyAllChildren();
	sb = nullptr;
	if (producer) {
		producer->destroy();
		producer = nullptr;
	}
	fnt = nullptr;

	if (hostLua)
		delete hostLua;
	int here = 0;
}

void rd::Console::init() {
	mkHost();
}

void rd::Console::release() {
	if (scriptingBackend == rd::ScriptLanguage::LUA) {
		hostLua->collect_garbage();
		*hostLua = sol::state();
	}
}

void Console::notifyKeyPressed(Pasta::ControllerType ctrl, Pasta::Key key) {
	pokeFade(false);
}
void Console::onConnexionChange(Pasta::ControllerType ctrl, bool connected) {
	pokeFade(false);
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
			pokeFade(true);
			input->focus();
		}
		return;
	}

	manageArchive(key);

	pokeFade(false);
}

void rd::Console::mkHost() {
	switch (scriptingBackend) {
		case rd::ScriptLanguage::LUA:
		{
			//deported to luaScriphHost for compile speed
			mkLuaHost();
			break;
		}
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
	pokeFade(false);
}

void rd::Console::update(double dt){
	Super::update(dt);

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

void rd::Console::pokeFade(bool forced) {
	if (!forced&&discreet)
		return;
	visible = true;
	timeToFade = 5.0;
	alphaTip = 1.0;
	alpha = 1.0;
}

void Console::dispose() {
	Super::dispose();
	if (sb) 
		sb->dispose();
	if ( producer )	
		producer->dispose();
}

bool rd::Console::hasFocus(){
	return input->hasFocus();
}

void rd::Console::im(){
	using namespace ImGui;
	if (TreeNode("Console")) {
		r2::Im::nodeButton( producer, "producer");
		Value("archive",archive);
		for (auto& l : lines) {
			if (TreeNode(l.content)) {
				l.im();
				TreePop();
			}
		}
		TreePop();
	}
	Super::im();
}

void Console::help() {
	if(me)
		me->log("Hello!");
}

void Console::log(const std::string & str) {
	log(str.c_str());
}

void Console::clear() {
	for (auto& l : lines){
		for (auto spr : l.spr) 
			spr->destroy();
		l.spr.clear();
	}
	
	lines.clear();
	timeToFade = -1.0;
	visible = false;
}

void Console::log(const char * str) {
	fullLog += str;
	fullLog += "\n";

	CLine l;
	l.content = str;

	producer->setText(str);
	std::vector<r2::BatchElem*> ae = producer->getAllElements();
	for (int i = 0; i < ae.size(); i++) {
		r2::BatchElem* e = ae[i];
		if (e){
			e->detach();
			l.spr.push_back(e);
			l.sprPos.push_back(Vector2(e->x, e->y));
			sb->add(e);
		}
	}
	l.height = producer->getSize().y;
	producer->removeAllElements();

	lines.push_back(l);
	calcLineCoords();
	pokeFade(false);
}

void Console::calcLineCoords() {
	auto start = lines.begin();
	bool full = false;
	bool debug = false;

	if (lines.size() > 10) {
		start = lines.end() - 10;
		full = true;
	}

	bool cleanupWastedLines = true;
	if( full && cleanupWastedLines){
		int nb = start - lines.begin();
		if (nb > 0) {
			//if(debug) 
			//	printf("cleaning wasted lines %d\n", nb);
			for (int i = 0; i < nb; ++i) {
				rd::CLine& l = *lines.begin();
				for (auto& b : l.spr) 
					if(b)
						b->destroy();
				l.spr.clear();
				lines.erase(lines.begin());
			}
		}
	}

	for (rd::CLine & cl : lines) {
		for (r2::BatchElem* e : cl.spr)
			if (e)
				e->visible = false;
			//else
			//	__debugbreak;
	}

	int total = 0;
	for (auto iter = lines.begin(); iter != lines.end(); iter++) {
		CLine & l = *iter;
		total += l.height;
	}

	int cy = 0;
	for (auto iter = lines.begin(); iter != lines.end(); iter++) {
		CLine & l = *iter;
		int k = 0;
		for (r2::BatchElem * e : l.spr) {
			if (e) {
				e->x = l.sprPos[k].x + 1;
				e->y = l.sprPos[k].y - total + cy - LH;
				e->visible = true;
			}
			k++;
		}
		cy += l.height;
	}

	bg->clear();
	bg->setGeomColor(r::Color(0.5f, 0.3f, 0.3f, 0.2f));
	bg->drawQuad(0, - LH - total, LW, -LH);
}


void Console::runAsSuperUser(const char* str){
	auto old = isSuperUser;
	isSuperUser = true;
	runCommand(str);
	isSuperUser = old;
}
void Console::runAsUser(const char* str) {
	runCommand(str);
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
			hostLua->unsafe_script(str);
			break;
		}
	}
	catch (const sol::error& e) {
		std::string msg = std::string("Lua Error\n") + e.what();
		log( msg );
		printf(msg.c_str());
	}
	catch (std::exception &e) {
		std::string msg = std::string("Std Error\n") + e.what();
		log( msg );
		printf(msg.c_str());
	}

}

void rd::CLine::im(){
	using namespace ImGui;
	Checkbox("isCommand", &isCommand);
	Value("content", content);
	Value("height", height);
	for (auto iter = spr.begin(); iter != spr.end(); iter++) {
		if (!*iter) {
			ImGui::Text("no batch elem");
			trace("no batch elem");
		}
		else {
			r2::Im::beButton(*iter);
		}
	}
	for (auto iter = spr.begin(); iter != spr.end(); iter++) {
		Value("pos", *iter);
	}
}
