#include "stdafx.h"

#include "JSerialize.hpp"
#include "Style.hpp"
#include "ri18n/T.hpp"

using namespace r;
using namespace std::literals;

static std::unordered_map<Str, bool> supportedState = {
	{"active",true},
	{"hovered",true},
	{"focused",true},
	{"greyed",true},
};

static std::unordered_map<Str,Type> supported = {
	{"position",			Type::Vec4},
	{"x",					Type::Float},
	{"y",					Type::Float},
	{"z",					Type::Float},
	{"color",				Type::Color},
	{"margin",				Type::Vec4i},
	{"padding",				Type::Vec4i},
	{"offset",				Type::Vec4i},
	{"font-size",			Type::Float},
	{"font-autosize",		Type::Vec4i},
	{"font-autosize-max-width",			Type::Int},
	{"font-autosize-target-font-size",	Type::Int},
	{"font-autosize-min-font-size",		Type::Int},
	{"font-family",			Type::String},
	{"background-color",	Type::Color},

	{"border",				Type::Vars},
	{"border-color",		Type::Color},

	{"layout",				Type::String},
	{"vertical-spacing",	Type::Float},
	{"horizontal-spacing",	Type::Float},
	{"drop-shadow",			Type::Vars},
	{"drop-shadow-x",		Type::Float},
	{"drop-shadow-y",		Type::Float},
	{"drop-shadow-color",	Type::Color},
	{"outline",				Type::Vars},
	{"outline-dist",		Type::Float},
	{"outline-color",		Type::Color},
	{"text-transform",		Type::Enum},
	{"width",				Type::Float},
	{"max-width",			Type::Float},
	{"height",				Type::Float},
};

rd::Style::Style(const Style& s){
	vals = s.vals;			parent = s.parent;
	onChanged();
}

rd::Style& rd::Style::operator=(const Style& s){
	vals = s.vals;			parent = s.parent;
	onChanged();
	return  *this;
}

void rd::Style::operator+=(const Style& s){
	vals.merge(s.vals);
	onChanged();
}

bool rd::Style::isEmpty() const {
	return vals.isEmpty();
}


rd::Style::Style() {
	name = Str( "Anonymous style "s + std::to_string( rd::Rand::get().random(0x7fffff)) );
	dirty = false;
	onChanged.add([this]() { dirty = true; });
}


rd::Style::Style(const char* str) : Style(){
	auto lits = rd::String::split(str, ";");
	for (auto l : lits)
		parseLit(l.c_str());
}

rd::Style::~Style() {
	vals.dispose();
	onChanged.clear();
	parent = nullptr;
}

bool rd::Style::has(const Str& name) const {
	return has(name.c_str());
}

bool rd::Style::has(const char* name) const{
	Str query = name;
	const char* subLeft = strstr(name, "-left");		if (subLeft) return has(rd::String::sub(name, subLeft));
	const char* subRight = strstr(name, "-right");		if (subRight) return has(rd::String::sub(name, subRight));
	const char* subTop = strstr(name, "-top");			if (subTop) return has(rd::String::sub(name, subTop));
	const char* subBottom = strstr(name, "-bottom");	if (subBottom) return has(rd::String::sub(name, subBottom));
	if(vals.has(name))
		return true;
	if (parent && parent->has(name))
		return true;
	return false;
}

void rd::Style::parseLit(const char * l) {
	auto chunks = rd::String::split(l, ":");
	vals.setUntyped(chunks[0].c_str(),chunks[1].c_str());
}

void rd::Style::ensure(const char* label){
#ifdef _DEBUG
	if (!_nonNativeSupportWarned) {
		Str tested = label;
		if (rd::String::contains(tested, ":")) {
			auto semicolon = strstr(label, ":");
			tested = rd::String::sub(label, semicolon);
		}
		if (!rs::Std::exists(supported, tested))
			traceWarning(std::string("Style : beware does not natively support ") + label);
		_nonNativeSupportWarned = true;
	}
#endif
}

