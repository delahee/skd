#pragma once

#include "../r2/Text.hpp"
#include "Nar.hpp"

namespace ri18n {
	struct RichTextConf {
		bool mapCharactersToAst = false;
	};

	struct CSSRule {
		rd::Vars	value;
		CSSRule*	parent = nullptr;
	};

	struct CSSSheet {
		std::unordered_map<Str, CSSRule> rules;
	};

	struct ActionDisplayInfo{
		//inputs
		int			fontSize = 24;
		int			playerId = 0;

		//outputs
		r2::Node*	raw=0;
	};
	/**
	 * rich texts are slow avoid them in intense gameplay...
	 * here is a formatting example
	 * *word*
	 * **word**
	 * <font color='#mycols'></font>
	 * 
	 * TODO : support proper serialisation 
	 */
	class RichText : public r2::Text {
		private:
			using Text::setText;//remove set text from public space for now

		public:
			AstNode*							tree = 0;//alter the tree at you own risk

			//fires all these events when display occurs. you may want to run these through a proper interpreter if need be
			std::function<void(void)>			onEmStart;
			std::function<void(void)>			onEmEnd;

			std::function<void(void)>			onStrongStart;
			std::function<void(void)>			onStrongEnd;

			std::function<void(const char*)>	onEvent;
			std::function<void(const char*)>	onCondEvent;
			std::function<void(const char*)>	onCondUniqueEvent;
			std::function<void(const char*)>	onUniqueEvent;
			std::function<void(const char*)>	onImportantEvent;

			std::function<void(const char*)>	onTagStart;
			std::function<void(const char*)>	onTagEnd;

			std::function<void(const char*)>	onTagFrom;

			std::function<void(void)>			onFirstChar;
			std::function<void(void)>			onLastChar;
			std::function<void(const char*)>	onScript;

			
			static RichText*			mk(const char * txt, r2::Node* parent = nullptr);
			static RichText*			mk(const char * txt, RichTextConf conf, r2::Node* parent );

			inline static RichText*		mk(const std::string& txt, r2::Node* parent = nullptr)	{ return mk(txt.c_str(), parent); };
			inline static RichText*		mk(const Str& txt, r2::Node* parent = nullptr)			{ return mk(txt.c_str(), parent); };

										RichText(AstNode* _tree, r2::Node* parent);

										RichText(rd::Font* fnt, const std::string& t, r2::Node* parent = nullptr);
										RichText(rd::Font* fnt, const Str& t, r2::Node* parent = nullptr);
										RichText(rd::Font* fnt = nullptr, const char* t = nullptr, r2::Node* parent = nullptr);
			virtual						~RichText();

			virtual	void				dispose() override;
			void						setTree(AstNode * tree );
			void						setTree(const char * tree);
			virtual void				cache() override;
			void						renderNode(AstNode* tree, float* x, float* y);

			void						execute(std::function<void(AstNode*)> f);

			void						stdTagStart(const char * t);
			void						stdTagEnd(const char * t);

			r::Color					getTextColor(int idx = -1);
			virtual void				setTextColor(const r::Color &c) override;

			std::optional<int>			curSize();

			eastl::vector<eastl::vector<AstNode*>> 
										charsToNodeStarts;

			eastl::vector<eastl::vector<AstNode*>>
										charsToNodeEnd;

			CSSRule*					stylesheet = nullptr;
			std::vector<Str>			classes;

			void						im() override;

			r::Color					emColor = r::Color::Red;
			r::Color					strongColor = r::Color::Cyan;
			r::opt<r::Color>			strongOutlineColor;
			rd::AgentList				al;

			void						updateStyle();

			virtual void				update(double dt) override;

			static std::function<bool(const char *,ActionDisplayInfo&)> 
										getActionDisplayInfo;
		protected:
			RichTextConf				conf;
			void						pushCurrentColor();
			void						popCurrentColor();

			void						pushCurrentOutline();
			void						popCurrentOutline();

			void						ensureCharMaps();
			void						addAction(ActionDisplayInfo& info);
			
			std::vector<rd::Font*>		fonts;
			std::vector<int>			textSize;

			float						curX = 0;
			float						curY = 0;
	};
}
