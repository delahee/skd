#include "stdafx.h"

#include "rui/IInteractive.hpp"
#include "rui/Menu.hpp"

#define SUPER r2::Node

using namespace std;
using namespace rui;

eastl::vector<rui::Menu*> rui::Menu::ALL;

rui::Menu::Menu( r2::Node * parent ) : SUPER(parent){
	al = new AgentList();
	inputBhv = new AgentList();
	ALL.push_back(this);
	name = "Menu#" + to_string(uid);
	al->push_back(&tw);
}

rui::Menu::~Menu() {
	dispose();

	if(al) delete al;
	al = nullptr;

	if (inputBhv) delete inputBhv;
	inputBhv = nullptr;
}

void rui::Menu::update(double dt){
	SUPER::update(dt);
	if (autoNav && navDirty) 
		startNavigation(currentFocus);

	if(al) al->update(dt);

	if (acceptsInput()) {
		doControls();
		if (inputBhv) inputBhv->update(dt);
	}

	yieldControls--;
	life++;
}

void rui::Menu::dispose() {
	if (al)
		al->dispose();

	if (blocker) {
		blocker->destroy();
		blocker = 0;
	}
	disposed = true;
	menuInputEnabled = false;

	auto pos = std::find(ALL.begin(), ALL.end(), this);
	if(pos!=ALL.end())
		ALL.erase(pos);

	r2::Node::dispose();

	sigDispose.trigger();
}

bool rui::Menu::isFrontMenu() {
	if (ALL.size() <= 0)
		return false;
	return ALL[ALL.size()-1] == this;
}

void rui::Menu::exit(){
	menuInputEnabled = false;
	sigExit.trigger();
}

bool rui::Menu::acceptsInput(){
	return 
		isFrontMenu()
	&&	yieldControls <= 0 
	&&	menuInputEnabled 
	&&	life >= 10;
}

void rui::Menu::simClick() {
	auto focs = getFocusables();
	IInteractive* cur = nullptr;
	if (currentFocus >= 0 && currentFocus < focs.size()) 
	cur = focs[currentFocus];
	if (cur) {
		cur->doClick();
		return;
	}

	for (auto b : buttons) 
		if (b->isFocused()) 
			b->doClick();
}

void rui::Menu::simBack() {
	rs::Timer::delay(1, [=]() { destroy(); });
}

void rui::Menu::doControls(){
	if (( flags & ALLOW_LEFT) && rs::Input::isDirJustPressed_Left())
		focusLeft();
	else	if ((flags & ALLOW_UP) && rs::Input::isDirJustPressed_Up())
		focusUp();
	else	if ((flags & ALLOW_RIGHT) && rs::Input::isDirJustPressed_Right())
		focusRight();
	else	if ((flags & ALLOW_DOWN) && rs::Input::isDirJustPressed_Down())
		focusDown();

	if ((flags & ALLOW_OK)) {
		if (
			rs::Input::isKeyboardKeyJustPressed(Key::KB_ENTER)
			||rs::Input::isKeyboardKeyJustPressed(Key::KB_NUMPAD_ENTER)
			||rs::Input::isAnyPadJustPressed(Key::PAD_CROSS)) {
			simClick();
		}
	}

	if ((flags & ALLOW_ESC)) {
		if (rs::Input::isKeyboardKeyJustPressed(Key::KB_ESC)) {
			simBack();
		}
	}
}


static std::vector<IInteractive*> getFocusablesOf(std::vector<IInteractive*>& src) {
	std::vector<IInteractive*> focs;
	for (IInteractive* ii : src) {
		if (!ii) continue;
		if (ii->isGreyed())continue;
		if (!ii->isVisible())continue;
		focs.push_back(ii);
	}
	return focs;
}