void rd::Style::operator()(const char* label, int val){
	ensure(label);
	if (StrRef("x") == label) {
		auto pos = getPosition();
		pos.x = val;
		setPosition(pos);
		return;
	}
	if (StrRef("y") == label) {
		auto pos = getPosition();
		pos.y = val;
		setPosition(pos);
		return;
	}
	if (StrRef("z") == label) {
		auto pos = getPosition();
		pos.z = val;
		setPosition(pos);
		return;
	}
	vals.set(label, val);
	onChanged();
}

void rd::Style::operator()(const char* label, float val) {
	ensure(label);
	if (StrRef("x") == label) {
		auto pos = getPosition();
		pos.x = val;
		setPosition(pos);
		return;
	}
	if (StrRef("y") == label) {
		auto pos = getPosition();
		pos.y = val;
		setPosition(pos);
		return;
	}
	if (StrRef("z") == label) {
		auto pos = getPosition();
		pos.z = val;
		setPosition(pos);
		return;
	}
	vals.set(label, val);
	onChanged();
}

void rd::Style::operator()(const char* label, const char * val) {
	ensure(label);
	vals.set(label, val);
	onChanged();
}

void rd::Style::operator()(const char* label, const r::Color& val) {
	ensure(label);
	vals.set(label, val);
	onChanged();
}


void rd::Style::operator()(const char* label, const Vector2i& val) {
	ensure(label);
	vals.set(label, val);
	onChanged();
}

void rd::Style::operator()(const char* label, const Vector4i& val) {
	ensure(label);
	vals.set(label, val);
	onChanged();
}

float rd::Style::getFloat(const char* name) const
{
	if (vals.has(name))
		return vals.getFloat(name);
	else if (parent)
		return parent->getFloat(name);
	else
		return 0.0f;
}

int rd::Style::getInt(const char* name)  const {
	if (vals.has(name))
		return vals.getInt(name);
	else if (parent)
		return parent->getInt(name);
	else
		return 0;
}

Vector2 rd::Style::getVec2(const char* name) const{
	if (!vals.has(name))
		return {};
	auto _v = vals.getVec2(name);
	if (!_v) return {};
	auto v = *_v;
	return Vector2(v.x, v.y);
}

Vector3 rd::Style::getVec3(const char* name) const {
	if (!vals.has(name))
		return {};
	auto _v = vals.getVec3(name);
	if (!_v) return {};
	auto v = *_v;
	return Vector3(v.x, v.y,v.z);
}

r::Color rd::Style::getColor()  const {
	return getColor("color");
}

r::Color rd::Style::getColor(const char* name)  const {
	if (vals.has(name))
		return vals.getColor(name);
	else if (parent)
		return parent->getColor(name);
	else
		return r::Color();
}

Vector4i rd::Style::getVec4i(const char* name)  const {
	if (vals.has(name))
		return *vals.getVec4i(name);
	else if (parent)
		return parent->getVec4i(name);
	else
		return Vector4i();
}

const char* rd::Style::getString(const char* name)  const {
	if (vals.has(name))
		return vals.getString(name);
	else if (parent)
		return parent->getString(name);
	else
		return "";
}

int rd::Style::getFontSize() const {
	return vals.getInt("font-size", 24);
}

void rd::Style::setFontFamily(rd::Font* val) {
	this->operator()("font-family", rd::FontManager::get().getFontName(val));
};

rd::Font* rd::Style::getFont(const char* name) const {
	return rd::FontManager::get().getFont(getString(name));
}

rd::Font* rd::Style::getFontFamily() const {
	return getFont("font-family");
}

void rd::Style::setInt(const char* name, int val) {
	vals.set(name, val);
}

void rd::Style::setBool(const char* name, bool val) {
	vals.set(name, val);
}

void rd::Style::setFloat(const char* name, float val){
	vals.set(name, val);
}

void rd::Style::setColor(const r::Color& col) {
	setColor("color", col);
}

void rd::Style::setColor(const char* name, const r::Color& col){
	ensure(name);
	vals.set(name, col);
	onChanged();
}

