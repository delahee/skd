#pragma once

#include "1-graphics/ShaderProgram.h"

#include "r/Types.hpp"
#include "r/Color.hpp"
#include "r2/Types.hpp"

#include <unordered_map>

namespace r2{
	class Node;
	class Scene;
	class Sprite;
	class Batch;

	static inline constexpr u32		UBER_START = 1<<6;

	//see BasicShaderFlags
	enum UberShaderFlags : u32 { // not an enum class to keep implicit conversion
		USF_Gaussian_Blur			= UBER_START << 0,
		USF_Bloom					= UBER_START << 1,
		USF_RGBOffset				= UBER_START << 2,
		USF_Glitch					= UBER_START << 3,
		USF_DisplacementMapping		= UBER_START << 4,
		USF_ColorMatrix				= UBER_START << 5,
		USF_FXAA					= UBER_START << 6,

		USF_Distortion			    = UBER_START << 7,
		USF_ChromaticAberration		= UBER_START << 8,
		USF_BloomPyramid			= UBER_START << 9,
		USF_Vignette				= UBER_START << 10,

		USF_Dissolve				= UBER_START << 11,
		USF_ShowNoise				= UBER_START << 12,
		USF_DissolveUseTexture		= UBER_START << 13,
		USF_Dither					= UBER_START << 14,

		USF_ColorAdd				= UBER_START << 15,
	};

	/***
	* To optimize this
	* doPad = false
	* offsetScale to 1.5
	* filter to true
	*/
	struct BloomCtrl{
		Pasta::Texture *		src				= nullptr;
		Pasta::Texture *		dst				= nullptr;

		float					dx				= 8.0;
		float					dy				= 8.0;
		float					offsetScale		= 1.5f;
		r2::TexFilter			blurFilter		= r2::TexFilter::TF_LINEAR;
		r2::TexFilter			renderFilter	= r2::TexFilter::TF_LINEAR;

		float					intensity		= 1.0f;
		float					pass			= 0.750f;
		r::Color				mul				= r::Color(1.f, 1.f, 1.f, 1.f);
		Pasta::Vector3			luminanceVector = Pasta::Vector3(0.299f, 0.587f, 0.114f);
		bool					skipBloomPass	= false;
		bool					skipColorPass	= false;
		bool					doPad			= true;

		BloomCtrl(Pasta::Texture * src = nullptr, Pasta::Texture * dest = nullptr)  : src(src), dst(dest) { }
		
		BloomCtrl(const BloomCtrl&c) {
			src = c.src;
			dst = c.dst;
			dx = c.dx;
			dy = c.dy;
			offsetScale = c.offsetScale;
			blurFilter = c.blurFilter;
			renderFilter = c.renderFilter;
			pass = c.pass;
			mul = c.mul;
			intensity = c.intensity;
			luminanceVector = c.luminanceVector;
			skipBloomPass = c.skipBloomPass;
			skipColorPass = c.skipColorPass;
			doPad = c.doPad;
		}

		void neutral() {
			offsetScale = 1.0f;
			dx = 0.0;
			dy = 0.0;
			mul = Pasta::Color(1,1,1,1);
			intensity = 1.0f;
			pass = 0.750f;
		};

		void abused() {
			offsetScale = 1.33f;
			dx = 64;
			dy = 64;
			mul = Pasta::Color(1, 1, 1, 1);
			intensity = 16;
			pass = 0.33f;
		};

		void fast() {
			offsetScale = 1.5f;
			dx = 16;
			dy = 16;
			mul = Pasta::Color(1, 1, 1, 1);
			intensity = 1.5f;
			pass = 0.750f;
		};

		void im();
	};

	struct GlitchControl {
		bool			enabled = true;
		Pasta::Vector4	glitchAmount = Pasta::Vector4(0, 0, 0, 1);
		r2::TexFilter	filter = r2::TexFilter::TF_NEAREST;

		void neutral() {
			glitchAmount = Pasta::Vector4(0, 0, 0, 1);
		};

		void setVertical(bool onOff);
		bool im();

