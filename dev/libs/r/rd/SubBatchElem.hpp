#pragma once

#include "r2/BatchElem.hpp"

namespace rd {
	class SubBatchElem : public r2::BatchElem {
		typedef r2::BatchElem Super;
	public:
		r2::BatchElem *			parent = nullptr;
		r::vec3					ofs;

		static constexpr rs::TVar VOfsX = rs::TVar(rs::TVar::VCustom0 + 32);
		static constexpr rs::TVar VOfsY = rs::TVar((((int)rs::TVar::VCustom0 + 32) + 1));
		static constexpr rs::TVar VOfsZ = rs::TVar((((int)rs::TVar::VCustom0 + 32) + 2));

		virtual BeType			getType() const;

	public:
								SubBatchElem();
		virtual					~SubBatchElem();
		virtual void			dispose();
		virtual void			reset();

		void					syncCoords();

		/**
		beware, must be updated by hand
		*/
		virtual void			update(double dt) override;

		static SubBatchElem *	fromPool(r2::Tile * tile, r2::BatchElem * parent, int priority = 0);
		virtual void			toPool();

		static bool				poolServiceInstalled;
		static void				updatePool(double dt);

		virtual double			 getValue(rs::TVar valType) override;
		virtual double			 setValue(rs::TVar valType, double val) override;
	};

	class SubABatchElem : public rd::ABatchElem {
		typedef rd::ABatchElem Super;
	public:
		r2::BatchElem*			parent = nullptr;
		r::vec3					ofs;

		static constexpr rs::TVar VOfsX = rs::TVar(rs::TVar::VCustom0 + 32);
		static constexpr rs::TVar VOfsY = rs::TVar((((int)rs::TVar::VCustom0 + 32) + 1));
		static constexpr rs::TVar VOfsZ = rs::TVar((((int)rs::TVar::VCustom0 + 32) + 2));

		virtual BeType				getType() const;

	public:
									SubABatchElem();
		virtual						~SubABatchElem();
		virtual void				dispose();
		virtual void				reset();

		void						syncCoords();

		/**
		beware, must be updated by hand
		*/
		virtual void				update(double dt) override;

		static SubABatchElem*		fromPool(rd::TileLib* l, const char* grp, r2::BatchElem* parent, int priority = 0);
		virtual void				toPool();

		static bool					poolServiceInstalled;
		static void					updatePool(double dt);

		virtual double				getValue(rs::TVar valType) override;
		virtual double				setValue(rs::TVar valType, double val) override;
	};
}