void rd::Style::editFloat(const char* name) {
	using namespace ImGui;
	PushID(name);
	if (!has(name)) {
		if (Button((std::string() + (""  ICON_MD_ADD) + name))) {
			vals.set(name, 0.0f);
			onChanged();
		}
	}
	else {
		auto nv = vals.getFloat(name);
		if (DragFloat(name, &nv)) {
			vals.set(name, nv);
			onChanged();
		}
		SameLine();
		if (Button(ICON_MD_DELETE)) vals.destroy(name);
	}
	PopID();
};

void rd::Style::editInt(const char* name) {
	using namespace ImGui;
	PushID(name);
	if (!has(name)) {
		if (Button((std::string() + (""  ICON_MD_ADD) + name))) {
			vals.set(name, 0); onChanged();
		}
	}
	else {
		auto nv = vals.getInt(name);
		if (DragInt(name, &nv)) {
			vals.set(name, nv); onChanged();
		}
		SameLine();
		if (Button(ICON_MD_DELETE)) {
			vals.destroy(name);
			onChanged();
		}
	}
	PopID();
};

void rd::Style::editInt2(const char* name) {
	using namespace ImGui;
	if (!has(name)) {
		if (Button((std::string() + (""  ICON_MD_ADD) + name))) {
			vals.set(name, 0); onChanged();
		}
	}
	else {
		auto nv = vals.getVec2i(name);
		if (DragInt2(name, nv->ptr())) {
			vals.set(name, *nv); onChanged();
		}
		SameLine();
		if (Button(ICON_MD_DELETE)) vals.destroy(name);
	}
}

void rd::Style::editColor(const char* name){
	using namespace ImGui;
	bool hasEntry = has(name);
	if (!hasEntry) {
		if (Checkbox(name, &hasEntry))
			setColor(r::Color());
	}
	else {
		auto val = getColor(name);
		if (ColorEdit4(name, val.ptr())) {
			vals.set(name, val);
			onChanged();
		}
		SameLine();
		if (Button(ICON_MD_DELETE)) vals.destroy(name);
	}
}


void rd::Style::editFloat2(const char* name) {
	using namespace ImGui;
	bool hasVal = has(name);
	if (!hasVal) {
		if (Checkbox(Str64f(ICON_MD_ADD "%s", name).c_str(), &hasVal)) {
			vals.set(name, Vector2(0, 0));
			onChanged();
		}
	}
	else {
		Vector2 pos = getVec2(name);
		if (DragFloat2(name, pos.ptr())) {
			vals.set(name, pos);
			onChanged();
		}
	}
	SameLine();
	if (Button(ICON_MD_DELETE)) vals.destroy(name);
};

void rd::Style::editFloat3(const char* name) {
	using namespace ImGui;
	bool hasVal = has(name);
	if (!hasVal) {
		if (Checkbox(Str64f(ICON_MD_ADD "%s", name).c_str(), &hasVal)) {
			vals.set(name, Vector3(0, 0, 0));
			onChanged();
		}
	}
	else {
		Vector3 pos = getVec3(name);
		if (DragFloat3(name, pos.ptr())) {
			vals.set(name, pos);
			onChanged();
		}
	}
	SameLine();
	if (Button(ICON_MD_DELETE)) vals.destroy(name);
};

