#pragma once

#include <string>
#include <vector>

#include "../Scene.hpp"
#include "../Node.hpp"

namespace r2 {
	namespace im {
		class HierarchyExplorer : rd::Agent {
		public:

			struct GameIPC {
				r2::Node*	cur = 0;
				r2::Node*	nu = 0;
				rd::Sig		onImAddNodeBegin;
				rd::Sig		onImAddNodeEnd;
			};
			static GameIPC				gameIPC;

			bool						showUtilities = false;
			std::string					searchFilter;

			static eastl::vector<r2::Scene*>	scs;

			bool						typeChanging = false;
			rd::Sig						onDestruction;
			rd::Sig						onImChanged;

			std::vector<r2::Node*>		deleted;
			r2::Node* typeChangeTarget = nullptr;
			bool						imOpened = true;
			static HierarchyExplorer *	me;

			static void					hide();
			static void					toggle(r2::Scene * sc);
			static r2::Node*			popupType();
			static r2::Node*			menuType();
			bool						popupAction(r2::Node* n, bool showEdit = false, int mouseButton = 0);

										HierarchyExplorer(r2::Scene * _sc, rd::AgentList * al);
			virtual						~HierarchyExplorer();
			
			static void					addScene(r2::Scene* _sc);
			static void					removeScene(r2::Scene* _sc);

			virtual void				update(double dt) override;

			void						im(r2::Node* n, int depth = 0);
			void						imToolbar(r2::Node* n, bool&skipAfter);


			void						save(r2::Node * n, const char* filename = nullptr);
			r2::Node*					load(r2::Node * n = nullptr, const char* filename = nullptr);

		};
	}
}