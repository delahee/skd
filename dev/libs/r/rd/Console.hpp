#pragma once

#include "1-input/InputMgr.h"

namespace sol {
	class state;
}

namespace r2 {
	class Batch;
	class BatchElem;
	class Graphics;
	class Text;
	class InputField;
}

namespace rd {
	struct CLine {
		std::string					content;
		std::vector<r2::BatchElem*> spr;
		std::vector<Pasta::Vector2>	sprPos;
		double						height = 0.0;
		bool						isCommand = false;

		void im();
	};

	enum class ScriptLanguage : u32 {
		LUA, 
	};

	class Console : public r2::Node, public Pasta::ControllerListener {
		typedef r2::Node			Super;
	public:
		r2::Batch*					sb				= nullptr;
		r2::Graphics *				bg				= nullptr;
		r2::Text*					producer		= nullptr;
		std::string					fullLog;

		bool						discreet =
		#ifdef PASTA_DEBUG 
		false;
		#else 
		true;
		#endif

		bool						isSuperUser =
		#ifdef PASTA_FINAL 
		false;
		#else 
		true;
		#endif 

		static eastl::vector<rd::Console*>	ALL;
		static rd::Console*					me;
	public:
									Console(rd::Font * fnt, r2::Node * parent);
		virtual						~Console();

		void						init();
		void						release();

		void						log(const char * str);
		void						log(const std::string & str);

		void						clear();

		void						runAsSuperUser(const char * str);
		void						runAsUser(const char * str);

		void						runCommand(const char * str);
		void						runCommand(const std::string & str);

		static void					help();
		void						calcLineCoords();
		ScriptLanguage				scriptingBackend = ScriptLanguage::LUA;
		sol::state*					hostLua=0;

		virtual	void				dispose();
		bool						hasFocus();
		virtual void				im();

		bool						show = true;
	protected:
		void						mkHost();
		void						mkLuaHost();

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
		void						pokeFade(bool forced);

		void						notifyKeyPressed(Pasta::ControllerType ctrl, Pasta::Key key);
		void						notifyKeyReleased(Pasta::ControllerType ctrl, Pasta::Key key);
		void						onConnexionChange(Pasta::ControllerType _controller, bool _connected);

		int archivePos				= -1;
		std::vector<std::string>	archive;
		void						manageArchive(Pasta::Key key);
	};
	
}