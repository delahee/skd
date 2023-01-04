#include "stdafx.h"

#include "Im.hpp"
#include "r2/BatchElem.hpp"
#include "1-graphics/Graphic.h"
#include "rd/AudioMan.hpp"
#include "im/TilePicker.hpp"
#include "rs/Checksum.hpp"

using namespace std;
using namespace r;
using namespace r2;

eastl::vector<r2::Node *>		r2::Im::depletedNodes;
eastl::vector<r2::BatchElem *>	r2::Im::depletedElems;

void r2::Im::keepAlive(r2::Node& n){
	rs::Std::remove(r2::Im::depletedNodes, &n);
}

void r2::Im::keepAlive(r2::BatchElem& be) {
	rs::Std::remove(r2::Im::depletedElems, &be);
}

bool r2::Im::imTextureData(Pasta::TextureData* data){
	using namespace ImGui;
	Value("Type", data->dataType);
	Value("Width", data->width);
	Value("Height", data->height);
	Value("Depth", data->depth);
	r2::Im::previewPixels(data,256);
	return false;
}

void r2::Im::imTextureInfos(Pasta::Texture* t){
	using namespace ImGui;
	NewLine();
	Value("Format", t->getFormat());
	if (IsItemHovered()) {
		string fmt;
		fmt = Pasta::TextureFormat::GetString(t->getFormat());
		SetTooltip(fmt.c_str());
	}
	Value("nb Channels", Pasta::TextureFormat::GetChannelCount(t->getFormat()));
	Value("Flags", t->getFlags());
	if (IsItemHovered()) {
		string fmt;
		u32 fl = t->getFlags();

		int lin = PASTA_TEXTURE_LINEAR;
		int near = PASTA_TEXTURE_POINT;

		if (fl & PASTA_TEXTURE_POINT)
			fmt += "filter:linear ";
		else if (fl & PASTA_TEXTURE_ANISOTROPIC)
			fmt += "filter:aniso ";
		else //PASTA_TEXTURE_LINEAR
			fmt += "filter:linear ";

		if (fl & PASTA_TEXTURE_CLAMP)
			fmt += "clamp ";
		else if (fl & PASTA_TEXTURE_MIRROR)
			fmt += "mirror ";
		else //if (fl & PASTA_TEXTURE_REPEAT)
			fmt += "repeat ";
		SetTooltip(fmt.c_str());
	}
	Value("Type", t->getType());
	Value("Width", t->getWidth());
	Value("Height", t->getHeight());
	Value("LWidth", t->getLogicalWidth());
	Value("LHeight", t->getLogicalHeight());
	Value("Array Size", t->getArraySize());
}

void r2::Im::imTw(rd::Tweener* t)
{
	using namespace ImGui;
	PushID(t);
	if (CollapsingHeader("Tweener")) {
		Indent();
		Value("Nb Tweens ", t->tList.size());
		if (TreeNode("Pool")) {
			Value("Nb pooled ", t->pool.repo.size());
			Value("Nb active", t->pool.active.size());
			TreePop();
		}
		if (TreeNode("Values")) {
			for (auto v : t->tList) v->im();
			TreePop();
		}
		Unindent();
	}
	PopID();
}

bool r2::Im::metadata(rd::Anon*& meta) {
	bool changed = false;
	ImGui::Columns(3);

	int width = ImGui::GetWindowWidth() - 120 - 10;
	ImGui::SetColumnWidth(0, width * 0.5);
	ImGui::SetColumnWidth(1, width * 0.5);
	ImGui::SetColumnWidth(2, 120);
	ImGui::Text("Name"); ImGui::NextColumn();
	ImGui::Text("Value"); ImGui::NextColumn();
	ImGui::Text("Type"); ImGui::NextColumn();
	ImGui::Separator();

	if (meta) {
		rd::Anon* cur = meta;
		changed |= meta->im(cur);
		meta = cur;
	}
	else {
		ImGui::Button(ICON_MD_ADD);
		rd::Anon* anon = r2::Im::anonContextMenu(nullptr);
		if (anon) meta = anon;
		changed = true;
	}

	ImGui::Columns(1);
	return changed;
}

static const char* noname = "noname";

rd::Anon* r2::Im::anonContextMenu(rd::Anon* old) {
	rd::Anon* metadata = old;
	if (ImGui::BeginPopupContextItem("Create Metadata", 0)) {
		if (ImGui::Selectable("int")) {
			if (!metadata) metadata = new rd::Anon(noname);
			metadata->mkInt(0);
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Selectable("int64")) {
			if (!metadata) metadata = new rd::Anon(noname);
			metadata->mkInt64(0);
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Selectable("uint64")) {
			if (!metadata) metadata = new rd::Anon(noname);
			metadata->mkUInt64(0);
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Selectable("float")) {
			if (!metadata) metadata = new rd::Anon(noname);
			metadata->mkFloat(0);
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Selectable("string")) {
			if (!metadata) metadata = new rd::Anon(noname);
			metadata->mkString("string");
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Selectable("color")) {
			if (!metadata) metadata = new rd::Anon(noname);
			metadata->mkColor(r::Color());
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Selectable("vec2")) {
			if (!metadata) metadata = new rd::Anon(noname);
			metadata->mkVec2(r::Vector2());
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Selectable("vec3")) {
			if (!metadata) metadata = new rd::Anon(noname);
			metadata->mkVec3(r::Vector3());
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Selectable("vec4")) {
			if (!metadata) metadata = new rd::Anon(noname);
			metadata->mkVec4(r::Vector4());
			ImGui::CloseCurrentPopup();
		}
		
		ImGui::EndPopup();
	}
	return metadata;
}


