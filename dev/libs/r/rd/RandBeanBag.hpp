#pragma once

namespace rd {
	class Rand;
}

template<typename Elem>
struct RandBeanBag {
	rd::Rand*			rand = 0;
	eastl::vector<Elem>	origin;
	eastl::vector<Elem>	bag;

	RandBeanBag() {

	};

	RandBeanBag(rd::Rand& r, const std::vector<Elem>& elems) {
		rand = &r;
		origin = rs::Std::eastlArray<Elem>(elems);
		bag = origin;
	};

	inline void set(rd::Rand& r, const std::vector<Elem>& elems) {
		rand = &r;
		origin = rs::Std::eastlArray(elems);
		bag = origin;
	};

	void set(rd::Rand& r, const eastl::vector<Elem>& elems) {
		rand = &r;
		origin = elems;
		bag = origin;
	}

	Elem next() {
		if (bag.empty())
			bag = origin;
		if (!bag.size())
			return {};
		auto idx = rand->dice(0, bag.size() - 1);
		if (idx < 0) return {};
		auto elem = bag[idx];
		bag.erase(bag.begin() + idx);
		return elem;
	};

	inline bool isEmpty() const {
		return origin.empty();
	};

	inline bool isDepleted() const {
		return bag.empty();
	};
};

