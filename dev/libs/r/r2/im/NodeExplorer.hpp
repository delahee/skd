#pragma once

#include "r2/Node.hpp"
#include "r2/Text.hpp"
#include "r2/Graphics.hpp"
#include "rd/Agent.hpp"

namespace r2 {
	namespace im {
		class NodeExplorer : public rd::Agent {
		public:
			std::string							name;
			r2::Node *							n = nullptr;
			rd::SignalHandler*					sigDelete = nullptr;
			bool								opened = true;
			bool								keepTargetHighlighted = false;
			static std::vector<NodeExplorer*>	ALL;

											NodeExplorer(r2::Node * _n, bool killPrevious = false);
											~NodeExplorer();
			virtual void					update(double dt) override;

			//will do nothing if such explorer is already opened;
			static NodeExplorer*			edit(r2::Node * _n);
			static void						cancel(r2::Node * _n);

			//will do nothing if such explorer is not opened;
			static void						onDeletion(r2::Node*n);
			static void						killAll();
		};
	}
}