r2::Text * r2::Im::text(const std::string & txt, r2::Node * parent){
	r2::Text * t = rd::Pools::texts.alloc();
	t->nodeFlags |= NF_UTILITY;
	if (parent)parent->addChild(t);
	t->reset();
	t->setFont(r2::GpuObjects::defaultFont);
	t->setText(txt);
	t->setTextColor(r::Color::White);
	depletedNodes.push_back(t);
	return t;
}

r2::Text* r2::Im::fadingText(const std::string& txt, r2::Node* parent, int fadingDurMs){
	r2::Text* t = rd::Pools::texts.alloc();
	t->nodeFlags |= NF_UTILITY;
	if (parent)parent->addChild(t);
	t->resetTRS();
	t->setFont(r2::GpuObjects::defaultFont);
	t->setText(txt);
	t->setTextColor(r::Color::White);
	auto tt = rs::Sys::tw->create(t, TVar::VAlpha, 0.0, TType::TLinear, fadingDurMs);
	tt->onEnd = [t](auto) {
		r2::Im::depletedNodes.push_back(t);
	};
	return t;
}

r2::Graphics* r2::Im::graphics(r2::Node* parent) {
	r2::Graphics* t = rd::Pools::graphics.alloc();
	t->nodeFlags |= NF_UTILITY;
	if (parent) parent->addChild(t);
	depletedNodes.push_back(t);
	return t;
}

r2::Graphics* r2::Im::circle(const r::Vector2& pos, int radius, r2::Node* parent, float thicc,int segs) {
	if (!parent) return nullptr;
	r2::Graphics* t = rd::Pools::graphics.alloc();
	t->nodeFlags |= NF_UTILITY;
	if (parent)parent->addChild(t);
	t->setPosVec2(pos);
	t->drawCircle(0, 0, radius, thicc,segs);
	depletedNodes.push_back(t);
	return t;
}

r2::Graphics *	r2::Im::cross(const r::Vector2 & pos, float radius, r2::Node* parent) {
	if (!parent) return nullptr;
	Scene* sc = parent->getScene();
	if (!sc) return nullptr;
	return r2::Im::cross(pos, radius, 2.0f / sc->cameraScale.x, parent);
}

r2::Graphics* r2::Im::cross(const r::Vector2& pos, float radius, float thicc, r2::Node* parent) {
	if (!parent) return nullptr;
	r2::Graphics* t = rd::Pools::graphics.alloc();
	t->nodeFlags |= NF_UTILITY;
	if (parent) parent->addChild(t);
	t->resetTRS();
	t->clear();
	t->drawCross(pos.x, pos.y, radius, thicc);
	depletedNodes.push_back(t);
	return t;
}

r2::Graphics *	r2::Im::lineRect(const r::Vector2 & tl, const r::Vector2 & size, r::Color col, r2::Node* parent) {
	if (!parent) return nullptr;
	Scene* sc = parent->getScene();
	if (!sc) return nullptr;
	return r2::Im::lineRect(tl, size, 1.0f / sc->cameraScale.x, col, parent);
}

r2::Graphics *	r2::Im::lineRect(const r::Vector2 & tl, const r::Vector2 & size, float thicc, r::Color col, r2::Node* parent) {
	r2::Graphics * t = rd::Pools::graphics.alloc();
	t->nodeFlags |= NF_UTILITY;
	if(parent)parent->addChild(t);
	t->resetTRS();
	t->clear();
	t->setGeomColor(col);
	t->drawHollowRect(tl.x,tl.y, tl.x+size.x,tl.y + size.y, thicc);
	depletedNodes.push_back(t);
	return t;
}


r2::Graphics *	r2::Im::quad(const r::Vector2 & tl, const r::Vector2 & size, r::Color col, r2::Node* parent) {
	r2::Graphics * t = rd::Pools::graphics.alloc();
	t->nodeFlags |= NF_UTILITY;
	if (parent)parent->addChild(t);
	t->resetTRS();
	t->clear();
	t->setGeomColor(col);
	t->drawRect(tl.x, tl.y, tl.x + size.x, tl.y + size.y);
	depletedNodes.push_back(t);
	return t;
}

r2::Graphics * r2::Im::line(const r::Vector2 & from, const r::Vector2 & to, r2::Node* parent) {
	r2::Graphics * t = rd::Pools::graphics.alloc();
	t->nodeFlags |= NF_UTILITY;
	parent->addChild(t);
	Scene* sc = parent->getScene();
	t->drawLine(from.x, from.y, to.x, to.y, 1.0f / sc->cameraScale.x);
	depletedNodes.push_back(t);
	return t;
}

r2::Graphics* r2::Im::line(const r::Vector2& from, const r::Vector2& to, float thicc, r2::Node* parent){
	r2::Graphics* t = rd::Pools::graphics.alloc();
	t->nodeFlags |= NF_UTILITY;
	parent->addChild(t);
	Scene* sc = parent->getScene();
	t->drawLine(from.x, from.y, to.x, to.y, thicc / sc->cameraScale.x);
	depletedNodes.push_back(t);
	return t;
}

r2::Graphics * r2::Im::outerRect(const Bounds & bnds, r::Color col, r2::Node * parent, float thickness) {
	r2::Graphics * t = rd::Pools::graphics.alloc();
	t->nodeFlags |= NF_UTILITY;
	parent->addChild(t);
	t->setGeomColor(col);
	t->drawHollowRect(bnds.left(), bnds.top(), bnds.right(), bnds.bottom(), thickness);
	depletedNodes.push_back(t);
	return t;
}

