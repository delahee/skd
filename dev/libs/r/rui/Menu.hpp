#pragma once

#include "r2/Node.hpp"
#include "rd/Tween.hpp"
#include "rd/Sig.hpp"

namespace rd { class AgentList; }
namespace r2 { class Interact; }

namespace Pasta {
	class ControllerListener;
}

namespace rui {
	class IInteractive;

	enum FOCUS_DIR_FLAG : u32 {
		ALLOW_UP = 1 << 0,
		ALLOW_DOWN = 1 << 1,
		ALLOW_LEFT = 1 << 2,
		ALLOW_RIGHT = 1 << 3,

		ALLOW_OK = 1 << 4,
		ALLOW_ESC = 1 << 5,

		ALL = 0xffff,
	};

	/**
	 * Menu is an all encompassing menu stack
	 * some best practices and conventions are in order
	 *
	 * basically the menu will try to grant safety over anything else everywhere
	 * but to have that you have to respect yourselves
	 *
	 * - don't move the menu, keep it at 0,0 but move its content, this will help content blocking to be effective
	 * - don't hesitate to override navigation
	 * - don't expect it to be perfect try to aim for something simple with it.
	 * - some auto navigation are given out of the box, don't hesitate to read the source
	 */
	class Menu : public r2::Node {

	public:
		
		static eastl::vector<rui::Menu*>	ALL;
		eastl::vector<rui::IInteractive*>	buttons;
		Tweener						tw;

		AgentList *					al = nullptr;

		bool						autoNav = true;
		//guarantees controls will be accepted in at best yieldControls frames
		int							yieldControls = 7;
		int							life = 0;
		int							currentFocus = -1;//currentFocus is always in getFocusables spaces
		
		bool						menuInputEnabled = true;
		bool						disposed = false;

		u32							flags = (u32)FOCUS_DIR_FLAG::ALL;

		//allows to bind behaviours based on input authorizations, thus having yield/life,focus etc thing working with this agent
		AgentList*					inputBhv;

		Sig							sigDispose;
		Sig							sigExit;
		
		Pasta::ControllerListener*	listener = nullptr;

		//will rebuild navigation info,
		//warning us with caution
		bool						navDirty = false;
		r2::Interact*				blocker = 0;
	public:
									Menu(r2::Node* parent = nullptr);
		virtual						~Menu();

		r2::Interact*				interactBlock();
		
		virtual void				onEnterHierarchy();
		virtual void				onExitHierarchy();

		static void					armaggeddon();

		virtual void				update(double dt);
		virtual void				dispose();
		
		/**
		one may want to ensure we are not called twice 
		*/
		void safeDispose() {
			if (!disposed)
				dispose();
		}

		bool			isFrontMenu();

		/**
		operate out transitions and call exit handler
		should be overriden in real situations
		*/
		virtual void	exit();

		virtual bool	acceptsInput();
		virtual void	doControls();

		virtual void	 focusLeft();
		virtual void	 focusUp();
		virtual void	 focusRight();
		virtual void	 focusDown();

		//eastl seems to bug here and volumes does not justify eastl
		std::vector<rui::IInteractive*>	leftRightOrder;
		std::vector<rui::IInteractive*>	upDownOrder;

		/**
		if not good enough, feel free to override leftRightOrder &  upDownOrder
		*/
		void			blur(IInteractive*inter);
		void			blurAll();
		bool			isFocusable(IInteractive* inter);
		void			grantFocus(IInteractive*inter);

		IInteractive*	getFocused();

		/**
		Crush anything interactive 
		*/
		void			resetNavigation();

		void			generateNavigation();
		/**
		Do appropriate computation and select first focused element
		*/
		virtual void	startNavigation(int idx = 0);

		std::vector<IInteractive*> getFocusables();

#ifdef _DEBUG
		inline static void	log(const char* str) {
			std::cout << str << std::endl;
		}
#else
		inline static void	log(...) {
			
		}
#endif

		void addButton(IInteractive* i) {
			buttons.push_back(i);
			if (autoNav) {
				navDirty = true;
			}
		}

		virtual void im() override;

		virtual void simClick();
		virtual void simBack();
	};
}