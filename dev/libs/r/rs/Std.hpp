#pragma once

#include <vector>
#include <algorithm>
#include <map>
#include "EASTL/vector.h"
#include "EASTL/unordered_map.h"

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
		inline std::vector<Dst> map( const std::vector<Src>& src, std::function<Dst(const Src& elem)> f) {
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
		inline void filter(std::vector<Elem>& dst, const std::vector<Elem>& src, std::function<bool(Elem elem)> f) {
			dst.clear();
			if (src.size() > dst.size())
				dst.reserve(src.size());
			for (auto it = src.begin(); it != src.end(); it++)
				if(f(*it))
					dst.push_back(*it);
		};

		template<typename T>
		inline bool exists(const eastl::vector<T>& vec, const T& elem) {
			return std::find(vec.cbegin(), vec.cend(), elem) != vec.cend();
		};

		template<typename T>
		inline bool exists(const std::vector<T>& vec, const T& elem) {
			return std::find(vec.cbegin(), vec.cend(), elem) != vec.cend();
		};

		template<typename T, typename V>
		inline bool exists(const std::unordered_map<T,V>& mp, const T& key) {
			return mp.find(key) != mp.cend();
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

		static inline bool exists(const std::vector<std::string>& mp, const char* key) {
			for (auto& s : mp)
				if (0 == strcmp(s.c_str(), key))
					return true;
			return false;
		};

		static inline bool exists(const std::vector<Str>& mp, const char* key) {
			for (auto& s : mp)
				if (s == key)
					return true;
			return false;
		};

		template<typename T>
		inline int findPos(const std::vector<T>& vec,const T& key) {
			auto pos = std::find(vec.cbegin(), vec.cend(), key);
			if (pos == vec.end())return -1;
			return pos - vec.begin();
		};

		inline int findPos(const std::vector<std::string>& vec, const char * key) {
			auto pos = std::find(vec.cbegin(),vec.cend(),key);
			if (pos == vec.end())return -1;
			return pos - vec.begin();
		};

		inline int findPos(const std::vector<std::string>& vec, const std::string & key) {
			auto pos = std::find(vec.cbegin(), vec.cend(), key);
			if (pos == vec.end())return -1;
			return pos - vec.begin();
		};

		template<typename T>
		inline int find(const std::vector<T>& vec, const T & elem) {
			auto pos = std::find(vec.cbegin(), vec.cend(), elem);
			if( pos == vec.cend() ) return -1;
			return std::distance(vec.cbegin(), pos);
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
		inline V* get(std::unordered_map<Str, V*>& mp, const Str& key) {
			auto f = mp.find(key);
			if (f == mp.end()) return nullptr;
			V* ref = (*f).second;
			return ref;
		};

		

		template<typename T>
		inline bool remove(std::vector<T*> & vec, T * elem) {
			auto pos = std::find(vec.cbegin(), vec.cend(), elem);
			if (pos != vec.cend()) {
				vec.erase(pos);
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
		
		inline bool stringEq(const std::string & s0, const char* s1) {
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
		}

		template<typename T>
		inline void nullstrip(eastl::vector<T*>& vec) {
			for (auto it = vec.begin(); it != vec.end();) {
				if (*it)
					it++;
				else
					it = vec.erase(it);
			}
		}

		template<typename T>
		inline bool isFuncEq( const std::function<T> & a, const std::function<T> & b) {
			return (a.target_type() == a.target_type())
				&& (a.template target<T>() == a.template target<T>());
		};

		template<typename T>
		inline void funcRemove(std::vector< std::function<T> >& vec, const std::function<T>& a) {
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
	}

	inline std::string to_string(const Pasta::Vector2& v) {
		return std::string("{") + std::to_string(v.x) + std::string(",") + std::to_string(v.y) + std::string("}");
	};

	inline std::string to_string(const Pasta::Vector3i& v) {
		return std::string("{") + std::to_string(v.x) + std::string(",") + std::to_string(v.y) + std::string(",") + std::to_string(v.z) + std::string("}");
	};

}

namespace std {
	inline std::ostream& operator<<(std::ostream& Str, const Pasta::Vector2& v) {
		Str << std::string("{") + std::to_string(v.x) + std::string(",") + std::to_string(v.y) + std::string("}");
		return Str;
	};

	inline std::ostream& operator<<(std::ostream& Str, const Pasta::Vector3i& v) {
		Str << std::string("{") + std::to_string(v.x) + std::string(",") + std::to_string(v.y) + std::string(",") + std::to_string(v.z) + std::string("}");
		return Str;
	};
}