r2::Graphics* r2::Im::bounds(r2::BatchElem* spr, float thicc) {
	if (!spr)return nullptr;
	if (!spr->batch)return nullptr;
	Scene * sc = spr->batch->getScene();
	if (!sc) return nullptr;
	Bounds lbnd;
	spr->getBounds(lbnd, sc);
	return r2::Im::outerRect(lbnd, r::Color::makeFromHSV(fmodf(rs::Timer::now * 32.f, 360), 1, 1), sc, thicc / sc->cameraScale.x);
}

r2::Graphics * r2::Im::bounds(r2::Node * spr, const r::Color * col, float thicc) {
	if (!spr)return nullptr;
	Scene * sc = spr->getScene();
	if (!sc) return nullptr;
	Bounds lbnd;
	lbnd = spr->getBounds(sc);
	if (spr == sc) {
		Matrix44 local = sc->getLocalMatrix();
		Matrix44 il = local.inverse();
		lbnd.transform(il);
	}
	r::Color nc;
	if (col == nullptr)
		nc = r::Color::makeFromHSV(fmodf(rs::Timer::now*8.f, 360), 1, 1);
	else 
		nc = *col;
	return r2::Im::outerRect(lbnd, nc, sc, thicc / sc->cameraScale.x);
}

r2::Bitmap* r2::Im::bmp(rd::TileLib* lib, const char* group, r2::Node* parent) {
	r2::Bitmap* bmp = rd::Pools::bitmaps.alloc();
	bmp->nodeFlags |= NF_UTILITY;
	if (parent) parent->addChild(bmp);
	bmp->copyTile(r2::GpuObjects::whiteTile);
	lib->getTile(group, 0, 0, 0, bmp->tile);
	depletedNodes.push_back(bmp);
	return bmp;
}

r2::Bitmap* r2::Im::bmp(r2::Tile* tile, r2::Node* parent) {
	r2::Bitmap* bmp = rd::Pools::bitmaps.alloc();
	bmp->nodeFlags |= NF_UTILITY;
	if(parent) parent->addChild(bmp);
	if (!tile)
		bmp->copyTile(r2::GpuObjects::whiteTile);
	else 
		bmp->copyTile(tile);
	depletedNodes.push_back(bmp);
	return bmp;
}


ImSprite::ImSprite(r2::Sprite* spr) {
	t.x = spr->x;
	t.y = spr->y;
	t.z = spr->z;
	r = spr->rotation;
	s.x = spr->scaleX;
	s.y = spr->scaleY;
	zOffsetTop = spr->zTopOffset;
	zOffsetBottom = spr->zBottomOffset;
	blendmode = spr->blendmode;
	depthRead = spr->depthRead;
	depthWrite = spr->depthWrite;
	killAlpha = spr->killAlpha;
	color = spr->color;

	if (spr->parent) {
		spr->parent->syncMatrix();
		parentMatrix = spr->parent->getGlobalMatrix();
	}
	else
		parentMatrix = Matrix44::identity;

	r2::Scene* sc = spr->getScene();
	if (sc)
		viewMatrix = sc->getViewMatrix();
	else
		viewMatrix = Matrix44::identity;
}

void r2::ImSprite::im()
{
	using namespace ImGui;
	Value("t", t);
	Value("r", r);
	Value("s", s);
	Value("zOffsetTop", zOffsetTop);
	Value("zOffsetBottom", zOffsetBottom);
	Value("blendmode", blendmode);
	Value("color.r", color.r);
	Value("color.g", color.g);
	Value("color.b", color.b);
	Value("color.a", color.a);
	Value("parentMatrix", parentMatrix);
	Value("viewMatrix", viewMatrix);
	Value("depthRead", depthRead);
	Value("depthWrite", depthWrite);
	Value("killAlpha", killAlpha);
}

