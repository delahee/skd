#include "stdafx.h"
#include "1-files/FileMgr.h"
#include "1-files/Path.h"
#include "FontManager.hpp"
#include "2-application/OS.h"
#include "r2/Types.hpp"
#include "rs/Std.hpp"

using namespace rd;
using namespace std;
using namespace Pasta;

FontManager * FontManager::me = nullptr;

FontManager & FontManager::get(){
	if (me == nullptr) me = new FontManager();
	return *me;
}

rd::Font* FontManager::getFont(const std::string& name) {
	return map[StrRef(name.c_str())];
}

const char* FontManager::getFontName(rd::Font*fnt)const {
	for (auto& p : map) 
		if (p.second == fnt)
			return p.first.c_str();
	return 0;
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
		switch (filter)
		{
		case r2::TexFilter::TF_LINEAR: filterFl = PASTA_TEXTURE_LINEAR; break;
		case r2::TexFilter::TF_ANISO: filterFl = PASTA_TEXTURE_ANISOTROPIC; break;
		default: filterFl = PASTA_TEXTURE_POINT; break;
		};
		tex->setFilterMode(filterFl);
	}

	if (!dflt)
		dflt = fnt;
		
	return map[name] = fnt;
}

void FontManager::destroy(const string & name) {
	Font* fnt = map[name];
	if (fnt) {
		delete fnt;
		map.erase(name);
	}
}

void FontManager::setPremultipliedAlpha(rd::Font* fnt, bool onOff) {
	int nbTex = fnt->getNbPages();
	for (int i = 0; i < nbTex; ++i) {
		Texture * tex = fnt->getTexture(i);
		tex->setPremultipliedAlpha(onOff);
	}
}


void FontManager::im() {
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
			ImGui::Text("Nb pages: %d", font->getNbPages());

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
}

void FontManager::alias(const char* dst, const char* src) {
	map[Str(dst)] = map[StrRef(src)];
}