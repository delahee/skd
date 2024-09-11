#pragma once

#include "rs/GfxContext.hpp"

#include "Tile.hpp"
#include "Sprite.hpp"
#include "Node.hpp"
#include "Batch.hpp"
#include "BatchElem.hpp"

#include "rd/FontManager.hpp"
#include "rd/Sig.hpp"
#include "rd/RdInterfaces.hpp"

namespace rd {
	class Style;
	class Stylable;
}

namespace r2{

	struct ElementInfo{
		int charcode = 0;
		int charPos = 0;
		int lineIdx = 0;
	};

	struct LineInfo {
		double width = 0;
		double bottom = 0;
	};

	struct CharPos {
		float x;
		float y;
	};
	
	struct TextColorRange {
		int start=0;
		int end=0;
		r::Color col;
		bool multiply = true;

		void serialize(Pasta::JReflect* jr, const char* name = 0);
	};

	struct DropShadow {
		float		dx		= 0.f;
		float		dy		= 0.f;
		r::Color	col		= r::Color(1,1,1,1);

		DropShadow() {};
		DropShadow(const DropShadow & ds) {
			dx = ds.dx;
			dy = ds.dy;
			col = ds.col;
		};

		void serialize(Pasta::JReflect* jr, const char* name = 0);
	};

	struct Outline {
		float		delta = 1.0f;
		r::Color	col		= r::Color(1, 1, 1, 1);

		Outline() {};

		Outline(const Outline & o) {
			col = o.col;
		};

		void serialize(Pasta::JReflect* jr, const char* name = 0);
	};

	struct FontSetup{
		rd::Font*	font = nullptr;
		int			size = 24;
	};

	class Text : public Batch, public rd::IStylable {
	private:
		typedef r2::Batch Super;
	public:
		static const int						ALIGN_LEFT = 0;
		static const int						ALIGN_TOP = 0;
		static const int						ALIGN_RIGHT = 1;
		static const int						ALIGN_BOTTOM = 2;
		static const int						ALIGN_HCENTER = 4;
		static const int						ALIGN_VCENTER = 8;
		static const int						ALIGN_CENTER = ALIGN_HCENTER | ALIGN_VCENTER;

		bool									snapToPixel = true;

		rd::Sig									onTextChanged;
		std::function<void(r2::BatchElem*)>		onPushChar;

								Text(FontSetup	fnt, const char *, r2::Node * parent = nullptr);
								Text(FontSetup	fnt, const std::string& t, r2::Node* parent = nullptr) : Text(fnt,t.c_str(),parent){ }
								Text(FontSetup	fnt, const Str& t, r2::Node* parent = nullptr) : Text(fnt, t.c_str(), parent) { }

								Text(rd::Font* fnt = nullptr, const char* t = nullptr, r2::Node* parent = nullptr);
								Text(rd::Font * fnt, const Str & t, r2::Node * parent = nullptr) : Text(fnt,t.c_str(),parent){}
								Text(rd::Font * fnt, const std::string & t, r2::Node * parent = nullptr) : Text(fnt, t.c_str(), parent) {}

		virtual					~Text();
		virtual void			reset() override;
		virtual void			dispose() override;

		const char*				getFontName() const;
		void					setFont(rd::Font * fnt);

		virtual const char*		setTranslationKey(const char * str);
		
		virtual const char*		setText(const char * str);
		const char*				setText(const std::string & str) { setText(str.c_str()); return text.c_str(); };
		const char*				setText(const Str & str) { setText(str.c_str()); return text.c_str(); };

		const char*				getText() { return text.c_str(); };

		static r::Vector2		getTextExtent(rd::Font* fnt, int fontSize, const char * txt);
		static r::Vector2		getTextSimulatedSize(rd::Font* fnt, int fontSize, const char * txt);

		r::Vector2				getTextSize(const char * text);
		const char *			getLineWidth(const char * text, float * w,int count = -1);
		void					getTextChar(const char * text, int pos, int * nextPos, int * charId, int * nextCharId);
		float					getLineHeight();

		float					getLineSpacingOffset() { return lineSpacingOffset; };
		void					setLineSpacingOffset(float f);


		int						getMaxLineWidth() { return maxLineWidth; };
		void					setMaxLineWidth(int v);
		void					clearMaxLineWidth();

		Pasta::Vector2			getSize() { return getTextSize(text.c_str()); }

		void					autosize(double maxWidth, int targetFontSize, int minFontSize = -1);
		void					clearAutosize();