void rui::Menu::focusUp() {
	auto focs = getFocusables();
	IInteractive* cur = nullptr;

	if (currentFocus >= 0 && currentFocus < focs.size()) 
		cur = focs[currentFocus];

	if (cur == nullptr) {
		auto ups = getFocusablesOf(upDownOrder);
		blurAll();
		if (ups.size()) 
			grantFocus(ups[0]);
		else if (focs.size())
			grantFocus(focs[0]);
		return;
	}
	else {
		auto & seekIn = upDownOrder;
		auto pos = std::find(seekIn.rbegin(), seekIn.rend(), cur);
		if (pos == seekIn.rend()) //already finished, stay locked
			return;
		auto next = pos + 1;
		if (next == seekIn.rend())
			return;
		do {
			if (isFocusable(*next)) {
				blurAll();
				grantFocus(*next);
				return;
			}
			next++;
		} while (next != seekIn.rend());
	}
}

void rui::Menu::focusRight(){
	auto focs = getFocusables();
	IInteractive* cur = nullptr;
	if (currentFocus >= 0 && currentFocus < focs.size()) {
		cur = focs[currentFocus];
	}
	if (cur == nullptr) {
		blurAll();
		if (focs.size()) grantFocus(focs[0]);
		return;
	}
	else {
		auto & seekIn = leftRightOrder;
		auto pos = std::find(seekIn.begin(), seekIn.end(), cur);
		auto next = pos + 1;
		if (pos == seekIn.end()) //already finished, stay locked
			return;
		do {
			if (isFocusable(*next)) {
				blurAll();
				grantFocus(*next);
				return;
			}
			next++;
		} while (next != seekIn.end());
	}
}


void rui::Menu::focusLeft() {
	auto focs = getFocusables();
	IInteractive* cur = nullptr;
	if (currentFocus >= 0 && currentFocus < focs.size()) {
		cur = focs[currentFocus];
	}
	if (cur == nullptr) {
		blurAll();
		if (focs.size()) grantFocus(focs[0]);
		return;
	}
	else {
		auto& seekIn = leftRightOrder;
		auto pos = std::find(seekIn.rbegin(), seekIn.rend(), cur);
		if (pos == seekIn.rend()) //already finished, stay locked
			return;
		auto next = pos + 1;
		if (next == seekIn.rend())
			return;
		do {
			if (isFocusable(*next)) {
				blurAll();
				grantFocus(*next);
				return;
			}
			next = pos + 1;
		} while (next != seekIn.rend());
	}
}

void rui::Menu::focusDown(){
	auto focs = getFocusables();
	IInteractive* cur = nullptr;
	if (currentFocus >= 0 && currentFocus < focs.size()) {
		cur = focs[currentFocus];
	}
	if (cur == nullptr) {
		blurAll();
		if (focs.size()) grantFocus(focs[0]);
		return;
	}
	else {
		auto & seekIn = upDownOrder;
		auto pos = std::find(seekIn.begin(), seekIn.end(), cur);
		if (pos == seekIn.end()) //already finished, stay locked
			return;
		auto next = pos + 1;
		if (next == seekIn.end()) 
			return;
		do {
			if (isFocusable(*next)) {
				blurAll();
				grantFocus(*next);
				return;
			}
			else
				next++;
		} while (next != seekIn.end());
	}
}

void rui::Menu::blur(IInteractive * b){
	b->onFocusLost();
}

void rui::Menu::blurAll(){
	for (IInteractive * b : buttons) 
		if (b->isFocused()) {
			b->onFocusLost();
		}
}

bool rui::Menu::isFocusable(IInteractive*inter){
	auto focs = getFocusables();
	auto pos = std::find(focs.begin(), focs.end(), inter);
	if (pos == focs.end())
		return false;
	return true;
}

void rui::Menu::grantFocus(IInteractive * inter)
{
	if (!inter) {
		currentFocus = -1;
		return;
	}
	auto focs = getFocusables();
	auto pos = std::find(focs.begin(), focs.end(), inter);
	if (pos==focs.end())
		return;
	currentFocus = std::distance(focs.begin(), pos);
	inter->setFocused(true);
}

IInteractive* rui::Menu::getFocused() {
	return getFocusables()[currentFocus];
}

void rui::Menu::resetNavigation(){
	buttons.clear();
}