bool r2::Im::draw(rs::GfxContext * gfx, const ImSprite & spr)
{
	r2::Tile * tile = spr.tile;
	if (!tile) return false;
	Pasta::Texture * tex = tile->getTexture();
	if (!tex) return false;

	Pasta::Graphic * g = Pasta::Graphic::getMainGraphic();
	Pasta::GraphicContext * ctx = Pasta::GraphicContext::GetCurrent();

	gfx->push();
	g->pushContext();

	Pasta::DepthStateID did = GpuObjects::depthNoSupport;
	if (spr.depthRead && spr.depthWrite)	did = GpuObjects::depthReadWrite;
	else if (spr.depthRead)					did = GpuObjects::depthReadOnly;
	else if (spr.depthWrite)				did = GpuObjects::depthWriteOnly;
	g->setDepthState(did);

	g->setTransparencyType(spr.blendmode);
	g->applyContextTransparency();
	g->setVertexDeclaration(Pasta::VD_POSITIONS | Pasta::VD_TEXCOORDS);
	g->setTexture(Pasta::ShaderStage::Fragment, 0, tex);
	g->applyContextTransparency();

	uint32_t flags = 0;

	flags |= Pasta::Graphic::BasicShaderFlags::BSF_TEXTURE;
	if (tex->isPremultipliedAlpha())
		flags |= Pasta::Graphic::BasicShaderFlags::BSF_PREMUL_ALPHA;
	if( spr.killAlpha )
		flags |= Pasta::Graphic::BasicShaderFlags::BSF_KILL_ALPHA;
	//todo add this support
	//g->setColorRGBA((Pasta::Color)spr.color);
	g->setShader(g->getBasicShader(flags));

	Pasta::Matrix44 trs = Pasta::Matrix44::identity;
	trs.setScale(spr.s.x, spr.s.y, 1.0f);
	trs.rotateZ(PASTA_RAD2DEG(spr.r));
	trs.translate(Vector3(spr.t.x, spr.t.y,0));
	trs = spr.parentMatrix * trs;

	//snatch the good view matrix
	gfx->loadViewMatrix(spr.viewMatrix);
	gfx->loadModelMatrix(trs);
	gfx->loadProjMatrix(gfx->projMatrix);

	const int stride = 3 + 2;
	float vertexData[4 * stride];//pos uv
	memset(vertexData, 0, 4 * stride * sizeof(float));
	uint32_t texCoordOffset = 3;//after pos

	float z = spr.t.z;

	{
		float *pos0 = &vertexData[0 * stride];
		float *pos1 = &vertexData[1 * stride];
		float *pos2 = &vertexData[2 * stride];
		float *pos3 = &vertexData[3 * stride];

		float v0x = tile->dx;
		float v1x = tile->dx + tile->width;

		float v0y = tile->dy;
		float v1y = tile->dy + tile->height;

		pos0[0] = v0x;
		pos0[1] = v0y ;
		pos0[2] = z + spr.zOffsetTop;

		pos1[0] = v1x;
		pos1[1] = v0y;
		pos1[2] = z + spr.zOffsetTop;

		pos2[0] = v0x;
		pos2[1] = v1y ;
		pos2[2] = z + spr.zOffsetBottom;

		pos3[0] = v1x;
		pos3[1] = v1y;
		pos3[2] = z + spr.zOffsetBottom;
	}

	// texcoords
	{
		float *tex0 = &vertexData[0 * stride + texCoordOffset];
		float *tex1 = &vertexData[1 * stride + texCoordOffset];
		float *tex2 = &vertexData[2 * stride + texCoordOffset];
		float *tex3 = &vertexData[3 * stride + texCoordOffset];

		const float u1 = tile->u1;
		const float v1 = tile->v1;
		const float u2 = tile->u2;
		const float v2 = tile->v2;

		tex0[0] = u1; tex0[1] = v1;
		tex1[0] = u2; tex1[1] = v1;
		tex2[0] = u1; tex2[1] = v2;
		tex3[0] = u2; tex3[1] = v2;
	}

	g->drawPrimitives(Pasta::PT_TRIANGLE_STRIP, 2, vertexData);

	g->setVertexDeclaration(0);
	g->setTexture(Pasta::ShaderStage::Fragment, 0, NULL);

	g->popContext();
	gfx->pop();

	return true;
}

void r2::Im::enterFrame(){
}

void r2::Im::exitFrame(){
	for (r2::Node * t : depletedNodes)
		rd::Pools::safeFree(t);
	depletedNodes.clear();

	for (r2::BatchElem* t : depletedElems)
		rd::Pools::free(t);
	depletedElems.clear();
}

static int StdStrResizeCallback(ImGuiInputTextCallbackData* data){
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize){
		std::string * my_str = (std::string *)data->UserData;
		my_str->resize(data->BufTextLen);
		data->Buf = (char*)my_str->c_str();
	}
	/*
	else if (data->ChainCallback) {
		// Forward to user callback, if any
		data->UserData = data->ChainCallbackUserData;
		return user_data->ChainCallback(data);
	}
	*/
	return 0;
};

static int StrStrResizeCallback(ImGuiInputTextCallbackData* data){
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize){
		Str * my_str = (Str *)data->UserData;
		int size = (data->BufSize <= 0) ? 0 : data->BufSize;
		if(my_str->capacity() <= (size+1))
			my_str->reserve(size+1); // NB: On resizing calls, generally data->BufSize == data->BufTextLen + 1
		data->Buf = my_str->c_str();
	}
	return 0;
};

namespace ImGui {

	//void Text(std::string & str) {
	//	ImGui::Text( str.c_str());
	//};

	void Image(r2::Tile* tile, const ImVec2& size, const r::Color & tint)
	{
		if (!tile) {
			ImGui::Error("No tile to display");
			return;
		}
		Pasta::ShadedTexture* st = rd::Pools::allocForFrame();
		st->texture = tile->getTexture();
		ImVec2 uv0(tile->u1, tile->v1);
		ImVec2 uv1(tile->u2, tile->v2);
		ImGui::Image((ImTextureID)st, size, uv0, uv1, tint.toVec4());
	}

	void Text(const std::string& label) {
		ImGui::Text(label.c_str());
	}

	void Text(const Str& label) {
		ImGui::Text(label.c_str());
	}

	bool InputText(const std::string& label, std::string& str, ImGuiInputTextFlags flags ) {
		return InputText(label.c_str(), str, flags);
	}


	bool InputText(const char* label, std::string & str, ImGuiInputTextFlags flags) {
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		return InputText(label, (char*)str.c_str(), str.capacity() + 1, flags, StdStrResizeCallback, &str);
	};

	bool InputText(const char* label, Str& str, ImGuiInputTextFlags flags) {
		int cap = str.capacity();
		bool changed = ImGui::InputText(label, &str[0], cap, ImGuiInputTextFlags_CallbackResize | flags, StrStrResizeCallback, &str);
		return changed;
	};

	bool InputTextMultiline(const char* label, std::string & str, const ImVec2& size) {
		ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize;
		return InputTextMultiline(label, (char*)str.c_str(), str.capacity() + 1, size, flags, StdStrResizeCallback, &str);
	};

