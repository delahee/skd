#pragma once

#include "r/Color.hpp"
#include "Vars.hpp"

namespace r2 {
	class Text;
	class Flow;
}
namespace rd {

	/**
	 * Stores styling stuff,
	 * has an on change signal
	 * in the cpp you can find common style literals
	 * 
	 */
	class Style {
	public:
		enum TextTransform :int {
			None,
			Capitalize,
			Lowercase,
			Uppercase,
		};
		Str							name;
		Style*						parent = nullptr;

		bool						_nonNativeSupportWarned = false;
		bool						dirty = false;

									Style(const char* str);
									Style(const Style& s);
									Style();

		virtual						~Style();

		Style&						operator=(const Style& s);
		void						operator+=(const Style& s);

		float						getFloat(const char* name) const;
		
		int							getInt(const char* name) const;
		Vector2						getVec2(const char* name) const;
		Vector3						getVec3(const char* name) const;
		r::Vector4i					getVec4i(const char* name) const;
		r::Color					getColor() const;
		r::Color					getColor(const char* name) const;
		const char*					getString(const char* name) const;
		rd::Font*					getFont(const char* name) const;
		rd::Font*					getFontFamily() const;
		int							getFontSize()const;


		void						setBool(const char* name, bool val);
		void						setInt(const char* name, int val);
		void						setFloat(const char* name, float val);
		void						setColor(const r::Color &col);
		void						setColor(const char* name, const r::Color &col);


		void						operator()(const char* label, int val);
		void						operator()(const char* label, float val);
		void						operator()(const char* label, const char* val);
		void						operator()(const char* label, const r::Color & col);
		void						operator()(const char* label, const r::Vector2i & vec);
		void						operator()(const char* label, const r::Vector4i & vec);

		void						im();
		
		rd::Sig						onChanged;

		bool						isEmpty() const;

		bool						has(const char * name) const;
		bool						has(const Str& name) const;

		void						load();
		void						setBackgroundColor(const r::Color& val) { this->operator()("background-color", val); };
		void						setVerticalSpacing(int val)				{ this->operator()("vertical-spacing", val); };
		void						setHorizontalSpacing(int val)			{ this->operator()("horizontal-spacing", val); };

		void						setFontSize(int val)					{ this->operator()("font-size", val); };
		void						setFontAutoSize(const Vector2i &val)	{ this->operator()("font-autosize", val); };
		void						setFontFamily(rd::Font* val);
		void						setTextTransform(TextTransform val);
		TextTransform				getTextTransform() const;

		/** margin( l t r b ) */
		void						setMargin(const Vector4i & val)			{ this->operator()("margin", val); };

		/** padding( l t r b ) */
		void						setPadding(const Vector4i & val)		{ this->operator()("padding", val); };

		/** offset( l t r b ) */
		void						setOffset(const Vector4i & val)			{ this->operator()("offset", val); };
		void						setPosition(const Vector3& val);
		void						setPosition(const Vector2& val);

		Vector3						getPosition() const;
		int							getMarginLeft() const;	int getMarginTop() const;		int getMarginRight() const;		int getMarginBottom() const;
		int							getPaddingLeft() const;	int getPaddingTop() const;		int getPaddingRight() const;	int getPaddingBottom() const;
		int							getOffsetLeft() const;	int getOffsetTop() const;		int getOffsetRight() const;		int getOffsetBottom() const;

		float						getDropShadowX() const;
		float						getDropShadowY() const;
		r::Color					getDropShadowColor() const;

		void						setDropShadow(float x, float y, const r::Color& col);

		float						getOutlineDist() const;
		r::Color					getOutlineColor() const;

		void						setOutline(float dist, const r::Color& col);
		virtual void				serialize(Pasta::JReflect& f, const char* name = nullptr);

		virtual void				apply(r2::Node & node) const;
		virtual void				apply(r2::Flow & fl) const;
		virtual void				apply(r2::Text & txt) const;

		
	protected:
		mutable rd::Vars			vals;
		void						ensure(const char * label);
		void						parseLit(const char* l);

		void						editFloat(const char* name);
		void						editFloat2(const char* name);
		void						editFloat3(const char* name);
		void						editInt(const char* name);
		void						editInt2(const char* name);
		void						editColor(const char* name);

	};

	
}

namespace std {
	static std::string			to_string(rd::Style::TextTransform tt);
}