void rd::Style::im(){
	using namespace ImGui;
	PushID(name.c_str());
	PushItemWidth(180);
	
	if (TreeNode(name)) {
		{
			bool hasPos = has("position");
			if (!hasPos) {
				if (Checkbox(ICON_MD_ADD"position", &hasPos))
					setPosition({0,0,0});
			}
			else {
				Vector2 pos = getPosition();
				if (DragFloat2("position", pos.ptr())) {
					setPosition(pos);
				}
			}
			SameLine();
			if (Button(ICON_MD_DELETE)) vals.destroy("position");
		}
		{
			bool hasTT = has("text-transform");
			if (!hasTT) {
				if (Checkbox(ICON_MD_ADD"text-transform", &hasTT))
					setTextTransform(None);
			}
			else {
				TextTransform tt = getTextTransform();
				if (BeginCombo("text-transform", std::to_string(tt).c_str())) {
					bool sel = false;
					if (Selectable("None", &sel)) 
						setTextTransform(None);
					if (Selectable("Capitalize", &sel))
						setTextTransform(Capitalize);
					if (Selectable("lowercase", &sel))
						setTextTransform(Lowercase);
					if (Selectable("UPPERCASE", &sel))
						setTextTransform(Uppercase);
					EndCombo();
				}
				SameLine();
				if (Button(ICON_MD_DELETE)) vals.destroy("text-transform");
			}
		}

		{
			std::string name = "font-autosize";
			if (!has(name)) {
				if (Button((std::string() + (""  ICON_MD_ADD) + name))) {
					vals.set(name.c_str(), Vector4i(250,28,22,0) ); onChanged();
				}
			}
			else {
				auto nv = vals.getVec4i(name.c_str());
				bool changed = false;

				changed |= DragInt("autosize max width", &nv->x);

				SameLine();
				if (Button(ICON_MD_DELETE)) {
					vals.destroy(name.c_str());
					onChanged();
				}

				changed |= DragInt("autosize desired font size", &nv->y);
				changed |= DragInt("autosize minimum font size", &nv->z);
					
				if (changed) {
					vals.set(name.c_str(), *nv); onChanged();
				}
				SameLine();
				if (Button(ICON_MD_DELETE)) vals.destroy(name.c_str());
			}
		}
		editInt("font-size");

		if (TreeNode("Spacing")) {
			editFloat("vertical-spacing");
			editFloat("horizontal-spacing");
			TreePop();
		}

		if (TreeNode("Colors")) {
			editColor("color");
			editColor("color:focused");
			editColor("color:active");
			editColor("color:greyed");
			TreePop();
		}

		editColor("drop-shadow-color");
		editColor("outline-color");

		if (TreeNode("raw data")) {
			bool chg = vals.im(false, "Values:");
			if (chg)
				onChanged();
			TreePop();
		}
		if (parent && TreeNode("Parent")) {
			parent->im();
			TreePop();
		}

		if (Button("Save")) {
			dirty = false;
			if (!jSerialize(*this, r::Conf::EDITOR_PREFAB_FOLDER, name.cpp_str() + ".style.json"))
				traceError(std::string() + "unable to save style " + name.cpp_str());
		}
		if (dirty) {
			SameLine();
			Warning("Style has unsaved changes");
		}
		if (Button("Reset")) {
			vals.dispose();
			onChanged();
		}
		if (Button("Reload"))
			load();
		TreePop();
	}
	PopItemWidth();
	PopID();
}

void rd::Style::load() {
	if (!jDeserialize(*this, r::Conf::EDITOR_PREFAB_FOLDER, name.cpp_str() + ".style.json"))
		traceError(std::string() + "unable to load style " + name.cpp_str());
	onChanged();
	dirty = false;
}

void rd::Style::setPosition(const Vector3& val){
	vals.set("position", Vector3(val.x, val.y,val.z));
	onChanged();
}

void rd::Style::setPosition(const Vector2& val) {
	vals.set("position", Vector3(val.x, val.y, 0));
	onChanged();
}

Vector3 rd::Style::getPosition() const{
	return getVec3("position");
}

int	rd::Style::getMarginLeft()		const { return getVec4i("margin").x; };
int rd::Style::getMarginTop()		const { return getVec4i("margin").y; };
int rd::Style::getMarginRight()		const { return getVec4i("margin").z; };
int rd::Style::getMarginBottom()	const { return getVec4i("margin").w; };

int rd::Style::getPaddingLeft()		const { return getVec4i("padding").x; };
int rd::Style::getPaddingTop()		const { return getVec4i("padding").y; };
int rd::Style::getPaddingRight()	const { return getVec4i("padding").z; };
int rd::Style::getPaddingBottom()	const { return getVec4i("padding").w; };

int rd::Style::getOffsetLeft()		const { return getVec4i("offset").x; };
int rd::Style::getOffsetTop()		const { return getVec4i("offset").y; };
int rd::Style::getOffsetRight()		const { return getVec4i("offset").z; };
int rd::Style::getOffsetBottom()	const { return getVec4i("offset").w; }

float rd::Style::getDropShadowX() const{
	return getFloat("drop-shadow-x");
}

