#pragma once

#include <vector>
#include <algorithm>
#include "../rs/Pool.hpp"
#include "../rs/ITweenable.hpp"

namespace rd { class AnimBus; }

namespace rd{
	enum class TType : u32 {
		TLinear=0,
		TLoop,
		TLoopEaseIn,
		TLoopEaseOut,
		TCos,
		TSin,
		TEase,
		TEaseIn,
		TEaseOut,
		TBurn,
		TBurnIn,
		TBurnOut,
		TZigZag,
		TJump,
		TBounceOut,
		TShake,
		TElasticEnd,

		TQuad,
		TCubic,

		TCount,
	};

	class Tweener;

	class Tween {
	public:
		static int			GUID				;
		int					uid					= 0;
		Tweener *			man					= nullptr;
		rs::ITweenable *	parent				= nullptr;
		rs::TVar			vname				= rs::TVar::VNone;
		double				n					= 0.0;
		double				duration			= 0.000001;//in seconds
		double				time				= 0.0;
		double				speed				= 0.0;
		double				from				= 0.0;
		double				to					= 0.0;
		TType				type				= TType::TLinear;
		int					nbPlays				= 1; // -1 = infini, 1 et plus = nombre d'exécutions (1 par défaut)
		void *				userData			= nullptr;
		double				delayMs				= -1.0;
		bool				autoReverse			= false;

		std::function<void(rs::ITweenable*)>			afterDelay		= nullptr;
		std::function<void(rs::ITweenable*)>			onUpdate		= nullptr;
		std::function<void(rs::ITweenable*,double t)>	onUpdateT		= nullptr;
		std::function<void(rs::ITweenable*)>			onEnd			= nullptr;
		std::function<double(double)>					interpolate		= nullptr;
		
		Tween();

		void reset(
			rs::ITweenable * parent,
			rs::TVar vname,
			double duration,
			double from,
			double to,
			TType type,
			int nbPlays,
			std::function<double(double)> interpolate
		);
		void clear();
		void im();
	};

	class Tweener : public rd::Agent {
	protected:
		float					normalizeAngle(float a);
	public:
		bool					isLoadingFrame = false;
		int						fr = 0;

		double					DEFAULT_DURATION = 1000.0;
		double					FPS				= 60.0;

		rs::Pool<Tween>			pool;
		std::vector<Tween*>		tList;
		std::vector<Tween*>		delayList;
		rd::AnimBus*			bus = nullptr;
		void					onUpdate(Tween* t, double n);
		void					onEnd(Tween* t);

								Tweener();
		virtual					~Tweener();

	public:
		rd::Tween*				createColorHSV(rs::ITweenable* parent, r::Color to, rd::TType tp = TType::TLinear, double duration_ms = -1.0, bool allowDup = false);
		rd::Tween*				createColorLinear(rs::ITweenable* parent, r::Color to, rd::TType tp = TType::TLinear, double duration_ms = -1.0, bool allowDup = false);
		rd::Tween*				create(rs::ITweenable* parent, rs::TVar varName, double to, rd::TType tp = TType::TLinear, double duration_ms = -1.0, bool allowDup = false);
		rd::Tween*				createFromTo(rs::ITweenable* parent, rs::TVar varName, double from, double to, rd::TType tp = TType::TLinear, double duration_ms = -1.0, bool allowDup = false);

		rd::Tween*				createPtr(rs::ITweenable* parent, double * ptr, double to, rd::TType tp = TType::TLinear, double duration_ms = -1.0, bool allowDup = false);
		rd::Tween*				createPtr(rs::ITweenable* parent, float * ptr, float to, rd::TType tp = TType::TLinear, double duration_ms = -1.0, bool allowDup = false);

		//delay allows duplicates by default as its probably something user wanted
		rd::Tween*				delay(double delay_ms, rs::ITweenable* parent, rs::TVar varName, double to, TType tp = TType::TLinear, double duration_ms = -1.0, bool allowDup = true);
		//delay allows duplicates by default as its probably something user wanted
		rd::Tween*				delayFromTo(double delay_ms, rs::ITweenable* parent, rs::TVar varName, double from, double to, TType tp = TType::TLinear, double duration_ms = -1.0, bool allowDup = true);

		static inline			std::function<double(double)> getInterpolateFunction(rd::TType);
		static std::function<double(double)>
								interpolators[(int)rd::TType::TCount];

		bool					exists(rs::ITweenable* p, rs::TVar v);
		rd::Tween*				get(rs::ITweenable* p, rs::TVar v);

		//does not call any callback
		void					forceTerminateTween(Tween * t);

		void					terminateAllTween(rs::ITweenable * p);
		void					killAllTween(rs::ITweenable * p);
		
		//calls appropriate callback
		void					terminateTween(Tween * t, bool fl_allowLoop);
		void					terminateTween(rs::ITweenable* t, rs::TVar tv);

		virtual void			update(double dt) override;
		virtual void			dispose() override;;
		void					clear();
		bool					im();

		static inline double	fastPow2(double n) { return n*n; };
		static inline double	fastPow3(double n) { return n*n*n; };
		static inline double	bezier(double t, double p0, double p1, double p2, double p3) {
			double minust = 1.0 - t;
			return
				fastPow3(minust)*p0 +
				3 * (t*fastPow2(minust)*p1 + fastPow2(t)*(minust)*p2) +
				fastPow3(t)*p3;
		};
	};


	//to be used for arbitrary variable tweening using createPtr, you should probably avoid to use it in general unless you know exactly what you do
	class TweenableDummy : public rs::ITweenable {
	public:
		double			dummyValue = 0.0f;
		virtual			~TweenableDummy() {};

		virtual double	getValue(rs::TVar valType) { return dummyValue; };
		virtual double	setValue(rs::TVar valType, double val) { return dummyValue=val; };
	};
}
