#include "stdafx.h"
#include "1-files/FileMgr.h"
#include "1-files/Path.h"
#include "FontManager.hpp"
#include "2-application/OS.h"
#include "r2/Types.hpp"
#include "rs/Std.hpp"
#include "UTF8Utils.hpp"

using namespace rd;
using namespace std;
using namespace Pasta;

FontManager * FontManager::me = nullptr;

FontManager & FontManager::get(){
	if (me == nullptr) me = new FontManager();
	return *me;
}

rd::FontManager::FontManager() {
	
}

rd::Font* FontManager::getFont(const char* name) {
	StrRef key(name);
	auto res = map.find(key);
	if( res == map.end())
		return 0;
	return res->second;
}

rd::Font* rd::FontManager::getFont(const std::string& name){
	return getFont(name.c_str());
}

const char* FontManager::getFontName(rd::Font*fnt)const {
	for (auto& p : map) 
		if (p.second == fnt)
			return p.first.c_str();
	return 0;
}

rd::FontManager::FontDesc* rd::FontManager::getExt(const char* name){
	auto res = ext.find(StrRef(name));
	if (res == ext.end() ) return 0;
	return res->second;
}

rd::Font* FontManager::add(const string & name, const string & _path, r2::TexFilter filter, r2::Shader shader) {
	StrRef key(name.c_str());
	if(map.find(key) != map.end())
		return map[key];
	
	FileMgr* mgr = FileMgr::getSingleton();
	string path = mgr->convertResourcePath(_path);
	if (!mgr->exists(path)) {
		printf("Font does not exist..%s\n", path.c_str());
		return nullptr;
	}
	Font* fnt = new Font(_path.c_str(), shader);
	fnt->load();

	if (RESOURCE_LOAD_ERROR==fnt->getErrorState()) {
		printf("Font not loaded..%s\n", path.c_str());
		return nullptr;
	}
	
	if (fnt->getNbPages() == 0) {
		printf("Font not loaded..%s\n", path.c_str());
		return nullptr;
	}

	int nbTex = fnt->getNbPages();

	for (int i = 0; i < nbTex; ++i) {
		Texture * tex = fnt->getTexture(i);
		TextureFlags fl = tex->getFlags();

		//should not put aniso here without customization caps
		u32 filterFl = 0;
		switch (filter){
			case r2::TexFilter::TF_LINEAR:	filterFl = PASTA_TEXTURE_LINEAR; break;
			case r2::TexFilter::TF_ANISO:	filterFl = PASTA_TEXTURE_ANISOTROPIC; break;
			default:						filterFl = PASTA_TEXTURE_POINT; break;
		};
		tex->setFilterMode(filterFl);
	}

	if (!dflt)
		dflt = fnt;
	
	map[name] = fnt;

	bool isFontReliable = fnt->hasCharDescr( rd::String::utf8code(u8"Ã"));
	if(!isFontReliable)//hmm to evaluate 
		createCharAliases(fnt);
	return fnt;
}

void FontManager::destroy(const string & name) {
	Font* fnt = rs::Std::get<rd::Font>(map,StrRef(name.data()));
	if (fnt) {
		delete fnt;
		map.erase(name);
	}
}

void rd::FontManager::createCharAliases(rd::Font*font){
	auto fntName = getFontName(font);
	auto desc = ext[Str(fntName)] = new FontDesc();

	bool KEEP_ACCENTS = false;
	auto m = [=](int oldCode, int newCode) {
		desc->fallbacks[oldCode] = newCode;
	};

	auto code = [](const char* str) ->int {
		return rd::String::utf8code(str);
	};

	if (!KEEP_ACCENTS) {
#define LOCAL_FOR( from, to ) for (int i = code( from ); i <= code( to); ++i)

		// Latin1 accents
		LOCAL_FOR(u8"À", u8"Æ")
			m(i, code("A"));

	
		LOCAL_FOR(u8"à",u8"æ")
			m(i, code("a"));
		LOCAL_FOR(u8"È", u8"Ë")
			m(i, code("E"));
		LOCAL_FOR(u8"è", u8"ë")
			m(i, code("e"));
		LOCAL_FOR(u8"Ì", u8"Ï")
			m(i, code("I"));
		LOCAL_FOR(u8"ì", u8"ï")
			m(i, code("i"));
		LOCAL_FOR("Ò", u8"Ö")
			m(i, code("O"));
		LOCAL_FOR(u8"ò", u8"ö")
			m(i, code("o"));
		LOCAL_FOR(u8"Ù", u8"Ü")
			m(i, code("U"));
		LOCAL_FOR(u8"ù", u8"ü")
			m(i, code("u"));

		m(code("Ç"), code("C"));
		m(code("ç"), code("C"));
		m(code("Ð"), code("D"));
		m(code("Þ"), code("d"));
		m(code("Ñ"), code("N"));
		m(code("ñ"), code("n"));
		m(code("Ý"), code("Y"));
		m(code("ý"), code("y"));
		m(code("ÿ"), code("y"));
#undef LOCAL_FOR
	}

	// unicode spaces
	m(0x3000, 0x20); // full width space
	m(0xA0, 0x20); // nbsp

	// unicode quotes
	m(code("«"), code("\""));
	m(code("»"), code("\""));
	m(code("“"), code("\""));
	m(code("”"), code("\""));
	m(code("‘"), code("'"));
	m(code("’"), code("'"));
	m(code("´"), code("'"));
	m(code("‘"), code("'"));
	m(code("‹"), code("<"));
	m(code("›"), code(">"));
}

