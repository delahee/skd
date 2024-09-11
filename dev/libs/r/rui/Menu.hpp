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
	class IController;
	class Menu;
	class Navigation;

	enum FOCUS_DIR_FLAG : u32 {
		ALLOW_UP = 1 << 0,
		ALLOW_DOWN = 1 << 1,
		ALLOW_LEFT = 1 << 2,
		ALLOW_RIGHT = 1 << 3,

		ALLOW_OK = 1 << 4,
		ALLOW_ESC = 1 << 5,

		ALLOW_DPAD = 1 <<6,
		ALLOW_LEFT_STICK = 1 << 7,
		ALLOW_RIGHT_STICK = 1 << 8,

		ALL = 0xffffff,
	};

	struct NavPointer {
		Navigation*			nav = 0;
		rui::IInteractive*	owner = 0;

		rui::IInteractive*	left = 0;
		rui::IInteractive*	right = 0;
		rui::IInteractive*	up = 0;
		rui::IInteractive*	down = 0;

		rui::IInteractive* follow(rd::Dir dir);

		void im(rui::IInteractive* elem,const char * prefix);
		void im();

		void remove(rui::IInteractive* i);
	};

	class Navigation {
		friend rui::Menu;

	public:
		rui::Menu*		menu = 0;
		std::unordered_map<rui::IInteractive*, NavPointer> 
						data;
		bool			filterOutGreyNodes = true;

		void			unlink(rui::IInteractive* a, rui::IInteractive* b, rd::Dir dir);
		void			link(rui::IInteractive* a, rui::IInteractive* b, rd::Dir dir, bool biDir = true);
		void			linkIfCloser(rui::IInteractive* a, rui::IInteractive* b, rd::Dir dir, bool biDir = true);
		rd::Dir			extractDirFromTo(const Vector2& from, const Vector2& to);
		void			generate();
		void			im();
		void			remove(rui::IInteractive* a);
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
	 * 
	 * typically use 
	 * new Button( menu )
	 * or 
	 * auto i = new interactiveOverride( parent )
	 * menu->addButton( i )
	 * 
	 * then when all button are in use menu->startNavigation()
	 * todo : push the interactives with a paired "navigator" that have direct TLDR links because right now "orders" don't manage grids
	 */
	class Menu : public r2::Node {
		typedef r2::Node Super;
		friend	IController;
		friend	rui::IInteractive;
		friend	rui::Navigation;
	public:
		static eastl::vector<rui::Menu*>	
									ALL;
	protected:
		eastl::vector<rui::IInteractive*>
									buttons;
	public:

		bool						useNewNavigation = false;
		bool						menuInputEnabled = true;
		bool						disposed = false;
		bool						showNavigation = false;
		bool						destructionInProgress = false;
		Navigation					navigation;

		rd::Tweener					tw;

		//allows to bind behaviours based on input authorizations, thus having yield/life,focus etc thing working with this agent
		rd::AgentList				inputBhv;
		rd::AgentList 				al;

		bool						autoNav = true;

		//guarantees controls will be accepted in at best yieldControls frames
		int							yieldControls = 7;
		double						yieldControlsSec = 0.0f;
		int							life = 0;
		int							currentFocus = -1;//currentFocus is always in getFocusables spaces

		u32							flags = (u32)FOCUS_DIR_FLAG::ALL;

		rd::Sig						sigBack;
		rd::Sig						sigExit;
		rd::Sig						sigFocusGranted;
		rd::Sig						sigFocusFailed;//wip
		
		Pasta::ControllerListener*	listener = nullptr;

		//will rebuild navigation info,
		//warning us with caution
		bool						navDirty		= false;
		r2::Interact*				blocker			= 0;

		rui::IController*			controlOverride	= 0;
		rui::IController*			controlAddition	= 0;
	public:
									Menu(const char* name, r2::Node* parent = nullptr);
		virtual						~Menu();

		r2::Interact*				interactBlock();
		
		virtual void				onEnterHierarchy();
		virtual void				onExitHierarchy();

		static void					armaggeddon();

		virtual void				update(double dt) override;
		virtual void				dispose() override;

		rui::IInteractive*			follow(rui::IInteractive* elem, rd::Dir dir);
		void						newFocus(rd::Dir dir);
		void						defaultFocus();

		static rui::Menu*			getMenu(r2::Node* entry);

		/**
		one may want to ensure we are not called twice 
		*/
		void safeDispose() {
			if (!disposed)
				dispose();
		}

		bool				isFrontMenu();


		virtual void		show() override;
		virtual void		hide() override;

		/**
		operate out transitions and call exit handler
		should be overriden in real situations to add motion design
		*/
		virtual void		exit();

		bool				shouldNavigateGreys() const;
		virtual bool		acceptsInput();
		virtual void		doControls();

		virtual void		newFocusLeft();
		virtual void		newFocusUp();
		virtual void		newFocusRight();
		virtual void		newFocusDown();

		virtual void		focusLeft();
		virtual void		focusUp();
		virtual void		focusRight();
		virtual void		focusDown();

		virtual void		defaultYield();
		virtual void		yieldHierarchy();

		//eastl seems to bug here and volumes does not justify eastl
		std::vector<rui::IInteractive*>	leftRightOrder;
		std::vector<rui::IInteractive*>	upDownOrder;

		/**
		if not good enough, feel free to override leftRightOrder &  upDownOrder
		*/
		virtual bool		isFocusable(IInteractive* inter);
		virtual void		grantFocus(IInteractive* inter);
		virtual void		blur(IInteractive* inter);
		void				blurAll();

		IInteractive*		getFocused();

		/**
		Crush anything interactive 
		*/
		void				resetNavigation();

		/**
		* beware if you have flows you should reflow first, 
		* automating it may trigger endless reflow/matrix sync chain to this case is manual for now
		*/ 
		void				generateNavigation();
		int					getFocusableIdx(IInteractive* inter);
		/**
		Do appropriate computation and select first focused element
		*/
		virtual void		startNavigation(int idx = 0);

		eastl::vector<IInteractive*> getButtons();
		std::vector<IInteractive*> getFocusables();

#ifdef _DEBUG
		static void	log(const char* str);
#else
		inline static void	log(...) {
			
		}
#endif

		bool			addButton(IInteractive* i);
		void			removeButton(IInteractive* i);

		virtual void	im() override;
		virtual void	simClick();
		virtual void	simBack();
		virtual void	setAsFrontMenu();
		rui::Menu*		getParentMenu();

	protected:
		static rui::Menu*	findFirstMenu(r2::Node* cur);
		void				generateSimpleNavigation();

	};


}