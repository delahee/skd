#pragma once

#include "r2/BatchElem.hpp"

namespace rd {
	class SubBatchElem : public r2::BatchElem {
	public:
		r2::BatchElem *			parent = nullptr;

		float					offX = 0.0;
		float					offY = 0.0;
	public:
		SubBatchElem();
		virtual ~SubBatchElem();
		virtual void dispose();
		virtual void reset();

		/**
		beware, must be updated by hand
		*/
		virtual void			update(double dt) override;

		void					setOff(double x, double y);


		static SubBatchElem *	fromPool(r2::Tile * tile, r2::BatchElem * parent, int priority = 0);
		virtual void			toPool();
		static void				updatePool(double dt);
	};
}