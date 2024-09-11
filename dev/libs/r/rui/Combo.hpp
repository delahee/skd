#pragma once

#include "rd/Style.hpp"

namespace rui {
	class DropDown;

	class Combo : public r2::Node {
		typedef r2::Node Super;
		public:
			struct Conf {
				//id x translation
				bool									translate = true;
				rd::Style*								style{};
				Str										activeId;
				eastl::vector<Str>						content;
				rui::Menu*								menu{};
				std::function<Str(const char* id)>		getUIName;
			};

			Conf			conf;
			rd::Style		style;

			rui::Button*	bt{};
			rui::DropDown*	dd{};

			rd::MsgChanStr	onAccept;
			rd::Sig			onReject;

			rd::Sig			onDropDownCreation;
			rd::Sig			afterDropDownCreation;

							Combo(Conf cnf, r2::Node* n);
							~Combo();

			bool			isActive();
			void			activate();
			virtual void	im() override;
	};
}