#include "stdafx.h"
#include "GradientEditor.hpp"
#include "r2/ext/ExtGraphics.hpp"
#include "rd/JSerialize.hpp"
#include <fstream>
#ifdef PASTA_WIN
#include <filesystem>
#endif
#include "rs/File.hpp"

using namespace std;

using namespace r2;
using namespace r2::tools;

#define SUPER rd::Agent

GradientEditor* GradientEditor::me=0;

r2::tools::GradientEditor::GradientEditor(r2::Node * r) {
	root = r;
	rs::Svc::reg(this);
}

r2::tools::GradientEditor::~GradientEditor() {
	if (target) {
		delete target;
		target = 0;
	}
	root = 0;
	rs::Svc::unreg(this);
}

template<> inline
void Pasta::JReflect::visit(eastl::pair<float, r::Color>& c, const char* name) {
	visitObjectBegin(name);
	visit(c.first, "f");
	visit(c.second, "s");
	visitObjectEnd(name);
};

template<> inline
void Pasta::JReflect::visit(eastl::tuple<float, r::Color, r::Color>& c, const char* name ) {
	visitObjectBegin(name);
	visit(eastl::get<0>(c), "a");
	visit(eastl::get<1>(c), "b");
	visit(eastl::get<2>(c), "c");
	visitObjectEnd(name);
};

static int fr = 0;
void r2::tools::GradientEditor::update(double dt) {
	SUPER::update(dt);
	im();
	fr++;
}

void r2::tools::GradientEditor::serialize(Pasta::JReflect& f,const char * _name) {
	if (_name) f.visitObjectBegin(_name);
	f.visit((int&)mode, "m");
	f.visit(name, "name");
	f.visit(cols, "cols");
	f.visit(boxCols, "boxCols");
	if (_name) f.visitObjectEnd(_name);
}

bool r2::tools::GradientEditor::im() {
	using namespace ImGui;

	if (!target) {
		target = new Graphics(root);
		target->setPos(100, 100);
	}
	if (!root) return false;
	if (!target) return false;

	bool rebuild = false;
	if (0==fr) rebuild = true;
	Begin("Gradient Editor");


	
	InputText("name",name);
	r2::Scene* sc = root->getScene();
	ColorEdit4("Background color", sc->clearColor.ptr());
	ImGui::Text("Gradient blend");
	target->imBlend();

	bool hasChecker = checker != nullptr;
	if (Checkbox("checker pattern", &hasChecker)) {
		if (!hasChecker) {
			if (checker) {
				checker->destroy();
				checker = nullptr;
			}
		}
		else {
			if (!checker) {
				checker = r2::Bitmap::fromImageFile("checker.png", sc);
				checker->setSize(1024, 1024);
				checker->toBack();
			}
		}

	}

	const char* modes[] = { "linear","box","bands" };
	Combo("mode", (int*)&mode, modes, IM_ARRAYSIZE(modes));

	if ((mode == GradientMode::Linear) || (mode == GradientMode::Bands)) {
		for (int i = 0; i < cols.size(); ++i) {
			auto& p = cols[i];
			PushID(i);
			Separator();
			Value("idx", i);
			if (DragFloat("pos", &p.first, 0.01f, 0.0f, 1.0f, "%0.2f %")) {
				rebuild = true;
			}
			if (ColorEdit4("col", p.second.ptr())) {
				rebuild = true;
			}
			if (Button(ICON_MD_DELETE)) {
				cols.erase(cols.begin() + i);
				rebuild = true;
				PopID();
				break;
			}
			Separator();
			PopID();
		}
		if (Button(ICON_MD_ADD)) {
			float last_date = 1.0;
			if (cols.size() == 0) {
				last_date = 0.0;
			}
			else if (cols.size() > 0) {
				last_date = cols[cols.size() - 1].first + 0.01;
			}
			cols.push_back(eastl::make_pair(last_date, r::Color::White));
			rebuild = true;
		}
	}
	else {
		for (int i = 0; i < boxCols.size(); ++i) {
			auto& p = boxCols[i];
			PushID(i);
			Separator();
			Value("idx", i);

			float& pos = get<0>(p);
			if (DragFloat("pos", &pos, 0.01f, 0.0f, 1.0f, "%0.2f %")) {
				rebuild = true;
			}
			if (ColorEdit4("left col", get<1>(p).ptr())) {
				rebuild = true;
			}
			if (ColorEdit4("right col", get<2>(p).ptr() )) {
				rebuild = true;
			}

			if (Button(ICON_MD_DELETE)) {
				boxCols.erase(boxCols.begin() + i);
				rebuild = true;
				PopID();
				break;
			}
			Separator();
			PopID();
		}
		if (Button(ICON_MD_ADD)) {
			float last_date = 1.0;
			if (boxCols.size() == 0) {
				last_date = 0.0;
			}
			else if (boxCols.size() > 0) {
				last_date = get<0>(boxCols.back()) + 0.01;
			}
			boxCols.push_back(eastl::make_tuple(last_date, r::Color::White, r::Color::Red));
			rebuild = true;
		}
	}

	if (Button("Rebuild")) {
		rebuild = true;
	}

	if (Button("Save")) {
		bool ok = false;
		ok = jSerialize(*this, r::Conf::EDITOR_GRADIENT_FOLDER, name + ".gradient.json");
		cout << "saved :" << ok << endl;
	}

#ifdef PASTA_WIN
	if (Button("Load")) {
		string fsPath;
		if (rs::Sys::filePickForOpen(fsPath)) {
			std::filesystem::path p(fsPath);

			string fname = rd::String::split(p.filename().string(),".")[0];
			name = fname;

			std::string content;
			if (rs::File::read(fsPath, content)) 
				jDeserializeFromString(*this, content.c_str());
			
			rebuild = true;
			cout << "loaded" << endl;
		}
	}
#endif
	
	if (Button("DumpCpp")) {
		dumpCpp();
	}

	if (rebuild) {
		target->clear();
		
		float w = 256;
		if (mode == GradientMode::Linear) {
			r2::ext::drawGradient(target, Vector3(50 + w * 0.5, 0, 0), Vector3(50 + w * 0.5, 800, 0), cols, 256);
		}
		else if (mode == GradientMode::Bands) {
			r2::ext::drawBands(target, Vector3(50 + w * 0.5, 0, 0), Vector3(50 + w * 0.5, 800, 0), cols, 256);
		}
		else {
			r2::ext::drawBoxGradient(target, Vector3(50 + w * 0.5, 0, 0), Vector3(50 + w * 0.5, 800, 0), boxCols, 256);
		}
	}

	End();
	return false;
}

