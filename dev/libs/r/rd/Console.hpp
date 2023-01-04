#pragma once

#include "r2/Text.hpp"
#include "r2/Node.hpp"
#include "r2/Batch.hpp"
#include "r2/Graphics.hpp"
#include "r2/BatchElem.hpp"
#include "r2/InputField.hpp"

#include "1-input/InputMgr.h"

#include "LuaScriptHost.hpp"

namespace rd {
	struct CLine {
		std::string					content;
		std::vector<r2::BatchElem*> spr;
		std::vector<Pasta::Vector2>	sprPos;
		double						height = 0.0;
		bool						isCommand = false;
	};

	enum class ScriptLanguage : u32 {
		LUA, 
	};

	class Console : public r2::Node, public Pasta::ControllerListener {
	public:
		r2::Batch*					sb				= nullptr;
		r2::Graphics *				bg				= nullptr;
		r2::Text*					producer		= nullptr;
	public:
									Console(rd::Font * fnt, r2::Node * parent);
		virtual						~Console();

		void						init();
		void						release();

		void						log(const char * str);
		void						log(const std::string & str);

		void						clear();

		void						runCommand(const char * str);
		void						runCommand(const std::string & str);

		static void					help();
		void						calcLineCoords();
		ScriptLanguage				scriptingBackend = ScriptLanguage::LUA;
		sol::state					hostLua;

		virtual	void				dispose();
		bool						hasFocus();

		bool						show = true;
	protected:
		void						mkHost();

		r2::InputField *			input	= nullptr;
		rd::Font *					fnt		= nullptr;
		bool						listening = false;
		std::vector<CLine>			lines;
			
		int							LH = 16;
		int							cy = 0;
		int							LW = 400;

		double						caretBlink = 0.0;
		double						timeToFade = 0.0;
		double						alphaTip = 1.0;
		int							caretPos = 0;

		virtual void				update(double dt) override;
		void						pokeFade();

		void						notifyKeyPressed(Pasta::ControllerType ctrl, Pasta::Key key);
		void						notifyKeyReleased(Pasta::ControllerType ctrl, Pasta::Key key);
		void						onConnexionChange(Pasta::ControllerType _controller, bool _connected);

		int archivePos				= -1;
		std::vector<std::string>	archive;
		void						manageArchive(Pasta::Key key);
	};
	
}