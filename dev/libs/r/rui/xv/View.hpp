#pragma once

#include "rd/Agent.hpp"

namespace r2 {	class Node; }
namespace rd {	class Style; }

namespace rui {
	namespace xv {
		class StyleSheet;
		class View : public r2::Node {
			typedef r2::Node Super;
		public:
			/**
			* roadmap
			* 
			* attach basic components
			* live style editing and propagation
			* load form files
			* hot reload from files
			*/ 
			rui::xv::StyleSheet *	styleSheet = 0;
			rd::AgentList			al;

									View(r2::Node * par = 0 );
									
			void					attachStyleSheet(const char* stylesheet);

			void					appendStyleSheet(const char* stylesheet);
			
			void					attachView(const char* view);

			static View*			fromString(const Str& _style, const Str& _view,r2::Node* par = 0);
			static View*			fromString(const char* _style, const char* _view, r2::Node* par = 0);
			static View*			fromFiles(const char* _stylePath, const char* _viewPath, r2::Node* par = 0);
			virtual void			im() override;
			
		protected:
									View(const char* _style, const char* _view, r2::Node* par);

			Str						lastStyleRead;
			Str						lastViewRead;
			ri18n::AstNode*			tree=0;
			r2::Node*				tip = 0;

			void					generateNode(ri18n::AstNode* node);
			void					applyStylesheet(rui::xv::StyleSheet* sheet, r2::Node* node);
		};
	}
}