void r2::tools::GradientEditor::toggle(r2::Node * n){
	if (!me)
		me = new GradientEditor(n);
	else {
		auto ge = me;
		delete ge;
		me = 0;
	}

}

void r2::tools::GradientEditor::dumpCpp() {
	cout << "auto g = new r2::Graphics(root);" << endl;
	cout << "r::Vector3 top = r::Vector3(0,-50,0);" << endl;
	cout << "r::Vector3 bottom = r::Vector3(0,50,0);" << endl;
	cout << "float w = 50;" << endl;
	if (mode == GradientMode::Linear) {
		cout << "eastl::vector<eastl::pair<float,r::Color>> cols = {" << endl;
		for (auto& p : cols)
			cout<< "eastl::make_pair(" << p.first <<", r::Color::fromUInt(0x" << p.second.toHexString() << ") ),"<<endl;
		cout << "};" << endl;
		cout << "r2::ext::drawGradient(g, top,bottom, cols, w);" << endl;
	}
	else if (mode == GradientMode::Bands) {
		cout << "eastl::vector<eastl::pair<float,r::Color>> cols = {" << endl;
		for (auto& p : cols)
			cout << "eastl::make_pair(" << p.first << ", r::Color::fromUInt(0x" << p.second.toHexString() << ") )," << endl;
		cout << "};" << endl;
		cout << "r2::ext::drawBands(g, top,bottom, cols, w);" << endl;
	}
	else {
		cout << "eastl::vector<eastl::tuple<float,r::Color,r::Color>> boxCols = {" << endl;
		for (auto& p : boxCols)
			cout << " eastl::make_tuple(" << eastl::get<0>(p) 
			<< " , r::Color::fromUInt(0x" << eastl::get<1>(p).toHexString()<<")"
			<< " , r::Color::fromUInt(0x" << eastl::get<2>(p).toHexString() 
			<< ") )," << endl;
		cout << "};" << endl;
		cout << "r2::ext::drawBoxGradient(g, top, bottom, boxCols, w);" << endl;
	}
}

#undef SUPER