void rui::Menu::generateNavigation() {
	std::vector<IInteractive*> sorter = getFocusables();// getFocusables();

	std::stable_sort(sorter.begin(), sorter.end(), [](IInteractive* a, IInteractive* b) {
		if (a->getPosition().x < b->getPosition().x)
			return true;
		else if (a->getPosition().x >= b->getPosition().x)
			return false;
		//ax == bx
		else if (a->getPosition().y < b->getPosition().y)
			return true;
		else if (a->getPosition().y >= b->getPosition().y)
			return false;
		else
			return false;
		});
	leftRightOrder = sorter;

	std::stable_sort(sorter.begin(), sorter.end(), [](IInteractive* a, IInteractive* b) {
		if (a->getPosition().y < b->getPosition().y)
			return true;
		else if (a->getPosition().y >= b->getPosition().y)
			return false;
		else if (a->getPosition().x < b->getPosition().x)
			return true;
		else if (a->getPosition().x >= b->getPosition().x)
			return false;
		else
			return true;
		});
	upDownOrder = sorter;
}

void rui::Menu::startNavigation(int idx){
	generateNavigation();

	auto focs = getFocusables();
	if(idx<focs.size())
		grantFocus(focs[idx]);

	navDirty = false;
}

r2::Interact* rui::Menu::interactBlock(){
	if (!blocker) {
		blocker = new r2::Interact(rs::Display::width(), rs::Display::height(), parent);
		blocker->name = std::string("InteractBlocker#") + std::to_string(uid);
		if( parent) parent->setChildIndex(blocker,parent->getChildIndex(this));
	}
	return blocker;
}

void rui::Menu::onExitHierarchy() {
	SUPER::onExitHierarchy();
}

void rui::Menu::onEnterHierarchy() {//coud
	SUPER::onEnterHierarchy();
}

void rui::Menu::armaggeddon(){
	for (int i = ALL.size() - 1; i >= 0; i--) 
		delete( ALL[i] );
	ALL.clear();
}

std::vector<IInteractive*> rui::Menu::getFocusables(){
	std::vector<IInteractive*> focs;
	for (IInteractive * ii : buttons) {
		if (!ii) continue;
		if( ii->isGreyed() )continue;
		if (!ii->isVisible())continue;
		focs.push_back(ii);
	}
	return focs;
}

void rui::Menu::im(){
	using namespace ImGui;
	if (CollapsingHeader("Menu")) {
		Indent();
		if (TreeNode("Navigation")) {
			if (TreeNode("Left Right Order")) {
				for (auto& e: leftRightOrder) {
					auto* n = dynamic_cast<Node*>(e);
					if (n) {
						r2::Im::imNodeListEntry(0,n);
						if (IsItemHovered())
							r2::Im::bounds(n);
					}
					else 
						ImGui::Text("Unable to inspect node");
				}
				TreePop();
			}

			if (TreeNode("Up Down Order")) {
				for (auto& e : upDownOrder) {
					auto* n = dynamic_cast<Node*>(e);
					if (n) {
						r2::Im::imNodeListEntry(0,n);
						if (IsItemHovered())
							r2::Im::bounds(n);
					}
					else
						ImGui::Text("Unable to inspect node");
				}
				TreePop();
			}

			if(ImGui::Button("Restart Navigation")) {
				startNavigation(0);
			}

			if (TreeNode("Button list")) {
				for (auto& e : buttons) {
					auto* n = dynamic_cast<Node*>(e);
					if (n) {
						r2::Im::imNodeListEntry(0, n);
						if (IsItemHovered())
							r2::Im::bounds(n);
					}
					else
						ImGui::Text("Unable to inspect node");
				}
				TreePop();
			}
			TreePop();
		}
		if (TreeNode("Values")) {
			bool ai = acceptsInput();
			Value("accepts input", ai);
			Value("yieldControls", yieldControls);
			Value("life", life);
			Value("currentFocus", currentFocus);
			r2::Im::imNodeListEntry("blocker", blocker);
			TreePop();
		}
		if (TreeNode("Tw")) {
			r2::Im::imTw(&tw);
			TreePop();
		}

		Unindent();
	}
	SUPER::im();
}
