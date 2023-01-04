
#pragma once

#define RAND_PI 3.14159265358979323846


namespace rd{
/************************************************************************/
/* Mersen Twister rand for correct distribution*/
/************************************************************************/
class Rand {
protected:
	int		status[4];
	
	int		mat1 = 0;
	int		mat2 = 0;
	int		tmat = 0;

	int		tinyMask	= 0x7FffFFff;
	int		tinySh0		= 1;
	int		tinySh1		= 10;
	int		tinySh8		= 8;
	int		uid			= 0;

	int		seed = 0;

public:
				Rand();
				Rand(int seed);
	void		init(int seed);
	void		nextState();

	inline int random( int r){
		nextState();
		return posMod(temper() , r);
	};

	inline double rand() {
		return random(1024 << 20)*1.0 / (1024 << 20);
	};
	
	template<typename Ty>
	inline Ty elem( const std::vector<Ty> & arr) {
		if (!arr.size()) return {};
		return arr[dice(0, arr.size()-1)];
	};

	template<typename Ty>
	inline Ty elem(const eastl::vector<Ty>& arr) {
		if (!arr.size()) return {};
		return arr[dice(0, arr.size() - 1)];
	};

	// usage shuffle(arr.data(), 0, arr.size());
	// end is excluded
	template<typename Ty>
	inline void shuffle(Ty * arr,int first, int end) {
		for(int i = first; i < end; ++i){
			int idxSrc = dice(first, end-1);
			int idxTgt = dice(first, end-1);
			Ty tmp = arr[idxTgt];
			arr[idxTgt] = arr[idxSrc];
			arr[idxSrc] = tmp;
		}
	};

	template<typename Ty>
	inline void shuffle(eastl::vector<Ty>& arr) {
		shuffle(arr.data(), 0, arr.size());
	};

	template<typename Ty>
	inline void shuffle(std::vector<Ty>& arr) {
		shuffle(arr.data(), 0, arr.size());
	};

	template<unsigned int arrSize>
	inline int elem(int (&arr)[arrSize]) {
		if (!arrSize) return {};
		return arr[dice(0, arrSize - 1)];
	}; 
	
	template<typename T>
	inline T elem(const std::initializer_list<T> & t) {
		if (!t.size()) return {};
		return *(t.begin() + dice(0, t.size() - 1));
	};

	inline int elem(int*arr,int arrSize) {
		if (!arrSize) return {};
		return arr[dice(0, arrSize - 1)];
	};

	inline bool toss() {
		return dice(0, 1) == 1;
	};

	inline int dice(int min,int max ) {
		return random(max - min + 1) + min;
	};
	
	inline int either(int a, int b) {
		return toss() ? a : b;
	};
	
	inline float fuzz(float val, float maxPc = 0.20f) {
		return diceF(val * (1 - maxPc), val * (1 + maxPc));
	}; 
	
	inline float fuzz(double val, float maxPc = 0.20f) {
		return diceF(val * (1 - maxPc), val * (1 + maxPc));
	};
	
	inline int fuzz(int val, float maxPc = 0.20f) {
		return dice(std::rint(val * (1 - maxPc)),std::rint(val * (1 + maxPc)));
	};

	inline DIRECTION randDir4() {
		int r = dice(0, 3);
		switch (r)
		{
		case 0: return DIRECTION::LEFT;
		case 1: return DIRECTION::RIGHT;
		case 2: return DIRECTION::UP;
		default: return DIRECTION::DOWN;
		}
	};

	inline float diceF(float min, float max) {
		return rand() * (max - min) + min;
	};

	inline Vector3 vec(const Vector3 &v) {
		return Vector3(diceF(0, v.x), diceF(0, v.y), diceF(0, v.z));
	};

	inline float angle() {
		return diceF(0.0f, 2.0f * (float)RAND_PI);
	};

	inline float angleDeg() {
		return diceF(0.f,360.f);
	};

	inline bool pc(float pc) {
		return diceF(0.f, 100.f) <= pc;
	};

	inline int posMod(int v, int m) {
		if (m == 0) return 0;
		int rv = v % m;
		return rv >= 0 ? rv : rv + m;
	};
	
	inline int sign() {
		return (random(2) & 1) ? 1 : -1;
	};


	inline Vector3 randSphere(float _radius) {
		for (int i = 0; i < 8; ++i) {
			float x1 = rand() * 2 - 1;
			float x2 = rand() * 2 - 1;
			float x3 = rand() * 2 - 1;
			float radius = sqrtf(x1 * x1 + x2 * x2 + x3 * x3);
			float ir = 1.0f / radius;
			if (radius > 0 && radius < 1) {
				x1 *= ir;
				x2 *= ir;
				x3 *= ir;
				return Vector3(x1 * _radius, x2 * _radius, x3 * _radius);
			}
		}
		return Vector3(0, 0, 1 * _radius);
	};

	static Rand&	get();

protected:
	static Rand *	_self;
	int temper();

};//end rand
}//end namespace rd