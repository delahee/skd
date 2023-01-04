#include "stdafx.h"

#include "TilePicker.hpp"

using namespace std;
using namespace r2;
using namespace r2::im;

static string tempBuffer;

std::vector<rd::TileLib*> TilePicker::sources;

#pragma region SETTINGS
#include "rd/JSerialize.hpp"
void r2::im::TilePicker::load() {
	if (!jDeserialize(*this, r::Conf::EDITOR_FOLDER, "editor.tilepicker.json"))
		std::cout << "[TilePicker] load failed" << std::endl;
}

void r2::im::TilePicker::save() {
	if (!jSerialize(*this, r::Conf::EDITOR_FOLDER, "editor.tilepicker.json"))
		std::cout << "[TilePicker] save failed" << std::endl;
}

void  r2::im::TilePicker::visit(Pasta::JReflect& functor) {
	functor.visitObjectBegin(nullptr);//necessary to create a json object
	functor.visitBool(preview, "preview");
	functor.visitBool(excludeAnimSubFrames, "excludeAnimSubFrames");
	functor.visitBool(animated, "animated");
	functor.visitObjectEnd(nullptr);
}

template<> inline void Pasta::JReflect::visit(r2::im::TilePicker& le, const char* name) {
	if (name) visitObjectBegin(name);
	le.visit(*this);
	if (name) visitObjectEnd(name);
};
#pragma endregion

r2::im::TilePicker::TilePicker(Promise* prom) {
	load();
	name = "Tile Picker";
	pickedTile = rd::Pools::tiles.alloc();
	this->prom = prom;
	opened = false;
	scheduleTermination = false;
	rs::Sys::enterFrameProcesses.add(this);
}

r2::im::TilePicker::~TilePicker() {
	if(prom && !prom->isSettled())
		prom->reject(string("Window Deletion"));
	if(pickedTile) rd::Pools::tiles.free(pickedTile);
	prom = nullptr;
	opened = false;
}

static std::vector<rd::TileGroup*> getSortedTiles(rd::TileLib* lib) {
	std::vector<rd::TileGroup*> groups;
	for (auto& pair : lib->groups)
		groups.push_back(pair.second);

	sort(groups.begin(), groups.end(), [](auto& t0, auto& t1) {
		return t0->id < t1->id;
	});
	return groups;
}