void FontManager::setPremultipliedAlpha(rd::Font* fnt, bool onOff) {
	int nbTex = fnt->getNbPages();
	for (int i = 0; i < nbTex; ++i) {
		Texture * tex = fnt->getTexture(i);
		tex->setPremultipliedAlpha(onOff);
	}
}

bool FontManager::isDefaultChar(rd::Font* font, int code) {
	if (!font) return true;
	return !font->hasCharDescr(code);
}

const CharDescr* rd::FontManager::getCharDescr(rd::Font* font, int code){
	if (!font) return 0;
	if (isDefaultChar(font, code)) {
		auto ext = getExt(getFontName(font));
		if (ext) 
			return font->getCharDescr(ext->fallbacks[code]);
	}
	return font->getCharDescr(code);
}

void FontManager::imChar(rd::Font*font,int code ) {
	using namespace ImGui;

	bool isDefaultChar = !font->hasCharDescr(code);
	auto desc = font->getCharDescr(code);
	if (!desc) return;
	Value("is Default/fallback char", isDefaultChar);
	Value("valid", desc->isValid());
	Value("srcX",desc->srcX);
	Value("srcY",desc->srcY);
	Value("srcW",desc->srcW);
	Value("srcH",desc->srcH);
	Value("xOff",desc->xOff);
	Value("yOff",desc->yOff);
	Value("xAdv",desc->xAdv);
	Value("page",desc->page);
	Value("chnl",desc->chnl);
}

void FontManager::im() {
	using namespace ImGui;
	std::vector<pair<Str, rd::Font*>> fonts(map.begin(), map.end());

	sort(fonts.begin(), fonts.end(), [](auto & p0, auto & p1) {
		return p0.first < p1.first;
	});

	ImGui::BeginMenuBar();
#ifdef PASTA_WIN
	if (ImGui::MenuItem("Add")) {
		//file pick to library
		{
			string str;
			vector<pair<string, string>> filters;
			filters.push_back(pair("BMFont file (.fnt)", "*.fnt"));
			if (rs::Sys::filePickForOpen(filters, str)) {
				if (str.find("res") == str.npos) {
					OsDialogError("It seems library is not in a res folder.\nCannot read library file", "Error");
				}
				else {
					int resPos = str.rfind("res");
					string pathRes = str.substr(resPos + strlen("res/"));
					for( size_t n = 0; (n = pathRes.find('\\', n)) != string::npos; ) pathRes.replace(n, 1, "/");

					int namePos = pathRes.rfind("/") + 1;
					string name = pathRes.substr(namePos, pathRes.size() - namePos - strlen(".fnt"));

					add(name, pathRes);
				}
			}
			else {
				//Operation Canceled
				//OsDialogInfo("Sorry", "Cannot pick file");
			}
		}
	}
#endif
	ImGui::EndMenuBar();

	for(auto & pair : fonts) {
		rd::Font* font = pair.second;
		if (ImGui::TreeNode(pair.first.c_str())) {
			ImGui::Text("Size: %7.3f", font->getSize());
			ImGui::Text("Line Height: %7.3f", font->getLineHeight());
			ImGui::Text("Base: %7.3f", font->getBase());
			ImGui::Separator();

			static Str query;
			ImGui::InputText("query", query);
			if (query.length()) {
				u32 code = 0;
				auto c = rd::UTF8Utils::Advance((UTF8Char*)query.c_str(),code);
				while (code) {
					Text("%c", code);
					Value("code", code);
					imChar(font,code);
					c = rd::UTF8Utils::Advance(c,code);
				}
			}

			ImGui::Text("Nb pages: %d", font->getNbPages());
			Separator();
			Text("Pages");
			for (int page = 0; page < font->getNbPages(); page++) {
				ImGui::Text("%d.", page + 1); ImGui::SameLine();
				ShadedTexture * st = rd::Pools::allocForFrame();
				st->texture = font->getTexture(page);

				r2::Im::imTextureInfos(st->texture);
				ImGui::Image((ImTextureID)st, ImVec2(200, 200));
			}

			ImGui::TreePop();
		}
	}

	Separator();
	Text("Extensions");
	for (auto& kv : ext) {
		for (auto& p : kv.second->fallbacks) {
			Text("%d => %d", p.first, p.first, p.second);
			if (Button("Trace"))
				trace( rd::String::getUtf8fromC32(p.second));
		}
	}
}

void FontManager::alias(const char* dst, const char* src) {
	map[Str(dst)] = map[StrRef(src)];
}