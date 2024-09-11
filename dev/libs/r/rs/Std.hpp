#pragma once

#include <vector>
#include <algorithm>
#include <map>
#include "EASTL/vector.h"
#include "EASTL/unordered_map.h"
#include "r/Types.hpp"

//am still a bit fearful to make a pure std extension
namespace rs {
	namespace Std {

		template<typename Dst, typename Src>
		inline void map(std::vector<Dst>& dst, const std::vector<Src>& src, std::function<Dst(Src elem)> f) {
			if (src.size() > dst.size())
				dst.reserve(src.size());
			for (auto it = src.begin(); it != src.end(); it++)
				dst.push_back(f(*it));
		};


		template<typename Dst, typename Src>
		inline std::vector<Dst> map(const std::vector<Src>& src, std::function<Dst(const Src& elem)> f) {
			std::vector<Dst> dst;
			if (src.size() > dst.size())
				dst.reserve(src.size());
			for (auto it = src.begin(); it != src.end(); it++)
				dst.push_back(f(*it));
			return dst;
		};

		template<typename Dst, typename Src>
		inline eastl::vector<Dst> map(const eastl::vector<Src>& src, eastl::function<Dst(const Src& elem)> f) {
			eastl::vector<Dst> dst;
			if (src.size() > dst.size())
				dst.reserve(src.size());
			for (auto it = src.begin(); it != src.end(); it++)
				dst.push_back(f(*it));
			return dst;
		};

		template<typename Elem>
		void filter(std::vector<Elem>& dst, std::vector<Elem>& src, std::function<bool(Elem elem)> f) {
			dst.clear();
			if (src.size() > dst.size())
				dst.reserve(src.size());
			for (auto it = src.begin(); it != src.end(); it++)
				if (f(*it))
					dst.push_back(*it);
		};

		/***
		* beware foe a strange reason stack based lambda are not well infered for eastl
		* so you have to to the function naming explictly
		* std::function<bool(rui::IInteractive*)> fl = [](rui::IInteractive* bt)->bool {
		* 	return !bt->isGreyed();
		* };
		* rs::Std::filter(bts, menu->buttons, fl);
		*/
		template<typename Elem>
		void filter(eastl::vector<Elem>& dst, eastl::vector<Elem>& src, std::function<bool(Elem elem)> f) {
			dst.clear();
			if (src.size() > dst.size())
				dst.reserve(src.size());
			for (auto it = src.begin(); it != src.end(); it++)
				if (f(*it))
					dst.push_back(*it);
		};

		template<typename T>
		inline bool exists(const eastl::vector<T>& vec, const T& elem) {
			return std::find(vec.cbegin(), vec.cend(), elem) != vec.cend();
		};

		template<typename T>
		inline void append(eastl::vector<T>& v0, const eastl::vector<T>& v1) {
			v0.insert(v0.end(), v1.begin(), v1.end());
		};

		template<typename T>
		inline void append(std::vector<T>& v0, const std::vector<T>& v1) {
			v0.insert(v0.end(), v1.begin(), v1.end());
		};

		template<typename T>
		inline bool exists(const std::vector<T>& vec, const T& elem) {
			return std::find(vec.cbegin(), vec.cend(), elem) != vec.cend();
		};

		template<typename T, typename V>
		inline bool exists(const std::unordered_map<T, V>& mp, const T& key) {
			return mp.find(key) != mp.cend();
		};
		
		template<typename V>
		inline std::vector<V> zero_iota(const V& nb) {
			std::vector<V> res;
			for (V z = {}; z < nb; z++)
				res.push_back(z);
			return res;
		};


		template<typename Elem>
		inline void shuffle(std::vector<Elem>& dst) {
			rd::Rand rand;
			for (auto it = dst.begin(); it != dst.end(); it++)
				std::swap( dst[rand.random(dst.size() - 1)], dst[rand.random(dst.size() - 1)] );
		};

		template<typename T, typename V>
		inline bool exists(const eastl::unordered_map<T, V>& mp, const T& key) {
			return mp.find(key) != mp.cend();
		};

		template<typename T, typename V>
		inline bool exists(const std::map<T, V>& mp, const T& key) {
			return mp.find(key) != mp.cend();
		};

		template<typename V>
		inline bool exists(const std::unordered_map<Str, V>& mp, const char* key) {
			return mp.find(StrRef(key)) != mp.cend();
		};

		template<typename V>
		inline bool exists(const std::unordered_map<std::string, V>& mp, const char* key) {
			return mp.find(key) != mp.cend();
		};

		static inline bool exists(const std::vector<std::string>& vec, const char* key) {
			for (auto& s : vec)
				if (0 == strcmp(s.c_str(), key))
					return true;
			return false;
		};

