#pragma once

#include <string>
#include <optional>

namespace rd { class TileLib; }

namespace r2 {

	struct ImSprite {
		r::Vector3				t = r::Vector3(0, 0, 0);
		float					r = 0.0f;
		r::Vector2				s = r::Vector2(1, 1);
		float					zOffsetTop = 0.0f;
		float					zOffsetBottom = 0.0f;
		r::TransparencyType		blendmode = r::TransparencyType::TT_ALPHA;
		r::Color				color;
		r2::Tile*				tile = nullptr;
		Matrix44				parentMatrix;
		Matrix44				viewMatrix;

		bool					depthRead = false;
		bool					depthWrite = false;
		bool					killAlpha = false;

		ImSprite() {};
		ImSprite(r2::Sprite * str);
		void im();
	};
	
	class Im {
		public:
			static r2::Bitmap*		bmp(r2::Tile* tile, r2::Node* parent);
			static r2::Bitmap*		bmp(rd::TileLib * lib, const char * group, r2::Node* parent);
			static r2::Text*		text( const std::string & txt, r2::Node* parent );
			static r2::Text*		fadingText(const std::string& txt, r2::Node* parent, int fadingDurMs = 1000);

			static r2::Graphics *	graphics(r2::Node* parent);
			static r2::Graphics*	circle(const r::Vector2& pos, int radius, r2::Node* parent = 0, float thicc = 1.0, int segs = 0);

			static r2::Graphics *	cross( const r::Vector2 & pos, float radius, r2::Node* parent);
			static r2::Graphics *	cross( const r::Vector2 & pos, float radius, float thicc, r2::Node* parent);
			static r2::Graphics *	line(const r::Vector2 & from, const r::Vector2 & to, r2::Node* parent);
			static r2::Graphics *	line(const r::Vector2 & from, const r::Vector2 & to, float thicc, r2::Node* parent);

			static r2::Graphics *	lineRect(const r::Vector2 & tl, const r::Vector2 & size, r::Color col = r::Color(), r2::Node* parent = nullptr);
			static r2::Graphics *	lineRect(const r::Vector2 & tl, const r::Vector2 & size, float thicc, r::Color col = r::Color(), r2::Node* parent = nullptr);
			static r2::Graphics *	quad(const r::Vector2 & tl, const r::Vector2 & size, r::Color col = r::Color(), r2::Node* parent = nullptr);

			static r2::Graphics *	outerRect( const Bounds & bnds, r::Color col = r::Color(), r2::Node* parent = nullptr, float thickness = 2.0F);
			static r2::Graphics *	bounds(r2::BatchElem *spr , float thicc = 1.0f);

			static r2::Graphics *	bounds(r2::Node *spr, const r::Color * col=nullptr, float thicc = 1.0f);

			//draw in dedicated context
			static bool				draw(rs::GfxContext * gfx, const ImSprite & spr);
			static bool				draw(rs::GfxContext * gfx, const Pasta::Vector3 & t, const Pasta::Vector3 & rs, r2::Tile * tile);
			static bool				draw(rs::GfxContext * gfx, const Pasta::Matrix44  & trs, r2::Tile * tile);

			static void				enterFrame();
			static void				exitFrame();

			static bool				metadata(rd::Anon*& meta);
			static rd::Anon*		anonContextMenu(rd::Anon* old);

			static bool				imColorMatrix(ColorMatrixControl& ctrl);
			static bool				imColorMatrix(r::uid id, Matrix44& mat);
			static void				imTextureInfos(Pasta::Texture* t);
			static void				imTw(rd::Tweener* t);
			static void				imNodeListEntry(const char * label, Node * e);
			static void				imElemEntry(const char * label, r2::BatchElem * be);
			static inline void		imNodeListEntry(const std::string& label, Node* e) { imNodeListEntry(label.c_str(), e); };
			static bool				imLibName(std::string& lib);
			static bool				imTileName(std::string& tile,const char * lib);
			static bool				imTags(std::string& tags);
			static bool				imTagsReadOnly(const std::string& tags);
			static bool				imSFX(std::string& sfx);
			static bool				imBlendmode(r::TransparencyType & t);
			static bool				imTextureData(Pasta::TextureData * data);

			static void				previewPixels(Pasta::TextureData* data, int size);

			static void				previewPixels(int w, int h, const r::u8 * data, int size);

			static void				keepAlive(r2::Node&n);
			static void				keepAlive(r2::BatchElem&be);