void r2::im::TilePicker::imTiles(rd::TileLib* lib) {
	std::vector<rd::TileGroup*> groups = getSortedTiles(lib);
	ImGui::Text(lib->name.c_str());
	ImGui::Separator();
	if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Content"))
		{
			if (strlen(curSearch.c_str()) == 0)
				curSearch = "";
			ImGui::InputText("filter", curSearch);

			ImGui::BeginChild("lib content");
			int x = 0, y = 0, margin = 10;
			for (rd::TileGroup* g : groups) {
				std::string name = g->id;
				bool skip = false;
				if (excludeAnimSubFrames) {
					for (int i = 0; i < name.size(); i++)
						if (name.at(i) >= '0' && name.at(i) <= '9') {
							skip = true;
							break;
						}
				}

				if (g->anim.size() > 0)
					name = name + " [" + std::to_string(g->anim.size()) +" frames]";

				if (curSearch != "") {
					if ( !rd::String::containsI(name.c_str(), curSearch.c_str()))
						skip = true;
				}

				if (skip) continue;
				bool selected = false;
				if (preview) {
					ImGui::PushID(g->id.c_str());

					ImGui::SetCursorPos(ImVec2(x, y));
					ImVec2 posContent = ImGui::GetCursorPos();
					selected = ImGui::Selectable("##selectable", false, 0, ImVec2(110, 105 + ImGui::GetTextLineHeight() * 2));
					ImGui::SetCursorPos(posContent);
					ImGui::PushTextWrapPos(posContent.x + 115);
					ImGui::BeginGroup();
						Pasta::ShadedTexture* st = rd::Pools::allocForFrame();
						FrameData* fd = nullptr;
						if (animated && g->anim.size() > 0)
							fd = lib->getFrameData(g->id, (rs::Timer::frameCount / 4) % g->anim.size());
						else
							fd = lib->getFrameData(g->id, 0);
						st->texture = lib->textures[fd->texSlot];
						ImVec2 uv0(fd->x / (float)st->texture->getWidth(), fd->y / (float)st->texture->getHeight());
						ImVec2 uv1((fd->x + fd->wid) / (float)st->texture->getWidth(), (fd->y + fd->hei) / (float)st->texture->getHeight());
						float ratio = fd->wid / (float)fd->hei;
						ImVec2 size = ImVec2(100, 100 / ratio);
						if (size.y > 100)
							size = ImVec2(100 * ratio, 100);
						ImGui::Dummy(ImVec2(1, 100)); ImGui::SameLine();
						ImGui::Image((ImTextureID)st, size, uv0, uv1);
						ImGui::TextWrapped(name.c_str());
					ImGui::EndGroup();
					ImGui::PopTextWrapPos();

					ImGui::PopID();

					x += 110 + margin;
					if (x + 110 < ImGui::GetContentRegionAvail().x) {

						//ImGui::SameLine();
					}
					else {
						x = 0;
						y += 105 + ImGui::GetTextLineHeight() * 2 + margin;
					}
				} else
					selected = ImGui::Selectable(name.c_str(), false, 0, ImVec2(400, 12));

				if (selected) {
					pickedTile = lib->getTile(g->id, 0, 0, 0, pickedTile);
					pickedLib = lib;
					pickedGroup = g;
					prom->resolve(pickedTile);
					prom = nullptr;
					scheduleTermination = true;
					break;
				}
			}

			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void r2::im::TilePicker::update(double dt) {
	Agent::update(dt);
	if (!opened) {
		ImGui::OpenPopup("Tile Picker");
		opened = true;
	}
	ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
	if (ImGui::BeginPopupModal("Tile Picker", &opened, 0)) {
		// top
		if (ImGui::Checkbox("excludeAnimSubFrames", &excludeAnimSubFrames))
			save();
		if (ImGui::Checkbox("preview", &preview))
			save();
		if (preview) {
			ImGui::SameLine();
			if (ImGui::Checkbox("animated", &animated))
				save();
		}

		// left
		ImGui::BeginChild("left pane", ImVec2(150, 0), true);
		std::vector<rd::TileLib*> libs(sources.begin(), sources.end());
		sort(libs.begin(), libs.end(), [](auto& t0, auto& t1) {
			return t0->name < t1->name;
			});

		for (rd::TileLib* tl : libs) {
			ImGui::PushID(tl->name.c_str());
			if (ImGui::Selectable(tl->name.c_str(), pickedLib == tl))
				pickedLib = tl;
			ImGui::PopID();
		}
		ImGui::EndChild();
		ImGui::SameLine();

		// right
		ImGui::BeginGroup();
		ImGui::BeginChild("right pane");
		if (pickedLib) imTiles(pickedLib);
		else ImGui::Text("Please select a library");
		ImGui::EndChild();
		ImGui::EndGroup();

		ImGui::EndPopup();
	}

	if (!opened) {
		prom->reject(string("Canceled"));
		prom = nullptr;
		scheduleTermination = true;
	}

	if (scheduleTermination) {
		ImGui::CloseCurrentPopup();
		opened = false;
		delete this;
	}
}

void r2::im::TilePicker::forceClose(){
	scheduleTermination = true;
	if(prom && !prom->isSettled()) prom->reject(string("Force Closed"));
}

Promise* r2::im::TilePicker::forPicker() {
	auto p = Promise::alloc();
	auto tp = new r2::im::TilePicker(p);
	//no need for auto release, end thener should do it
	p->autoReleased();
	p->then([tp](Promise* lthis, std::any data) {
		return tp;
	}, [](Promise* lthis, std::any err) {
		return err;
	});
	return p;
}

Promise* r2::im::TilePicker::forPackage(rd::TilePackage& tpack) {
	auto p = Promise::alloc();
	auto tp = new r2::im::TilePicker(p);
	p->autoReleased();
	p->then([&tpack,tp](Promise* lthis, std::any data) {
		r2::Tile* t = std::any_cast<r2::Tile*>(data);
		if (t) {
			tpack.tile->copy(*t);
			if (tp->pickedLib) tpack.lib = tp->pickedLib->name;
			if (tp->pickedGroup) tpack.group = tp->pickedGroup->id;
		}
		return data;
		}, [](Promise* lthis, std::any err) {
			return err;
		});
	return p;
}

Promise* r2::im::TilePicker::forTile(r2::Tile & tile) {
	auto p = Promise::alloc();
	auto tp = new r2::im::TilePicker(p);
	p->then([&tile](Promise* lthis, std::any data) {
		r2::Tile* t = std::any_cast<r2::Tile*>(data);
		if( t )
			tile.copy(*t);
		return data;
	}, [](Promise* lthis, std::any err) {
		return err;
	});
	return p;
}

void r2::im::TilePicker::forBitmap(r2::Bitmap* bmp){
	auto p = Promise::alloc();
	auto tp = new r2::im::TilePicker(p);
	std::function cbk = [tp]() { tp->forceClose(); };
	SignalHandler* hdl = bmp->onDeletion.addOnce(cbk);
	p->autoReleased();
	p->then([tp,bmp, hdl](Promise* lthis, std::any data) {
		bmp->onDeletion.remove(hdl);
		r2::Tile* t = std::any_cast<r2::Tile*>(data);
		bmp->copyTile(t);
		if(tp->pickedLib) bmp->vars.set("rd::TileLib", tp->pickedLib->name);
		if(tp->pickedGroup) bmp->vars.set("rd::TileGroup", tp->pickedGroup->id);
		return data;
	}, [bmp, hdl](Promise* lthis, std::any err) {
		bmp->onDeletion.remove(hdl);
		return err;
	});
}

void r2::im::TilePicker::pick(Promise * prom) {
	TilePicker * picker = new TilePicker(prom);
	rs::Sys::enterFrameProcesses.push_back(picker);
}

void r2::im::TilePicker::registerTileSource(rd::TileLib * lib) {
	auto pos = find(sources.begin(), sources.end(), lib);
	if (lib && pos == sources.end()) sources.push_back(lib);
}

void r2::im::TilePicker::unregisterTileSource(rd::TileLib * lib) {
	auto pos = find(sources.begin(), sources.end(), lib);
	if(pos != sources.end())
		sources.erase(pos);
}

//don't forget the .xml
rd::TileLib* r2::im::TilePicker::getLib(const char* name) {
	if (!name) return nullptr;
	if (name && !strstr(name, ".xml")) {
		tempBuffer = string() + name + ".xml";
		name = tempBuffer.c_str();
	}
	for (auto iter = sources.begin(); iter != sources.end(); ++iter)
		if ((*iter)->name == name)
			return *iter;
	return nullptr;
}

bool r2::im::TilePicker::hasLib(const char* name) {
	if (!strstr(name, ".xml")) {
		tempBuffer = string() + name + ".xml";
		name = tempBuffer.c_str();
	}
	for (rd::TileLib* iter : sources)
		if (iter->name == name) 
			return true;
	return false;
}

rd::TileLib* r2::im::TilePicker::getLib(const std::string & _name) {
	const char* name = _name.c_str();
	return getLib(name);
}

rd::TileLib* r2::im::TilePicker::getOrLoadLib(const std::string& _name, TPConf * tpc) {
	rd::TileLib* tl = getLib(_name);
	if (tl) return tl;

	auto l = rd::TexturePacker::load(!rd::String::endsWith(_name, ".xml")?(_name+".xml"):_name, tpc);
	if (l) registerTileSource(l);
	return l;
}