		static inline bool exists(const std::vector<Str>& vec, const char* key) {
			for (auto& s : vec)
				if (s == key)
					return true;
			return false;
		};

		static inline bool exists(const eastl::vector<Str>& vec, const char* key) {
			for (auto& s : vec)
				if (s == key)
					return true;
			return false;
		};

		template<typename T>
		inline int findPos(const std::vector<T>& vec, const T& key) {
			auto pos = std::find(vec.cbegin(), vec.cend(), key);
			if (pos == vec.end())return -1;
			return pos - vec.begin();
		};

		inline int findPos(const std::vector<std::string>& vec, const char* key) {
			auto pos = std::find(vec.cbegin(), vec.cend(), key);
			if (pos == vec.end())return -1;
			return pos - vec.begin();
		};

		inline int findPos(const std::vector<std::string>& vec, const std::string& key) {
			auto pos = std::find(vec.cbegin(), vec.cend(), key);
			if (pos == vec.end())return -1;
			return pos - vec.begin();
		};

		template<typename T>
		inline int find(const std::vector<T>& vec, const T& elem) {
			auto pos = std::find(vec.cbegin(), vec.cend(), elem);
			if (pos == vec.cend()) return -1;
			return std::distance(vec.cbegin(), pos);
		}; 
		
		template<typename T>
		inline int indexOf(const std::vector<T>& vec, const T& elem) {
			auto pos = std::find(vec.cbegin(), vec.cend(), elem);
			if (pos == vec.cend()) return -1;
			return std::distance(vec.cbegin(), pos);
		};

		template<typename T>
		inline int indexOf(const eastl::vector<T>& vec, const T& elem) {
			auto pos = eastl::find(vec.cbegin(), vec.cend(), elem);
			if (pos == vec.cend()) return -1;
			return eastl::distance(vec.cbegin(), pos);
		};

		template<typename V>
		inline V* get(const std::unordered_map<std::string, V>& mp, const char* key) {
			auto& f = mp.find(key);
			if (f == mp.end()) return {};
			return (*mp).second;
		};

		template<typename V>
		inline const V* get(const std::unordered_map<std::string, V>& mp, const char* key) {
			auto& f = mp.find(key);
			if (f == mp.cend()) return {};
			return (*mp).second;
		};

		template<typename V>
		inline const V* get(const std::unordered_map<Str, V>& mp, const Str& key) {
			auto f = mp.find(key);
			if (f == mp.end()) return nullptr;
			const V& ref = (*f).second;
			return &ref;
		};


		template<typename V>
		inline const V* get(const std::unordered_map<Str, V>& mp, const std::string& key) {
			return get(mp, StrRef(key));
		};

		template<typename V>
		inline V* get(std::unordered_map<Str, V>& mp, const Str& key) {
			auto f = mp.find(key);
			if (f == mp.end()) return nullptr;
			V& ref = (*f).second;
			return &ref;
		};
		
		template<typename V>
		inline V* get(eastl::unordered_map<Str, V>& mp, const Str& key) {
			auto f = mp.find(key);
			if (f == mp.end()) return nullptr;
			V& ref = (*f).second;
			return &ref;
		};

		template<typename V>
		inline V* get(eastl::unordered_map<Str, V*>& mp, const Str& key) {
			auto f = mp.find(key);
			if (f == mp.end()) return nullptr;
			V* ref = (*f).second;
			return ref;
		};

		template<typename V>
		inline V* get(std::unordered_map<Str, V*>& mp, const Str& key) {
			auto f = mp.find(key);
			if (f == mp.end()) return nullptr;
			V* ref = (*f).second;
			return ref;
		};

		template<typename T>
		inline bool remove(std::vector<T*>& vec, T* elem) {
			auto pos = std::find(vec.cbegin(), vec.cend(), elem);
			if (pos != vec.cend()) {
				vec.erase(pos);
				return true;
			}
			return false;
		};

		template<typename T>
		inline bool remove_if(std::vector<T*>& vec, std::function<bool(T*)> pred) {
			int idx = 0;
			for (auto e : vec) {
				if (pred(e)) {
					vec.erase(vec.begin() + idx);
					return true;
				}
				idx++;
			}
			return false;
		};

		template<typename K, typename V>
		std::vector<K> keys(std::unordered_map<K, V>& repo) {
			std::vector<K> ks;
			for (auto& p : repo)
				ks.push_back(p.first);
			return ks;
		};

		template<typename K, typename V>
		std::vector<V> values(std::unordered_map<K, V>& repo) {
			std::vector<V> ks;
			for (auto& p : repo)
				ks.push_back(p.second);
			return ks;
		};

		template<typename K, typename V>
		inline bool remove(std::unordered_map<K, V>& mp, V& elem) {
			auto pos = mp.find(elem);
			if (pos != mp.end()) {
				mp.erase(pos);
				return true;
			}
			return false;
		};