			//will be sent back to pools at end of frame
			static eastl::vector<r2::Node*>			depletedNodes;
			static eastl::vector<r2::BatchElem*>	depletedElems;

			static const char*		blends[r::TransparencyType::TRANSPARENCY_TYPE_COUNT];
			static const char*		filters[(u32)r2::FilterType::FT_COUNT];
			static const char*		modeMatrix[(u32)r2::ColorMatrixMode::CMM_Count];
			static const char*		texFilters[r2::TexFilter::TF_Count];

			
	};
}

namespace ImGui {
	template<typename Ty>
	inline void Value(const char* label, std::optional<Ty>& opt) {
		if (opt)
			Value(label, (Ty) *opt);
		else
			LabelText(label, "<nullopt>");
	};

	template<typename Ty>
	inline bool DragInt(const char* label, std::optional<Ty>& opt,float sp=1.0f, int mn = 0,int mx = 0) {
		bool hasValue = opt != std::nullopt;
		bool changed = false;
		if (Checkbox(label, &hasValue)) {
			if (hasValue) {
				opt = (mn + mx) >> 1;
			}
			else {
				opt = std::nullopt;
			}
			changed = true;
		}
		if (hasValue) {
			changed|=DragInt(label, &(*opt), sp, mn, mx);
		}
		return changed;
	};

	void Image(r2::Tile*tile, const ImVec2&size,const r::Color & tint = r::Color::White);

	void Text(const std::string & label);
	void Text(const Str& label);
	inline void SetTooltip(const Str& label)								{ SetTooltip(label.c_str()); };
	inline void SetTooltip(const std::string & label)						{ SetTooltip(label.c_str()); };
	bool InputText(const std::string & label, std::string & str, ImGuiInputTextFlags flags = 0);
	bool InputText(const char* label, std::string & str, ImGuiInputTextFlags flags = 0);
	bool InputText(const char* label, Str & str, ImGuiInputTextFlags flags = 0);
	bool InputTextMultiline(const char* label, std::string & str, const ImVec2& size = ImVec2(0, 0));
	bool Button( const std::string & label);
	bool Matrix(const char* label, r::Matrix44 & mat);

	bool DragDouble	(const char* label,	double * val, double vspeed=0.1, double vmin=0.0, double vmax=1000.0, const char* format = NULL);
	bool DragDouble2(const char* label, double * val, double vspeed=0.1, double vmin=0.0, double vmax=1000.0, const char* format = NULL);
	bool DragDouble4(const char* label, double * val, double vspeed=0.1, double vmin=0.0, double vmax=1000.0, const char* format = NULL);

	inline bool DragDouble(const std::string& label, double* val, double vspeed = 0.1, double vmin = 0.0, double vmax = 1000.0, const char* format = NULL) {
		return DragDouble(label.c_str(), val, vspeed, vmin, vmax, format);
	};

	bool SliderDouble(const char* label, double* v, double v_min, double v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);

	bool Selectable(const std::string & str,bool * selected);
	inline bool Selectable(const Str& str, bool* selected) { return Selectable(str.c_str(), selected); };

	bool DragFloat3Col(const r::Color & col, const char* label, float v[3], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);

	void PushTextColor(const r::Color& col);
	void PopTextColor();

	bool Vertex3dList(rd::Anon& holder);

	bool Box3dList(rd::Anon& holder);

	void TextHint(const char * label, r::Color col, const char * tip);

	void Warning(const char* tip);
	void Warning(const std::string& tip);

	void Error(const char * tip);
	void Error(const std::string & tip);
	
	void Value(const char* prefix, double val);
	void Value(const char* prefix, r::u64 id);
	void Value(const char* prefix, const Str & str);
	void Value(const char* prefix, const std::string & str);
	void Value(const char* prefix, const r2::Bounds &obj);
	void Value(const char* prefix, const Vector2i &v);
	
	
	inline bool ColorEdit4(const char* prefix, r::Color& c, ImGuiColorEditFlags fl=0){
		return ColorEdit4(prefix, c.ptr(), fl);
	}
	
	inline bool ColorEdit3(const char* prefix, r::Color& c, ImGuiColorEditFlags fl=0) {
		return ColorEdit3(prefix, c.ptr(), fl);
	}

	bool TreeNode(const std::string & txt);
	bool TreeNode(const Str & txt);
	
	std::optional<std::pair<std::string,std::string>> AudioPicker(bool edit = true);
	//cuz sol thing
	//void LabelText(const char * fmt, const std::string & str);
	
}