		void setAmount(float f) { glitchAmount.x = f; };
		void setBigGlitchAmount(float f) { glitchAmount.y = f; };
		void setResolutionDivider(float f) { glitchAmount.w = f; };

		float getAmount() { return glitchAmount.x; };
		float getBigGlitchAmount() { return glitchAmount.y; };
		float getResolutionDivider() { return glitchAmount.w; };

		void readFromShader(r2::Sprite* spr);
		void upload(r2::Sprite* spr);
	};

	struct TextureHolder {
		r::u32						flags = 0;
		std::string					path;
		Pasta::Texture*				tex = nullptr;
		Pasta::TextureData*			texData = nullptr;

		TextureHolder(const std::string& path, Pasta::Texture* t, Pasta::TextureData* td = nullptr) {
			tex = t;
			texData = td;
			this->path = path;
			flags = 0;
		};
	};

	enum class ColorMatrixMode : u32 {
		CMM_HSV, 
		CMM_Colorize, 
		CMM_Matrix,
		CMM_Count,
	};

	struct ColorMatrixControl {
		bool						fresh = true;
		ColorMatrixMode				mode = ColorMatrixMode::CMM_HSV;
		float						hue = 0.0f;
		float						sat = 1.0f;
		float						val = 1.0f;
		r::Color					tint = r::Color(1, 1, 1, 1);
		float						ratioNew = 0.8f;
		float						ratioOld = 0.2f;

		r::Matrix44					mat;
		r2::TexFilter				filter = r2::TexFilter::TF_NEAREST;

		static inline constexpr float DEFAULT_HUE = 0.0f;
		static inline constexpr float DEFAULT_VAL = 1.0f;
		static inline constexpr float DEFAULT_SAT = 1.0f;

		ColorMatrixControl() {
			mat = r::Matrix44::identity;
		};

		//packup parameters into mat before upload
		void pack();
		void serialize(Pasta::JReflect* refl, const char * name=0);
		void upload(r2::Sprite* spr);
		void readFromShader(r2::Sprite* spr);
	};

	class Lib {

	friend class HotReloadShader;

	public:
		//uncached
		//uncounted
		static Pasta::TextureData*			getTextureDataFromCache(const std::string& path);
		static Pasta::Texture*				getTexture(const std::string& path, r2::TexFilter  filter = r2::TexFilter::TF_NEAREST, bool cacheTexture = true, bool cacheTextureData = false);

		static Pasta::Texture*				createTexture(Pasta::TextureData* texData, r2::TexFilter filter = r2::TexFilter::TF_NEAREST, bool tracked = true, const char* debugName=0);

		static Pasta::Texture *				createDepthTexture(int width, int height, r2::TexFilter filter = r2::TexFilter::TF_NEAREST);
		static Pasta::Texture *				createRenderTexture(int width, int height, r2::TexFilter filter = r2::TexFilter::TF_NEAREST, Pasta::TextureFormat::Enum textureFormat = Pasta::TextureFormat::RGBA8);

		static Pasta::FrameBuffer *			createRenderTextureFB(int _width, int _height, r2::TexFilter filter, Pasta::Texture * in = nullptr, bool destroyTextures = false, bool depth = false, Pasta::TextureFormat::Enum colorFormat = Pasta::TextureFormat::RGBA8);

		static Pasta::FrameBuffer *			createRenderTextureFBDepth(int _width, int _height, r2::TexFilter filter, bool destroy);

		static void							destroyFramebuffer(Pasta::FrameBuffer * fb);
		static void							destroyTexture(Pasta::Texture * tex);
		static double						dt();

		static void							init();
		static void							dispose();

		static void							m_gaussian_kernel(float *dest, int size, float radius);