		template<typename K, typename V>
		inline bool remove(eastl::unordered_map<K, V>& mp, V& elem) {
			auto pos = mp.find(elem);
			if (pos != mp.cend()) {
				mp.erase(pos);
				return true;
			}
			return false;
		};

		template<typename K, typename V>
		inline bool remove(std::map<K, V>& mp, V& elem) {
			auto pos = std::find(mp.cbegin(), mp.cend(), elem);
			if (pos != mp.cend()) {
				mp.erase(pos);
				return true;
			}
			return false;
		};



		template<typename T>
		inline bool remove(std::vector<T>& vec, const T& elem) {
			auto pos = std::find(vec.cbegin(), vec.cend(), elem);
			if (pos != vec.cend()) {
				vec.erase(pos);
				return true;
			}
			return false;
		};

		template<typename T>
		inline bool remove(eastl::vector<T>& vec, const T& elem) {
			auto pos = std::find(vec.cbegin(), vec.cend(), elem);
			if (pos != vec.cend()) {
				vec.erase(pos);
				return true;
			}
			return false;
		};

		template<typename T>
		inline bool remove(eastl::vector<T*>& vec, T* elem) {
			auto pos = eastl::find(vec.cbegin(), vec.cend(), elem);
			if (pos != vec.cend()) {
				vec.erase(pos);
				return true;
			}
			return false;
		};

		inline bool stringEq(const std::string& s0, const char* s1) {
			return 0 == strncmp(s0.c_str(), s1, s0.size());
		};

		template<typename T>
		inline void nullstrip(std::vector<T*>& vec) {
			for (auto it = vec.begin(); it != vec.end();) {
				if (*it)
					it++;
				else
					it = vec.erase(it);
			}
		};

		template<typename T>
		inline eastl::vector<T*> eastlArray(const std::vector<T*>& vec) {
			eastl::vector<T*> res;
			res.reserve(vec.size());
			for (auto& k : vec)
				res.push_back(k);
			return res;
		};

		template<typename T>
		inline eastl::vector<T> eastlArray(const std::vector<T>& vec) {
			eastl::vector<T> res;
			res.reserve(vec.size());
			for (auto & k : vec)
				res.push_back(k);
			return res;
		};

		template<typename T>
		inline void nullstrip(eastl::vector<T*>& vec) {
			for (auto it = vec.begin(); it != vec.end();) {
				if (*it)
					it++;
				else
					it = vec.erase(it);
			}
		};

		template<typename T>
		inline bool isFuncEq(const std::function<T>& a, const std::function<T>& b) {
			return (a.target_type() == a.target_type())
				&& (a.template target<T>() == a.template target<T>());
		};

		template<typename T>
		inline void funcRemove(std::vector<std::function<T>>& vec, const std::function<T>& a) {
			for (auto it = vec.begin(); it != vec.end();) {
				if (isFuncEq(*it, a))
					it = vec.erase(it);
				else
					it++;
			}
		};

		template<typename T>
		inline void ringBufferPush(std::vector<T>& vec, int ringSize, const T& a) {
			if (vec.size() >= ringSize)
				vec.erase(vec.first());
			vec.push_back(a);
		};

		template<typename T>
		inline void ringBufferPush(eastl::vector<T>& vec, int ringSize, const T& a) {
			if (vec.size() >= ringSize)
				vec.erase(vec.begin());
			vec.push_back(a);
		};

		template<typename T>
		inline int nextClamped(int pos, eastl::vector<T>& vec) {
			pos++;
			if (pos >= vec.size())
				pos = vec.size() - 1;
			return pos;
		};

		template<typename T>
		inline int prevClamped(int pos, eastl::vector<T>& vec) {
			pos--;
			if (pos < 0)
				pos = 0;
			return pos;
		};

		template<typename T>
		inline int nextClamped(int pos, std::vector<T>& vec) {
			pos++;
			if (pos >= vec.size())
				pos = vec.size() - 1;
			return pos;
		};

		template<typename T>
		inline int prevClamped(int pos, std::vector<T>& vec) {
			pos--;
			if (pos < 0)
				pos = 0;
			return pos;
		};
	}
}

namespace std {
	std::ostream& operator<<(std::ostream& Str, const Pasta::Vector2& v);
	std::ostream& operator<<(std::ostream& Str, const Pasta::Vector3i& v);
	std::string to_string(const Pasta::Vector2& v);
	std::string to_string(const Pasta::Vector3& v);
	std::string to_string(const Pasta::Vector4& v);
	std::string to_string(const Pasta::Vector3i& v);
	std::string to_string(const r::Vector3s& c);
	std::string to_string(const r::Matrix44& c);
}