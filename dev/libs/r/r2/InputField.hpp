#pragma once

#include "Text.hpp"
#include "Interact.hpp"
#include "rd/FontManager.hpp"

#include <string>

namespace r2 {
	class Scissor;

	struct InputConf {
		rd::Font *		font = nullptr;
		std::string		text;

		float			width = 100.0;

		float			leftPadding = 1.0;
		float			topPadding = 0.0;

		r::Color		textColor;
		r::Color		bgColor;
		bool			leftAlign = true;

		InputConf(rd::Font * _font) : font(_font) {
			textColor = r::Color(0., 0., 0., 1.);
			bgColor = r::Color(1., 1., 1., 1.);
		};
	};

	class InputField : public Interact {
	public:
		Text *			tf			= nullptr;
		Graphics*		bg			= nullptr;
		Graphics*		caret		= nullptr;
		int				caretPos		= 0;
		bool			deferValidation = false;

		int				maxChar			= 600;

		std::string		value;

		InputConf		conf;

		rd::Sig			sigFocus;
		rd::Sig			sigFocusLost;

		rd::Sig			sigEnter;

		r2::Scissor *	mask = nullptr;

		bool			isDefaultValue = false;
		r::Color		defaultValueColor;
		std::string		defaultValue = "";

	protected:
		double			caretTimer = 0.0;

	public:
						InputField(InputConf&conf, Node * parent);
						
		virtual			~InputField();
		
		/***
		Reflect val into text field
		*/
		void			syncVal();

		/***
		Reflect val into styling
		*/
		void			syncX();

		void			update(double dt) override;
		
		void			setAsDefault(r::Color col);
		void			insertAtCaret(const std::string& str);
		void			setDefaultValue(const std::string& value);
		void			setValue(const std::string & t);
		void			setCaretPos(int v);

		//beware the new value can me mutated
		std::function<void(std::string & newValue)>	onChangeCbk = nullptr;

		virtual void	onChange(std::string & newValue);

	protected:
		void			_onFocus(rs::InputEvent &);
		void			_onFocusLost(rs::InputEvent &);

		void			createDelegate();

		void			handleKey(unsigned char key);

		void			_onChar(rs::InputEvent & ev);
		void			_onKeyUp(rs::InputEvent & ev);
		void			_onKeyDown(rs::InputEvent & ev);
		
		
	public:

	};
}
