#include "stdafx.h"

#include "rui/IInteractive.hpp"
#include "rui/Menu.hpp"
#include "rui/IController.hpp"

using namespace std;
using namespace rd;
using namespace rui;

eastl::vector<rui::Menu*> rui::Menu::ALL;

rui::Menu::Menu( const char* _name, r2::Node * parent ) : Super(parent){
	ALL.push_back(this);
	name = _name;
	al.push_back(&tw);
	navigation.menu = this;
}

rui::Menu::~Menu() {
	inputBhv.dispose();

	dispose();
}

void rui::Menu::update(double dt) {
	Super::update(dt);

	tw.update(dt);
	al.update(dt);

	if (autoNav && navDirty)
		startNavigation(currentFocus);

	if (acceptsInput()) {
		doControls();
		inputBhv.update(dt);
	}

	yieldControls--;
	yieldControlsSec -= dt;
	life++;
}

void rui::Menu::dispose() {
	//trace("disposing "+ name);

	if (disposed) {
		//trace("Menu - "s +name+ " - disposed twice, I don't think it's gonna be okay, menu are not meant to be reused");
		return;
	}

	//okay give it away anyway
	tw.dispose();
	al.dispose();

	if (blocker) {
		blocker->destroy();
		blocker = 0;
	}
	disposed = true;
	menuInputEnabled = false;

	auto pos = std::find(ALL.begin(), ALL.end(), this);
	if(pos!=ALL.end())
		ALL.erase(pos);

	Super::dispose();
}

bool rui::Menu::isFrontMenu() {
	if (ALL.size() <= 0)
		return false;
	return ALL[ALL.size()-1] == this;
}

void rui::Menu::exit(){
	yieldHierarchy();
	trace("rui::Menu::exit"s + name.c_str());
	menuInputEnabled = false;
	sigExit.trigger();
}

bool rui::Menu::shouldNavigateGreys() const
{
	return useNewNavigation && (navigation.filterOutGreyNodes == false);
}

bool rui::Menu::acceptsInput(){
	return 
		isFrontMenu()
	&&	!disposed
	&&	yieldControls <= 0 
	&&	yieldControlsSec <= 0
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
	yieldHierarchy();
	sigBack();
	rs::Timer::delay(1, [=]() { 
		destroy(); 
	});
}

