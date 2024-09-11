#pragma once

#include "rd/Agent.hpp"

namespace r2 {
	class BatchElem;

	namespace im {
		class BatchElemExplorer : public rd::Agent {
		public:
			r::uid									nUid = 0;//used to guess if elem was deleted
			r2::BatchElem *							n = nullptr;
			bool									opened = true;
			bool									doFocus = false;
			bool									keepTargetHighlighted = false;
			static std::vector<BatchElemExplorer*>	ALL;

			BatchElemExplorer(r2::BatchElem * _n, bool killPrevious);
			~BatchElemExplorer();
			virtual void						update(double dt) override;

			//will do nothing if such explorer is already opened;
			static BatchElemExplorer*			edit(r2::BatchElem * _n);

			static void							onDeletion(r2::BatchElem*n);
			static void							killAll();
		};
	}
}