#pragma once

#include "rui/Menu.hpp"
#include "rd/Style.hpp"
#include "ri18n/T.hpp"

namespace rui {
	//very simple dropdown, just override getButton and be done with it (at least its the spirit )
	class DropDown : public rui::Menu {
		typedef rui::Menu				Super;
	public:
		eastl::vector<Str>				labels;
		eastl::vector<rui::Button*>		bts;

		//for translation purpose you can override getUIName
		std::function<Str(const char* id)> getUIName;

		Str								selected;
		rd::MsgChanCStr					onAccepted;
		rd::Sig							onRejected;
		rd::Promise*					result = 0;
		rd::Style						style;

	//protected:
		r2::Flow*						frame = 0;
		r2::Graphics*					bg = 0;
		r2::Node*						root = 0;
		bool							enableHelpPanel = false;
	//public:
										DropDown(r2::Node* parent);
		virtual							~DropDown();

		virtual void					dispose();
		virtual rui::Button*			getButton(const char* entryLabel, const char* buttonLabel);

		//takes helpReplacer and returns help frame content
		ri18n::TextReplacer				helpReplacer;

		virtual void					makeDefaultButtonInteractor();
		virtual void					makeTestButtonInteractor();
		virtual void					simBack() override;
		void							setContent(const std::initializer_list<const char*>& _labels);
		void							setContent( const eastl::vector<Str>&labels );
		
		virtual void					update( double dt) override;
		virtual void					build();
		virtual void					accept(const char * lbl);
		virtual void					reject();
		virtual r2::Node*				feedHelp(const char* lbl);

		virtual void					im() override;
	};

}