		//mere stub no need to virtualize
		void					setTextColor(const Pasta::Color & c);
		virtual void			setTextColor(const r::Color & c);

		void					addTextColor(r::Color c, int start = 0, int end = -1);//can react badly to newlines count wise, todo test
		void					addTextColor(Pasta::Color c, int start = 0, int end = -1);//can react badly to newlines count wise, todo test

		void					replaceTextColor(Pasta::Color c, int start = 0, int end = -1);//can react badly to newlines count wise, todo test
		void					replaceTextColor(r::Color c, int start = 0, int end = -1);//can react badly to newlines count wise, todo test

		void					addDropShadow(float dx=0, float dy=1, r::Color col=r::Color::Black);
		void					removeDropShadow();
		
		void					addOutline(const r::Color &col,float delta=1.0f);//col manage the alpha of the outline
		void					removeOutline();

		virtual void			applyStyle(const rd::Style& st) override;

		void					update(double dt) override;
		/**
		* 
		* 
		* ALIGN_LEFT = 0;
		* ALIGN_TOP = 0;
		* ALIGN_RIGHT = 1;
		* ALIGN_BOTTOM = 2;
		* ALIGN_HCENTER = 4;
		* ALIGN_VCENTER = 8;
		* ALIGN_CENTER = ALIGN_HCENTER | ALIGN_VCENTER;*/
		void					setBlockAlign(int align);

		static r2::Text*		mk(rd::Font* fnt, const r::TKey & txt,		r::Color color, Node* parent = nullptr);
		static r2::Text*		fromPool(rd::Font* fnt, const char* txt,	r::Color color, Node* parent = nullptr);
		static inline r2::Text*	fromPool(rd::Font* fnt, const Str& txt,		r::Color color, Node* parent = nullptr) {
			return fromPool(fnt, txt.c_str(), color, parent );
		};
		static inline r2::Text*	fromPool(rd::Font* fnt, const std::string& txt, r::Color color, Node* parent = nullptr) {
			return fromPool(fnt, txt.c_str(), color, parent );
		};

		void					setBgColor(r::Color c);

		virtual void			im() override;
		//only display the text specific im;
		virtual void			textIm();

		virtual r2::Node*		clone(r2::Node*) override;

		virtual	NodeType		getType()const override { return NodeType::NT_TEXT; };
		virtual void			serialize(Pasta::JReflect& f, const char* name = nullptr) override;
		rd::Font*				getFont() { return fnt; };

		float					getFontSize();
		void					setFontSize(std::optional<float> newSize);

		void					setBgExtent(const r::Vector4 & ex);
		void					setItalicBend(r::opt<int> point, bool doCache = true);

		//centers paragraph rendering vertically and horizontally
		void					centered();
		void					tryTranslate();

	protected:
		virtual void			addBg();

		const char*				pushLineStraight(const char* str, float* _x, float* _y, int count = -1);
		const char*				pushLineWithCesure(const char* str, float* _x, float* _y);
		const char *			pushLine( const char * str, float * x, float * y);
		void					pushChar( int charId, int nextCharId, int curPos, float * x, float * y);

		void					clearContent();
		bool					canBeCesured(int charId);
		virtual void			cache();
		void					advanceCursorPos(float * x, float * y, const Pasta::CharDescr * ch, int nextCharId);

		r::Color				bgColor;
		Vector4					bgExtent;

		r::opt<r::Color>		forceColor;
		
		int						maxLineWidth = -1;
		int						originalMaxLineWidth = -1;
		float					lineSpacingOffset = 0;
		bool					retainElementInfos = true;

		std::unordered_map<r::uid,ElementInfo> 
								elInfos;
		//todo get it in vector
		std::vector<LineInfo> 
								lineInfos;

		r::opt<int>				autoSizeWidth;
		r::opt<int>				autoSizeTarget;
		r::opt<int>				autoSizeActual;
		r::opt<int>				autoSizeMin;

		void					setLineInfos(int idx, LineInfo li);
		void					applyAlign();

	protected:
		int						blockAlign = 0;
		rd::Font*				fnt=nullptr;
		Str						text="";
		Str						translationKey="";
		int						curPrio = 0;

		r::opt<DropShadow>		dropShadow;
		r::opt<Outline>			outline;
		std::vector<TextColorRange>	colors;
		int						curLogicalCharacter = 0;
		int						curLogicalLine = 0;


		r::opt<int>				italicBend;

		int						getNextCesurePoint(const char* text);
		int						getAnteCesurePoint(const char* text, float beforeW);

		void					onBeforeDrawFlush();
	};

}//end namespace