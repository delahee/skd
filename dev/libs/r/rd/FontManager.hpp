#pragma once

#include <algorithm>
#include <unordered_map>

#include "r2/Types.hpp"
#include "2-fonts/FontResource.h"
#include "Str.h"

namespace rd{
	class Font : public Pasta::FontResource {
		r2::Shader m_shader;
	public:
		Font(const char* path) : Pasta::FontResource(path), m_shader(r2::Shader::SH_Basic) {};
		Font(const char* path, r2::Shader shader) : Pasta::FontResource(path), m_shader(shader) {};

		r2::Shader getShader() const { return m_shader; }
	};

	class FontManager {
	public:
		std::unordered_map<Str, rd::Font* > map;
								FontManager() {}

		rd::Font*				getFont(const std::string& name);
		rd::Font*				add(const std::string& name, const std::string& path, r2::TexFilter filter = r2::TexFilter::TF_NEAREST, r2::Shader shader = r2::Shader::SH_Basic);
		void					destroy(const std::string& name);

		void					alias(const char* dst, const char* src);
		void					setPremultipliedAlpha(rd::Font* fnt, bool onOff);
		void					im();
		static FontManager&		get();
		const char*				getFontName(rd::Font* fnt) const;

		rd::Font*				dflt = 0;
		static FontManager*		me;
	};
}