	bool Button(const std::string & label) {
		return ImGui::Button(label.c_str());
	};

	bool Matrix(const char* label, r::Matrix44 & mat) {
		ImGui::Text(label);
		bool touched0 = ImGui::SliderFloat4("line 0", mat.values	 , -100, 100);
		bool touched1 = ImGui::SliderFloat4("line 1", mat.values + 4 , -100, 100);
		bool touched2 = ImGui::SliderFloat4("line 2", mat.values + 8 , -100, 100);
		bool touched3 = ImGui::SliderFloat4("line 3", mat.values + 12, -100, 100);
		return touched0 || touched1 || touched2 || touched3;
	};

	bool DragDouble(const char* label, double* val, double vspeed, double vmin, double vmax, const char* format){
		return DragScalarN(label, ImGuiDataType_::ImGuiDataType_Double, val, 1, vspeed, &vmin, &vmax, format);
	}
	
	bool DragDouble2(const char * label, double * val, double vspeed, double vmin, double vmax, const char* format){
		return DragScalarN(label, ImGuiDataType_::ImGuiDataType_Double, val, 2, vspeed, &vmin, &vmax, format);
	};

	bool DragDouble4(const char * label, double * val, double vspeed, double vmin, double vmax, const char* format){
		return DragScalarN(label, ImGuiDataType_::ImGuiDataType_Double, val, 4, vspeed, &vmin, &vmax, format);
	}

	bool SliderDouble(const char* label, double* v, double v_min, double v_max, const char* format, ImGuiSliderFlags fl) {
		return SliderScalarN(label, ImGuiDataType_::ImGuiDataType_Double, v, 1, &v_min, &v_max, format, fl);
	};

	bool Selectable(const std::string& str,bool * selected ){
		return ImGui::Selectable(str.c_str(),selected);
	};

	bool DragFloat3Col(const r::Color& col, const char* label, float v[3], float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, col.toVec4());
		bool res = ImGui::DragFloat3(label, v, v_speed, v_min, v_max, format, flags = 0);
		ImGui::PopStyleColor();
		return res;
	};

	void PushTextColor(const r::Color& col)	{
		PushStyleColor(ImGuiCol_::ImGuiCol_Text, col.toVec4());
	};

	void PopTextColor() {
		PopStyleColor();
	};

	
	bool Vertex3dList(rd::Anon& holder) {
		ImGui::PushItemWidth(180);
		if (!holder.hasValue()) {
			Vector3 v0 = {};
			holder.setVertex3Buffer(v0.ptr(),1);
		}
		int nb = holder.getSize() / 3;
		bool changed = false;
		for (int i = 0; i < nb; ++i) {
			string name = "vtx#" + to_string(i);
			ImGui::PushID(i);
			if (ImGui::DragFloat3(name.c_str(), &holder.asFloatBuffer()[i * 3],0.1f, -100, 100)) {
				//changed?
				changed = true;
			}
			ImGui::SameLine();
			if (ImGui::Button(ICON_MD_DELETE)) {
				vector<float> nbuf = holder.getFloatBufferCopy();
				auto eraseStart = nbuf.begin() + 3 * i;
				auto eraseEnd = eraseStart + 3;
				nbuf.erase(eraseStart, eraseEnd);
				holder.setFloatBuffer(nbuf.data(), nbuf.size());
				i--;
				nb--;
			}
			ImGui::PopID();
		}
		if (ImGui::Button("+")) {
			holder.reserve(holder.getByteSizeCapacity() + 3 * 4);
			holder.asFloatBuffer()[holder.getSize() - 3] = 0.0;
			holder.asFloatBuffer()[holder.getSize() - 2] = 0.0;
			holder.asFloatBuffer()[holder.getSize() - 1] = 0.0;
			changed = true;
		}
		ImGui::PopItemWidth();
		return changed;
	}

	bool Box3dList(rd::Anon& holder) {
		ImGui::PushItemWidth(180);
		int vertexPerBox = 3;
		int floatPerVertex = 3;
		if (!holder.hasValue()) {
			float boxIdentity[] = {
				0.5,0.5,0.5,//half extent
				0,0,0,//trans id
				1,1,1//scale id
			};
			holder.setVertex3Buffer(boxIdentity, vertexPerBox);
		}
		int floatsPerBox = floatPerVertex * vertexPerBox;
		int nbBox = holder.getSize() / floatsPerBox;
		bool changed = false;
		for (int i = 0; i < nbBox; ++i) {
			string name = "box#" + to_string(i);
			ImGui::PushID(i);

			float* boxStart		= holder.asFloatBuffer() + i * floatsPerBox;
			float* boxExtent	= boxStart;
			float* boxTrans		= boxStart + 3;
			float* boxScale		= boxStart + 6;

			if (ImGui::DragFloat3("Box Extent - radius - (local coords)", boxExtent, 0.1f, -100, 100))
				changed = true;

			if (ImGui::DragFloat3("Box Trans (world coords)", boxTrans, 0.1f, -1000, 1000))
				changed = true;

			if (ImGui::DragFloat3("Box Scale (world coords)", boxScale, 0.1f, -1000, 1000))
				changed = true;
			
			ImGui::SameLine();

			if (ImGui::Button(ICON_MD_DELETE)) {
				vector<float> nbuf = holder.getFloatBufferCopy();
				auto eraseStart = nbuf.begin() + floatsPerBox * i;
				auto eraseEnd = eraseStart + floatsPerBox;
				nbuf.erase(eraseStart, eraseEnd);
				holder.setVertex3Buffer(nbuf.data(), nbuf.size() / vertexPerBox);
				i--;
			}
			ImGui::PopID();
		}
		if (ImGui::Button("+")) {
			holder.reserve(holder.getByteSizeCapacity() + floatsPerBox * sizeof(float) );
			float* end = holder.asFloatBuffer() + holder.getSize();
			end -= floatsPerBox;

			end[0] = end[1] = end[2] = 0.5f;
			end[3] = end[4] = end[5] = 0;
			end[6] = end[7] = end[8] = 1.0f;
			
			changed = true;
		}
		ImGui::PopItemWidth();
		return changed;
	}


}