float rd::Style::getDropShadowY() const {
	return getFloat("drop-shadow-y");
}

r::Color rd::Style::getDropShadowColor() const {
	return getColor("drop-shadow-color");
}

void rd::Style::setDropShadow(float x, float y, const r::Color& col){
	vals.set("drop-shadow", true);
	vals.set("drop-shadow-x", x);
	vals.set("drop-shadow-y", y);
	vals.set("drop-shadow-color", col);
}

float rd::Style::getOutlineDist() const {
	return getFloat("outline-dist");
}

r::Color rd::Style::getOutlineColor() const {
	return getColor("outline-color");
}

void rd::Style::setOutline(float dist, const r::Color& col) {
	vals.set("outline", true);
	vals.set("outline-color", col);
	vals.set("outline-dist", dist);
}

void rd::Style::serialize(Pasta::JReflect& jr, const char* _name){
	if (_name) jr.visitObjectBegin(_name);
	jr.visit(this->name, "name");
	jr.visit(vals, "vals");
	if (_name) jr.visitObjectEnd(_name);
}

void rd::Style::apply(r2::Node& node) const{
	if (has("position"))
		node.setPosVec3(getPosition());
}

void rd::Style::apply(r2::Flow& fl) const{
	rd::Style::apply((r2::Node&)fl);

	if (has("layout") && vals.getString("layout") == "vertical")
		fl.vertical();
	if (has("layout") && vals.getString("layout") == "horizontal")
		fl.horizontal();
	if (has("vertical-spacing")) {
		fl.verticalSpacing = std::lrint(vals.getFloat("vertical-spacing"));
		fl.needReflow = true;
	}
	if (has("horizontal-spacing")) {
		fl.horizontalSpacing = std::lrint(vals.getFloat("horizontal-spacing"));
		fl.needReflow = true;
	}
}


void rd::Style::apply(r2::Text& txt) const {
	apply((r2::Node&)txt);
	auto fnt = getFontFamily();
	if (fnt)
		txt.setFont(getFontFamily());

	if (has("font-autosize")) {
		Vector4i t = getVec4i("font-autosize");
		txt.autosize(t.x,t.y,t.z);
	}
	else 
		txt.clearAutosize();
	
	if (has("font-size"))
		txt.setFontSize(getInt("font-size"));
	
	if (has("color"))
		txt.setTextColor(getColor());
	else
		txt.setTextColor(r::Color());

	if (has("drop-shadow"))
		txt.addDropShadow(getDropShadowX(), getDropShadowY(), getDropShadowColor());
	else
		txt.removeDropShadow();
	
	if (has("outline"))
		txt.addOutline(getOutlineColor(), getOutlineDist());
	else
		txt.removeOutline();

	if (has("max-width")) 
		txt.setMaxLineWidth(vals.getFloat("max-width"));
	else
		txt.setMaxLineWidth(-1);

	if (has("text-transform")) {
		auto tt = getTextTransform();
		if (tt != None) {
			std::string t = txt.getText();
			switch (tt) {
			case rd::Style::TextTransform::Capitalize:
				ri18n::T::capitalize(t);
				break;
			case rd::Style::TextTransform::Uppercase:
				t = ri18n::T::toupper(t);
				break;
			case rd::Style::TextTransform::Lowercase:
				t = ri18n::T::tolower(t);
				break;
			}
			txt.setText(t);
		}
	}
}

std::string std::to_string( rd::Style::TextTransform tt){
	switch (tt) {
		case rd::Style::TextTransform::None: return "None";
		case rd::Style::TextTransform::Capitalize: return "Capitalize";
		case rd::Style::TextTransform::Lowercase: return "Lowercase";
		case rd::Style::TextTransform::Uppercase: return "Uppercase";
	}
	return "";
}

void rd::Style::setTextTransform(TextTransform val) {
	vals.set("text-transform", (int)val);
	onChanged();
}

rd::Style::TextTransform rd::Style::getTextTransform() const {
	if(!has("text-transform"))
		return rd::Style::TextTransform::None;
	return (rd::Style::TextTransform)getInt("text-transform");
}