void rui::Menu::doControls(){
	if (controlOverride) {
		controlOverride->execute();
		return;
	}

	bool focLeft = false;
	bool focRight = false;
	bool focUp = false;
	bool focDown = false;
	if (flags & ALLOW_DPAD) {
		if ((flags & ALLOW_LEFT) && rs::Input::isDirJustPressed_Left())
			focLeft = true;
		else	if ((flags & ALLOW_UP) && rs::Input::isDirJustPressed_Up())
			focUp = true;
		else	if ((flags & ALLOW_RIGHT) && rs::Input::isDirJustPressed_Right())
			focRight = true;
		else	if ((flags & ALLOW_DOWN) && rs::Input::isDirJustPressed_Down())
			focDown = true;
	}

	auto test = rs::Input::isAnyPadJustPressed;

	if( (flags & ALLOW_LEFT_STICK) && acceptsInput() ) {
		if ((flags & ALLOW_DOWN)	&& test( Key::PAD_STICK_LEFT_Y_NEG)) focDown = true;
		if ((flags & ALLOW_UP)		&& test( Key::PAD_STICK_LEFT_Y_POS)) focUp = true;
		if( (flags & ALLOW_LEFT)	&& test( Key::PAD_STICK_LEFT_X_NEG)) focLeft = true;
		if ((flags & ALLOW_RIGHT)	&& test( Key::PAD_STICK_LEFT_X_POS)) focRight = true;
	}

	if ((flags & ALLOW_RIGHT_STICK) && acceptsInput()) {
		if ((flags & ALLOW_DOWN)	&& test(Key::PAD_STICK_RIGHT_Y_NEG)) focDown = true;
		if ((flags & ALLOW_UP)		&& test(Key::PAD_STICK_RIGHT_Y_POS)) focUp = true;
		if ((flags & ALLOW_LEFT)	&& test(Key::PAD_STICK_RIGHT_X_NEG)) focLeft = true;
		if ((flags & ALLOW_RIGHT)	&& test(Key::PAD_STICK_RIGHT_X_POS)) focRight = true;
	}

	if (focDown)							{ focusDown();	 }
	else if (focUp && acceptsInput())		{ focusUp();	 }
	else if (focLeft && acceptsInput())		{ focusLeft();	 }
	else if (focRight && acceptsInput())	{ focusRight();	 }

	if ((flags & ALLOW_OK)) {
		if (
			rs::Input::isKeyboardKeyJustPressed(Key::KB_ENTER)
			||rs::Input::isKeyboardKeyJustPressed(Key::KB_NUMPAD_ENTER)
			||rs::Input::isAnyPadJustPressed(Key::PAD_CROSS)) {
			simClick();
			yieldHierarchy();
		}
	}

	if ((flags & ALLOW_ESC)) {
		if (rs::Input::isKeyboardKeyJustPressed(Key::KB_ESC)) {
			simBack();
			yieldHierarchy();
		}
	}

	if (controlAddition) controlAddition->execute();
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
	if (useNewNavigation) {
		newFocusUp();
		return;
	}
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
		if (seekIn.empty()) return;
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
	if (useNewNavigation) {
		newFocusRight();
		return;
	}
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
		if (seekIn.empty()) return;
		auto pos = std::find(seekIn.begin(), seekIn.end(), cur);
		auto next = pos + 1;
		if (next == seekIn.end()) //already finished, stay locked
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

void rui::Menu::newFocusLeft() {
	IInteractive* cur = getFocused();
	if (cur == nullptr) defaultFocus();
	else
		newFocus(LEFT);
}

void rui::Menu::newFocusUp() {
	IInteractive* cur = getFocused();
	if (cur == nullptr) defaultFocus();
	else
		newFocus(UP);
}

void rui::Menu::newFocusRight() {
	IInteractive* cur = getFocused();
	if (cur == nullptr) defaultFocus();
	else
		newFocus(RIGHT);
}

rui::IInteractive* rui::NavPointer::follow(rd::Dir dir) {
	switch (dir)
	{
	case UP:
		return up;
		break;
	case DOWN:
		return down;
		break;
	case LEFT:
		return left;
		break;
	case RIGHT:
		return right;
		break;
	default:
		break;
	}
	return right;
}


rui::IInteractive* rui::Menu::follow(rui::IInteractive* elem, rd::Dir dir) {
	if (navigation.data.find(elem) == navigation.data.end())
		return 0;
	return navigation.data[elem].follow(dir);
}

void rui::Menu::defaultFocus() {
	blurAll();
	auto focs = getFocusables();
	if (focs.size()) grantFocus(focs[0]);
}

rui::Menu* rui::Menu::getMenu(r2::Node* entry) {
	while (entry) {
		rui::Menu* asMenu = dynamic_cast<rui::Menu*>(entry);
		if (asMenu)
			return asMenu;
		entry = entry->parent;
	}
	return 0;
}

//todo have a way to not "continue" 
void rui::Menu::newFocus(rd::Dir dir) {
	IInteractive* cur = getFocused();
	if (navigation.data.find(cur) == navigation.data.end())
		return;
	auto next = follow(cur, dir);
	while (next && !isFocusable(next)) {
		auto future = follow(cur, dir);
		if (future == next)
			break;
	}
	if (next) {
		blurAll();
		grantFocus(next);
	}
}

void rui::Menu::newFocusDown() {
	IInteractive* cur = getFocused();
	if (cur == nullptr) defaultFocus();
	else
		newFocus(DOWN);
}

void rui::Menu::focusLeft() {
	if (useNewNavigation) {
		newFocusLeft();
		return;
	}
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
	if (useNewNavigation) {
		newFocusDown();
		return;
	}
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

void rui::Menu::defaultYield(){
	yieldControlsSec = 0.15f;
}

void rui::Menu::yieldHierarchy(){
	rui::Menu* cur = this;
	while(cur){
		rui::Menu* ocur = cur;
		cur->defaultYield();
		auto par = cur->parent;
		while( par ){
			rui::Menu* asM = dynamic_cast<rui::Menu*>(par);
			if (asM) {
				break;
				cur = asM;
			}
			par = par->parent;
		}
		if (cur == ocur)
			break;
	}
}

void rui::Menu::blur(IInteractive* inter) {
	inter->setFocused(false);
}

void rui::Menu::blurAll(){
	for (IInteractive* b : buttons)
		blur(b);
}

bool rui::Menu::isFocusable(IInteractive*inter){
	auto focs = getFocusables();
	auto pos = std::find(focs.begin(), focs.end(), inter);
	if (pos == focs.end())
		return false;
	return true;
}

void rui::Menu::grantFocus(IInteractive * inter){
	if (!inter) {
		currentFocus = -1;
		return;
	}
	auto focs = getFocusables();
	auto pos = std::find(focs.begin(), focs.end(), inter);
	if (pos == focs.end()) {
		sigFocusFailed();
		return;
	}
	currentFocus = std::distance(focs.begin(), pos);
	inter->setFocused(true);
	sigFocusGranted();
}

IInteractive* rui::Menu::getFocused() {
	if (currentFocus < 0) return nullptr;
	return getFocusables()[currentFocus];
}

void rui::Menu::resetNavigation(){
	buttons.clear();
}

void rui::Menu::generateSimpleNavigation() {
	std::vector<IInteractive*> sorter = getFocusables();// getFocusables();
	std::stable_sort(sorter.begin(), sorter.end(), [](IInteractive* a, IInteractive* b) {
		auto aPos = a->getAbsolutePosition();
		auto bPos = b->getAbsolutePosition();

		if (aPos.x < bPos.x)
			return true;
		else if (aPos.x >= bPos.x)
			return false;
		//ax == bx
		else if (aPos.y < bPos.y)
			return true;
		else if (aPos.y >= bPos.y)
			return false;
		else
			return false;
		});
	leftRightOrder = sorter;
	std::stable_sort(sorter.begin(), sorter.end(), [](IInteractive* a, IInteractive* b) {
		auto aPos = a->getAbsolutePosition();
		auto bPos = b->getAbsolutePosition();
		if (aPos.y < bPos.y)
			return true;
		else if (aPos.y >= bPos.y)
			return false;
		else if (aPos.x < bPos.x)
			return true;
		else if (aPos.x >= bPos.x)
			return false;
		else
			return true;
		});
	upDownOrder = sorter;
	navDirty = false;
}

void rui::Menu::generateNavigation() {
	syncAllMatrix();
	if (useNewNavigation) {
		navigation.generate();
		navDirty = false;
		return;
	}
	generateSimpleNavigation();
}

int rui::Menu::getFocusableIdx(IInteractive* e) {
	auto focs = getFocusables();
	int idx = 0;
	for (auto cur : focs){
		if (e == cur)
			return idx;
		idx++;
	}
	return -1;
}

void rui::Menu::startNavigation(int idx){
	if (navDirty) 
		generateNavigation();

	auto focs = getFocusables();
	if(idx<focs.size())
		grantFocus(focs[idx]);

	navDirty = false;
}

r2::Interact* rui::Menu::interactBlock(){
	if (!blocker) {
		blocker = new r2::Interact(parent);
		blocker->boundlessInteract = true;
		blocker->name = std::string("InteractBlocker#") + std::to_string(uid);
		if( parent) parent->setChildIndex(blocker,parent->getChildIndex(this));
	}
	return blocker;
}

void rui::Menu::onExitHierarchy() {
	Super::onExitHierarchy();
}

void rui::Menu::onEnterHierarchy() {//coud
	Super::onEnterHierarchy();
}

void rui::Menu::armaggeddon(){
	for (int i = ALL.size() - 1; i >= 0; i--) {
		if (ALL[i]) {
			if( !ALL[i]->destructionInProgress )
				delete(ALL[i]);
		}
	}
	ALL.clear();
}

eastl::vector<IInteractive*> rui::Menu::getButtons(){
	return buttons;
}

std::vector<IInteractive*> rui::Menu::getFocusables(){

	bool navigateGreys = shouldNavigateGreys();
	std::vector<IInteractive*> focs;
	for (IInteractive * ii : buttons) {
		if (!ii) continue;
		if(!navigateGreys)
			if( ii->isGreyed() )continue;
		if (!ii->isVisible())continue;
		focs.push_back(ii);
	}
	return focs;
}

#ifdef _DEBUG
void rui::Menu::log(const char* str) {
	std::cout << str << " \n";
}
#endif

void rui::Menu::im(){
	using namespace ImGui;
	if (CollapsingHeader("Menu")) {
		Indent();

		if (TreeNode("Focused")) {
			DragInt("current", &currentFocus);
			auto asNode = dynamic_cast<r2::Node*>(getFocused());
			r2::Im::nodeButton(asNode,"focused");
			TreePop();
		}
		if (TreeNode("Navigation")) {
			Checkbox("Use new navigation", &useNewNavigation);
			if (useNewNavigation) {
				Text("New Navigation");
				Checkbox("show nav", &showNavigation);
				navigation.im();
			}
			else {
				Checkbox("show nav", &showNavigation);
				if (showNavigation) {
					auto idx = 0;
					IInteractive* next = 0;
					for (auto& e : leftRightOrder) {
						if (idx < leftRightOrder.size() - 1)
							next = leftRightOrder[idx + 1];
						else
							next = 0;
						auto* nCur = dynamic_cast<Node*>(e);
						auto* nNext = dynamic_cast<Node*>(next);
						if (nCur && next) {
							auto l = r2::Im::arrow(nCur->getBounds(this).getCenter(), nNext->getBounds(this).getCenter(), this);
							l->setColor(r::Color::Red);
						}
						idx++;
					}

					for (auto& e : leftRightOrder) {
						r2::Im::cross(e->getAbsolutePosition(), 6, getScene());
					}
				}

				if (TreeNode("Left Right Order")) {
					for (auto& e : leftRightOrder) {
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

				if (TreeNode("Up Down Order")) {
					for (auto& e : upDownOrder) {
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

				if (ImGui::Button("Generate simple nav")) {
					navDirty = true;
					generateSimpleNavigation();
				}

				if (ImGui::Button("Restart nav")) {
					startNavigation(0);
				}
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
			Value("is front", isFrontMenu());
			Checkbox("input enabled", &menuInputEnabled);
			Value("accepts input", ai);
			Value("yieldControlsSec", yieldControlsSec);
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
	Super::im();
}

void rui::Menu::setAsFrontMenu() {
	rs::Std::remove(ALL, this);
	ALL.push_back(this);
}

rui::Menu* rui::Menu::findFirstMenu(r2::Node* cur){
	if (!cur) return 0;
	auto pAsMenu = dynamic_cast<rui::Menu*>(cur);
	if (pAsMenu)
		return pAsMenu;
	else
		return findFirstMenu(cur->parent);
}

rui::Menu* rui::Menu::getParentMenu(){
	return findFirstMenu(parent);
}

/**
* You may want to add a onDispose( removeButton )
*/
bool rui::Menu::addButton(IInteractive* i) {
	if (!i)
		return false; 
	if (rs::Std::exists(buttons, i))
		return false;
	buttons.push_back(i);
	if (autoNav) 
		navDirty = true;
	trsDirty = true;
	return true;
}

void rui::Menu::removeButton(IInteractive* i) {
	rs::Std::remove(buttons,i);
	rs::Std::remove(leftRightOrder, i);
	rs::Std::remove(upDownOrder, i);
	if (useNewNavigation)
		navigation.remove(i);
	if (autoNav) navDirty = true;
}


void rui::Navigation::unlink(rui::IInteractive* a, rui::IInteractive* b, rd::Dir dir) {
	switch (dir) {
		case LEFT:
			data[a].left = 0;
			data[b].right = 0;
			break;
		case RIGHT:
			data[a].right = 0;
			data[b].left = 0;
			break;
		case DOWN:
			data[a].down = 0;
			data[b].up = 0;
			break;
		case UP:
			data[a].up = 0;
			data[b].down = 0;
			break;
	}
}

void rui::Navigation::linkIfCloser(rui::IInteractive* a, rui::IInteractive* b, rd::Dir dir, bool biDir){
	data[a].nav = data[b].nav = this;
	data[a].owner = a;
	data[b].owner = b;
	auto pos = a->getAbsolutePosition();
	auto oldB = data[a].follow(dir);
	if (oldB == b) {
		//trace("link exists");
		return;
	}
	if (!oldB) {
		//trace("link!");
		link(a, b, dir, biDir);
		return;
	}
	auto nbPos = b->getAbsolutePosition();
	auto obPos = oldB->getAbsolutePosition();
	if ((nbPos - pos).getNormSquared() < (obPos - pos).getNormSquared()) {
		//unlink(a, oldB, dir);
		link(a, b, dir, false);
		//trace("shortening link!");
	}
}

void rui::Navigation::link(rui::IInteractive* a, rui::IInteractive* b, rd::Dir dir, bool biDir ){
	data[a].nav = data[b].nav = this;
	data[a].owner = a;
	data[b].owner = b;
	switch (dir){
		case UP: data[a].up = b; if (biDir)  data[b].down = a;
			break;
		case DOWN: data[a].down = b; if (biDir)  data[b].up = a;
			break;
		case LEFT: data[a].left = b; if (biDir)  data[b].right = a;
			break;
		case RIGHT: data[a].right = b; if (biDir)  data[b].left = a;
			break;
	
		default:
			break;
	}
}

rd::Dir rui::Navigation::extractDirFromTo(const Vector2& from, const Vector2& to) {
	Vector2 diff = to - from;
	float angle = fmod(atan2(diff.y, diff.x), 2 * PASTA_PI);
	while (angle < 0) angle += 2 * PASTA_PI;
	while (angle >= 2 * PASTA_PI) angle -= 2 * PASTA_PI;

	if (angle < PASTA_PI * 0.25)
		return RIGHT;
	else if (angle < PASTA_PI * 0.75)
		return DOWN;
	else if (angle < PASTA_PI * 1.25)
		return LEFT;
	else if (angle > PASTA_PI * 1.75)
		return RIGHT;
	else
		return UP;
}

template<typename Elem>
void eastlFilter(eastl::vector<Elem>& dst, eastl::vector<Elem>& src, std::function<bool(Elem elem)> f) {
	dst.clear();
	if (src.size() > dst.size())
		dst.reserve(src.size());
	for (auto it = src.begin(); it != src.end(); it++)
		if (f(*it))
			dst.push_back(*it);
};

void rui::Navigation::generate(){
	trace("rui::Navigation::generate");
	data.clear();

	if (!menu) return;
	
	eastl::vector<rui::IInteractive*> bts;
	if (!filterOutGreyNodes)
		bts = menu->buttons;
	else {
		std::function<bool(rui::IInteractive*)> fl = [](rui::IInteractive* bt)->bool {
			return !bt->isGreyed();
		};
		rs::Std::filter(bts, menu->buttons, fl);
	}

	for(int i = 0; i < bts.size(); ++i){
		auto cur = bts[i];
		r2::Node* curNode = dynamic_cast<r2::Node*>(cur);
		for (int j = 0; j < bts.size(); ++j) {//brute force likely rd::Dir
			auto tgt = bts[j];
			if( tgt == cur )
				continue;
			r2::Node* tgtNode = dynamic_cast<r2::Node*>(tgt);
			auto cp = cur->getAbsolutePosition();
			auto tp = tgt->getAbsolutePosition();
			rd::Dir dir = extractDirFromTo(cp,tp);
			linkIfCloser(cur, tgt, dir, false);//update if needed
		}
	}

	//fine tune for unnatural duplicates
	for (int i = 0; i < bts.size(); ++i) {
		auto& n = data[bts[i]];
		if (n.up == n.left)
			n.up = 0;
		if (n.up == n.right)
			n.up = 0;
		if (n.down == n.left)
			n.down = 0;
		if (n.down == n.right)
			n.down = 0;
	}
}

void rui::NavPointer::im(rui::IInteractive* elem,const char * prefix){
	using namespace ImGui;

	if (!elem) {
		Text("%s : no value", prefix);
		return;
	}

	auto asNode = dynamic_cast<r2::Node*>(elem);
	if (asNode) 
		r2::Im::nodeButton(asNode, prefix);
	else
		Text("%s : Unknown interactive 0x%x", prefix,elem);
}

void rui::NavPointer::im() {
	using namespace ImGui;

	ImGui::BeginGroup();
	im(owner, "owner");
	im(left, "left");
	im(up, "up");
	im(right, "right");
	im(down, "down");
	ImGui::EndGroup();

	if(nav&&nav->menu)
	if (IsItemHovered()) 
		for (auto t : { left,up,right,down }) {
			if (!t) continue;
			r2::Node* on = dynamic_cast<r2::Node*>(owner);
			r2::Node* tn = dynamic_cast<r2::Node*>(t);
			if( on&&tn){
				auto sc = nav->menu->getScene();
				r2::Im::arrow( on->getBounds(sc).getCenter(), tn->getBounds(sc).getCenter(), sc);
			}
			else {
				auto oc = owner->getAbsolutePosition() + owner->getSize();
				auto tc = t->getAbsolutePosition() + t->getSize();
				r2::Im::arrow(oc, tc, nav->menu->getScene());
			}
		}
}

void rui::NavPointer::remove(rui::IInteractive* i){
	if (left == i) left = 0;
	if (right == i) right = 0;
	if (up == i) up = 0;
	if (down == i) down = 0;
}

void rui::Navigation::im(){
	using namespace ImGui;
	Checkbox("filter greyed", &filterOutGreyNodes);
	if (ImGui::Button("Generate Navigation")) 
		generate();
	int idx = 0;
	for (auto& kv : data) {
		auto it = kv.first;
		r2::Node* n = dynamic_cast<r2::Node*>(it);
		Str v;
		v.setf("%d", idx);
		if (n) 
			v.setf("%d %s", idx, n->name.c_str());
		if (it && TreeNode(v.c_str())) {
			kv.second.im();
			TreePop();
		}
		if (IsItemHovered())
			r2::Im::bounds(n);
		idx++;
	}
}

void rui::Navigation::remove(rui::IInteractive* a){
	for(auto& kv : data)
		kv.second.remove(a);
	data.erase(a);
}

void Menu::show(){
	visible = true;
}

void Menu::hide() {
	visible = false;
}