static bool dbgSkip = false;

//assumes matrixes are ok
bool r2::Im::draw(rs::GfxContext * gfx, const Pasta::Matrix44 & trs, r2::Tile * tile) {
	if (!tile) return false;

	Pasta::Texture * tex = tile->getTexture();
	if (!tex) return false;

	if (dbgSkip)
		return false;

	Pasta::Graphic * g = Pasta::Graphic::getMainGraphic();
	Pasta::GraphicContext * ctx = Pasta::GraphicContext::GetCurrent();

	gfx->push();

	g->setDepthState(r2::GpuObjects::depthNoSupport);
	g->setTransparencyType(r::TransparencyType::TT_ALPHA);
	g->applyContextTransparency();
	g->setVertexDeclaration(Pasta::VD_POSITIONS | Pasta::VD_TEXCOORDS);
	g->setTexture(Pasta::ShaderStage::Fragment, 0, tex);
	g->applyContextTransparency();

	uint32_t flags = 0;

	flags |= Pasta::Graphic::BasicShaderFlags::BSF_TEXTURE;
	if (tex->isPremultipliedAlpha())
		flags |= Pasta::Graphic::BasicShaderFlags::BSF_PREMUL_ALPHA;

	g->setShader(g->getBasicShader(flags));

	//snatch the good view matrix
	gfx->loadModelMatrix(trs);
	gfx->loadViewMatrix(gfx->viewMatrix);
	gfx->loadProjMatrix(gfx->projMatrix);
	gfx->setViewport(gfx->vpx, gfx->vpy, gfx->vpw, gfx->vph);

	const int stride = 3 + 2;
	float vertexData[4 * stride];//pos uv
	memset(vertexData, 0, 4 * stride * sizeof(float));
	uint32_t texCoordOffset = 3;//after pos

	{
		float *pos0 = &vertexData[0 * stride];
		float *pos1 = &vertexData[1 * stride];
		float *pos2 = &vertexData[2 * stride];
		float *pos3 = &vertexData[3 * stride];

		float v0x = tile->dx;
		float v1x = tile->dx + tile->width;

		float v0y = tile->dy;
		float v1y = tile->dy + tile->height;

		pos0[0] = v0x;
		pos0[1] = v0y;
		pos0[2] = 0;

		pos1[0] = v1x;
		pos1[1] = v0y;
		pos1[2] = 0;

		pos2[0] = v0x;
		pos2[1] = v1y;
		pos2[2] = 0;

		pos3[0] = v1x;
		pos3[1] = v1y;
		pos3[2] = 0;
	}

	// texcoords
	{
		float *tex0 = &vertexData[0 * stride + texCoordOffset];
		float *tex1 = &vertexData[1 * stride + texCoordOffset];
		float *tex2 = &vertexData[2 * stride + texCoordOffset];
		float *tex3 = &vertexData[3 * stride + texCoordOffset];

		tex0[0] = tile->u1; tex0[1] = tile->v1;
		tex1[0] = tile->u2; tex1[1] = tile->v1;
		tex2[0] = tile->u1; tex2[1] = tile->v2;
		tex3[0] = tile->u2; tex3[1] = tile->v2;
	}

	g->drawPrimitives(Pasta::PT_TRIANGLE_STRIP, 2, vertexData);

	g->setVertexDeclaration(0);
	g->setTexture(Pasta::ShaderStage::Fragment, 0, NULL);

	gfx->pop();


	return true;
}

bool r2::Im::imLibName(std::string& lib)
{
	using namespace ImGui;
	std::vector<const char*> allLibNames;
	for (auto iter : r2::im::TilePicker::sources) 
		allLibNames.push_back(iter->name.c_str());
	sort(allLibNames.begin(), allLibNames.end(), [](const char * a0,const char * a1) {
		return strcmp(a0, a1) < 0;
	});

	int idx = -1;
	int i = 0;
	for (auto& s : allLibNames) {
		if (0 == strcmp(s, lib.c_str())) 
			idx = i;
		i++;
	}
	if (idx == -1) {
		ImGui::Text("current value : > %s > but no such library loaded", lib.c_str());
		if (Button("FIX by using first lib ?")) {
			lib = allLibNames[0];
		}
		if (Button("FIX by loading ?")) {
			static string err;
			if(!(r2::im::TilePicker::getOrLoadLib(lib))) {
				err = "Cannot load library...";
			}
			if (err.length()) {
				if (ImGui::Begin("Alert")) {
					ImGui::Text(err.c_str());
					if (Button("OKAY")) {
						err = "";
					}
				}
				ImGui::End();
			}
		}
	}
	else
		if (Combo("lib", &idx, allLibNames.data(), allLibNames.size())) {
			lib = allLibNames[idx];
			return true;
		}
	return false;
}