		//DEPRECATED USE SERVICES UNLESS YOU KNOW WHAT YOU ARE DOING
		static void							tex_Copy(Pasta::Texture * dest, Pasta::Texture * src);
		//DEPRECATED USE SERVICES UNLESS YOU KNOW WHAT YOU ARE DOING
		static Pasta::Texture*				tex_Clone(Pasta::Texture * src);
		//DEPRECATED USE SERVICES UNLESS YOU KNOW WHAT YOU ARE DOING
		static Pasta::Texture *				tex_RGBOffset(
			Pasta::Texture * src, 
			Pasta::Vector2 dr, 
			Pasta::Vector2 dg, 
			Pasta::Vector2 db, 
			r2::TexFilter filter,
			Pasta::Texture * dst=nullptr
		);
		//DEPRECATED USE SERVICES UNLESS YOU KNOW WHAT YOU ARE DOING
		static Pasta::Texture *				tex_Glitch(Pasta::Texture * src, float intensityX, float percentBigX, float intensityY, float percentBigY, float phase, r2::TexFilter  filter = r2::TexFilter::TF_LINEAR, Pasta::Texture * dst = nullptr);
		//DEPRECATED USE SERVICES UNLESS YOU KNOW WHAT YOU ARE DOING
		static Pasta::Texture *				tex_Blur(Pasta::Texture * src, float dx, float dy, float offsetScale = 1.0f, r2::TexFilter filter = r2::TexFilter::TF_LINEAR, Pasta::Texture * dst = nullptr);
		//DEPRECATED USE SERVICES UNLESS YOU KNOW WHAT YOU ARE DOING
		static Pasta::Texture *				tex_ColorMatrix(Pasta::Texture * src, Pasta::Matrix44 mat, r2::TexFilter filter = r2::TexFilter::TF_LINEAR, Pasta::Texture * dst = nullptr);
		//DEPRECATED USE SERVICES UNLESS YOU KNOW WHAT YOU ARE DOING
		static Pasta::Texture *				tex_Acc(Pasta::Texture * src, Pasta::Texture * acc, float dx, float dy, r2::TexFilter filter = r2::TexFilter::TF_LINEAR, Pasta::Texture * dst = nullptr);
		//DEPRECATED USE SERVICES UNLESS YOU KNOW WHAT YOU ARE DOING
		static Pasta::Texture *				tex_Bloom(const BloomCtrl & ctrl);

		static Pasta::ShaderProgram*		getShader(r2::Shader, Pasta::u32 shaderFlags);

		static std::unordered_map<Pasta::u32, Pasta::ShaderProgram*>			uberShaders;
		static std::unordered_map<Pasta::u32, Pasta::ShaderProgram*>			msdfShaders;

		static Pasta::ShaderProgram*		loadCustomShader(const char* shaderName, std::vector<const char*> defines = {});
		static Pasta::ShaderProgram*		loadCustomShader(const char* pathVS, const char* pathFS, const char* version = "140", std::vector<const char*> defines = {});
		static void							applyShaderValues(Pasta::Graphic* g, Pasta::ShaderProgram* shader, rd::Vars & shaderValues);

		static r::Color						intToColor24(int rgb);
		static r::Color						getColor(int rgb, float alpha=1.0f);

		static constexpr bool				DEBUG = true;

		
		static void							changeType(r2::Node * nu, r2::Node * old);

		static void							imTex( const char * filter );

		static bool							saveContent(const char * path, const std::string & content);
		static std::string					loadContent(const char * path);

		static r::Vector2					screenToGlobal(r2::Scene*, r::Vector2 pt);
	private:
		static void							loadShader(Pasta::ShaderProgramDescription*, std::string);
		static void							releaseShader(std::unordered_map<Pasta::u32, Pasta::ShaderProgram*>*);
		static void							cacheShader(Pasta::ShaderProgramDescription*, Pasta::u32, std::unordered_map<Pasta::u32, Pasta::ShaderProgram*>*);
		static void							setShader(Pasta::u32, Pasta::ShaderProgram*, std::unordered_map<Pasta::u32, Pasta::ShaderProgram*>*);

		static std::vector<TextureHolder>	texCache;//todo move this to rsclib
		static std::vector<r::Texture*>		renderTexCache;//todo move this to rsclib

	};
}

static std::string transparencytype_to_string(Pasta::TransparencyType tt);