bool r2::Im::imTagsReadOnly(const std::string& tags){
	using namespace ImGui;
	static string tmp;
	std::vector<string> all = rd::String::split(tags, ',');
	bool someDisplayed = false;
	for (auto& s : all) {
		if (s.length() == 0)continue;
		someDisplayed = true;
		ImGui::Text(s.c_str());
		SameLine();
	}
	if (!someDisplayed) {
		ImGui::Text("No tags currently");
	}
	return false;
}

bool r2::Im::imTags(string& tags) {
	using namespace ImGui;
	static string tmp;
	std::vector<string> all = rd::String::split(tags, ',');
	bool someDisplayed = false;
	for (auto& s : all) {
		if (s.length() == 0)continue;
		someDisplayed = true;
		if (SmallButton(s.c_str())) {
			//remove
			auto pos = std::find(all.cbegin(), all.cend(), s);
			if (pos != all.cend()) {
				all.erase(pos);
				tags = rd::String::join(all, ",");
			}
			return true;
		}
	}
	if (!someDisplayed) 
		ImGui::Text("No tags currently");
	InputText("new tag",tmp);
	SameLine();
	if (Button(ICON_MD_ADD)) {
		if (tags.length()) 
			tags+=(",");
		tags+=(tmp);
		tmp = "";
		return true;
	}
	return false;
}

bool r2::Im::imTileName(std::string& tile,const char * libName){
	using namespace ImGui;

	PushID(&tile);
	std::vector<const char*> allTileNames;
	static bool onlyAnims = true;
	static bool showPreview = true;
	auto lib = r2::im::TilePicker::getLib(libName);
	if (!lib)
		return false;

	static string filter;
	Indent();
	InputText("filter", filter);
	Checkbox("show Only Anims", &onlyAnims);
	Checkbox("show Preview", &showPreview);
	if (showPreview) {
		auto lib = r2::im::TilePicker::getLib(libName);
		if (lib->isAnim(tile.c_str())) {
			auto group = lib->getGroup(tile.c_str());
			r2::Tile* t = lib->getTile(tile.c_str(), group->anim[0]);
			if (t) {
				ImGui::Image(t, ImVec2(t->width, t->height));
				t->destroy();
			}
		}
		else {
			r2::Tile* t = lib->getTile(tile.c_str());
			if (t) {
				ImGui::Image(t, ImVec2(t->width, t->height));
				t->destroy();
			}
		}
	}
	Unindent();
	for (auto& p : lib->groups) {
		auto group = p.second;
		if( onlyAnims && !group->anim.size())
			continue;
		if( filter.length() && !rd::String::containsI(group->id.c_str(),filter.c_str()))
			continue;
		allTileNames.push_back(group->id.c_str());
	}
	sort(allTileNames.begin(), allTileNames.end(), [](const char* a0, const char* a1) {
		return strcmp(a0, a1) < 0;
	});

	int idx = -1;
	int i = 0;
	for (auto& s : allTileNames) {
		if (0 == strcmp(s, tile.c_str()))
			idx = i;
		i++;
	}
	if (Combo("tile", &idx, allTileNames.data(), allTileNames.size())) {
		tile = allTileNames[idx];
		PopID();
		return true;
	}
	PopID();
	return false;
}

//assumes matrixes are ok
bool r2::Im::draw(rs::GfxContext * gfx, const Pasta::Vector3& t, const Pasta::Vector3& rs, r2::Tile * tile) {
	Pasta::Matrix44 mat;
	mat.setScale(rs.x, rs.y, 1.0f);
	mat.rotateZ(PASTA_RAD2DEG(rs.z));
	mat.translate(t);
	return draw(gfx, mat, tile);
}

void ImGui::TextHint(const char* label, r::Color col, const char* tip) {
	PushStyleColor(ImGuiCol_::ImGuiCol_Text, col.toVec4());
	Text(label);
	PopStyleColor();
	if (IsItemHovered())
		ImGui::SetTooltip(tip);
}

void ImGui::Warning(const char* tip) {
	TextHint(ICON_MD_WARNING, r::Color(Pasta::Color::Orange), tip);
}

void ImGui::Warning(const std::string& tip) {
	Error(tip.c_str());
}


void ImGui::Error(const char* tip) {
	TextHint(ICON_MD_WARNING, r::Color(Pasta::Color::Red), tip);
}

void ImGui::Error(const std::string & tip) {
	Error(tip.c_str());
}


void ImGui::Value(const char* prefix, double d) {
	ImGui::Text("%s: %f", prefix, d);
}

void ImGui::Value(const char* prefix, r::u64 id){
	ImGui::Text("%s: %lu", prefix, id);
}

void ImGui::Value(const char* prefix, const std::string& str) {
	ImGui::Text("%s: %s", prefix, str.c_str());
}

void ImGui::Value(const char* prefix, const Str& str){
	ImGui::Text("%s: %s", prefix, str.c_str());
}

void ImGui::Value(const char* prefix, const r2::Bounds& obj){
	ImGui::Text("%s:", prefix);
	ImGui::Text("x:%lf y:%lf",obj.left(),obj.top());
	ImGui::Text("right:%lf bottom:%lf",obj.right(),obj.bottom());
	ImGui::Text("width:%lf height:%lf", obj.width(), obj.height());
}

void ImGui::Value(const char* prefix, const Vector2i& v){
	ImGui::Text("x:%d y:%d", v.x,v.y);
}


bool ImGui::TreeNode(const std::string& txt){
	return ImGui::TreeNode(txt.c_str());
}

bool ImGui::TreeNode(const Str& txt) {
	return ImGui::TreeNode(txt.c_str());
}


static bool eventPickerOpened = false;
std::optional<std::pair<std::string, std::string>> ImGui::AudioPicker(bool edit) {
	using namespace ImGui;
	if (Button(edit ? ICON_MD_EDIT "Pick audio" : ICON_MD_ADD "Add audio")) {
		ImGui::OpenPopup("Pick audio event");
		eventPickerOpened = true;
	}

	if (eventPickerOpened) {
		ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
		if (BeginPopupModal("Pick audio event", &eventPickerOpened)) {
			AudioMan& sm = AudioMan::get();
			if (!sm.banks.size())
				Text("No banks loaded, open Audio Explorer ( F3 )");
			std::string resBnk;
			std::string resEv;
			bool found = false;
			for (auto& bnk : sm.banks) {
				if (strstr(bnk.first.c_str(), ".strings")) continue;
				if (CollapsingHeader(bnk.first.c_str())) {
					eastl::vector<string> events = sm.getEventList(bnk.first.c_str());
					for (int ev = 0; ev < events.size(); ++ev) {
						if (Selectable(events[ev].c_str(), false)) {
							resBnk = bnk.first;
							resEv = events[ev];
							found = true;
							break;
						}
					}
				}
			}
			EndPopup();
			if (found) {
				eventPickerOpened = false;
				return std::make_optional(std::make_pair(resBnk, resEv));
			}
		}
	}
	return std::nullopt;
}

bool r2::Im::imSFX(string& sfx)
{
	using namespace ImGui;

	if (sfx.length()) {
		ImGui::Text(sfx.c_str()); SameLine();
	}
	if (Button(ICON_MD_ADD "Set audio")) {
		ImGui::OpenPopup("Pick audio event");
		eventPickerOpened = true;
	}

	if (eventPickerOpened) {
		ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
		if (BeginPopupModal("Pick audio event", &eventPickerOpened)) {
			AudioMan& sm = AudioMan::get();
			if (!sm.banks.size())
				ImGui::Text("No banks loaded, open Audio Explorer ( F3 )");
			std::string resBnk;
			std::string resEv;
			bool found = false;
			for (auto& bnk : sm.banks) {
				if (strstr(bnk.first.c_str(), ".strings")) continue;
				if (CollapsingHeader(bnk.first.c_str())) {
					eastl::vector<string> events = sm.getEventList(bnk.first.c_str());
					for (int ev = 0; ev < events.size(); ++ev) {
						if (Selectable(events[ev].c_str(), false)) {
							resBnk = bnk.first;
							resEv = events[ev];
							found = true;
							break;
						}
					}
				}
			}
			ImGui::EndPopup();
			if (found) {
				eventPickerOpened = false;
				sfx = resEv;
				return true;
			}
		}
	}
	return false;
}


void r2::Im::previewPixels(Pasta::TextureData* data, int size) {
	Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
	TransientTexture* tt = nullptr;
	for (auto& t : RscLib::transientTex) {
		if (t.data == data) {
			tt = &t;
			break;
		}
	}
	int w = 0;
	int h = 0;

	if(tt){
		int tw = 0;
		data->getRawTexel(0, 0, tw);
		auto crc = Checksum::CRC32(data->images[0].pixels[0], data->width*data->height*tw);
		if (crc != tt->crc) {
			for (auto iter = RscLib::transientTex.begin(); iter != RscLib::transientTex.end();) {
				auto& tt = *iter;
				if (tt.data == data) {
					iter = RscLib::transientTex.erase(iter);
				}
				else 
					iter++;
			}
			ctx->DestroyTexture(tt->tex);
			tt = nullptr;
		}
	}
	if (!tt) {
		TransientTexture nt;
		RscLib::transientTex.push_back(nt);
		tt = &RscLib::transientTex.back();

		int tw;
		data->getRawTexel(0, 0, tw);
		auto crc = Checksum::CRC32(data->images[0].pixels[0], data->width * data->height * tw);
		tt->crc = crc;
		tt->data = data;
		Pasta::Texture* tex = ctx->CreateTexture(tt->data, PASTA_TEXTURE_POINT);
		tt->tex = tex;
		trace(std::string("creating transient texture from tex data CRC32 : ") + to_string(crc));
	}
	
	if (!tt)
		return;
	w = tt->data->width;
	h = tt->data->height;
	if (!w || !h)
		return;
	float r = 1.0f * w / h;
	ImGui::Image(r2::Tile::fromTexture(tt->tex), ImVec2(size, size*r));
}

void r2::Im::previewPixels(int w, int h, const r::u8* data, int size){
	if (!w || !h)
		return;
	auto crc = Checksum::CRC32(data, size);
	TransientTexture* tt = nullptr;
	for( auto & t : RscLib::transientTex){
		if (t.crc == crc) {
			tt = &t;
			break;
		}
	}
	if(!tt){
		TransientTexture nt;
		RscLib::transientTex.push_back(nt);
		tt = &RscLib::transientTex.back();
		tt->crc = crc;
		auto td = tt->data = new Pasta::TextureData();
		td->width = w;
		td->height = h;
		td->images.resize(1);
		td->images[0].pixels[0] = malloc(size);
		td->images[0].sizes[0] = size;
		memcpy(
			td->images[0].pixels[0],
			data,
			size
		);
		Pasta::GraphicContext* ctx = Pasta::GraphicContext::GetCurrent();
		Pasta::Texture* tex = ctx->CreateTexture(tt->data, PASTA_TEXTURE_POINT);
		tt->tex = tex;
		trace( std::string("creating transient texture from data CRC32 : ") + to_string(crc));
	}
	ImGui::Image(r2::Tile::fromTexture(tt->tex), ImVec2(w,h));
}