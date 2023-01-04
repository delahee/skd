#pragma once
#include "r/Color.hpp"

namespace Pasta {
	struct JReflect;
}

namespace rd {
	class Anon;

	class Vars {
	public:
		mutable rd::Anon*	
					head = 0;

					Vars();
					Vars(const Vars& other);
					~Vars();

		bool		has(const char* name) const;

		void		destroy(const char* name);

		rd::Anon*	get(const char* name);
		rd::Anon*	get(const char* name) const;
		rd::Anon*	getOrCreate(const char* name);

		rd::Anon*	add(rd::Anon* nmeta);

		rd::Anon*	set(const char* name, const r::u8* content, int nbBytes);
		rd::Anon*	set(const char* name, const float* content, int nbFloats);
		rd::Anon*	set(const char* name, const Vector3* content, int nbVec3);
		rd::Anon*	set(const char* name, const void** content, int nbPtr);
		rd::Anon*	set(const char* name, const char* content);
		rd::Anon*	set(const char* name, const std::string& content);
		rd::Anon*	set(const char* name, float content);
		rd::Anon*	set(const char* name, double content);
		rd::Anon*	set(const char* name, int content);
		rd::Anon*	set(const char* name, bool content);
		rd::Anon*	set(const char* name, r::u64 content);
		rd::Anon*	set(const char* name, r::u32 content);
		rd::Anon*	set(const char* name, const r::Vector3& content);
		rd::Anon*	set(const char* name, void* content);
		rd::Anon*	set(const char* name, const r::Color& content);

		void		setUntyped(const char* name, const char* content);

		template<typename T>
		inline void	ensure(const char* name, const T& dflt) {
			auto v = get(name);
			if (!v) set(name, dflt);
		};

		void		incr(const char* name);

		rd::Anon*	searchWithPrefix(const char* prefix);
		void		cleanByPrefix(const char* prefix);

		bool		im(bool showAsHeader = true);
		void		serialize(Pasta::JReflect* jr, const char* name = 0);

		void		load(const Vars& v);
		bool		copy(const char* name, Vars& v);

		void		dispose();

		Vars&		operator+=(const Vars& other);
		Vars&		operator+=(const rd::Anon* e);
		Vars&		operator=(const rd::Anon & e);
		Vars&		operator=(const rd::Vars & e);

		//deletes all "other" vars already existing here and copies them
		//avoid this at game runtime, it generates fragmentation
		void		merge(Vars& other);
		void		loadWithFilter(Vars& other, std::vector<std::string>& varFilters);

		void		traverse(std::function<void(rd::Anon*)> f);

		const char*	getString(const char* name, const char* dflt = 0) const;
		StrRef		getStr(const char* name) const;
		bool		getBool(const char* name, bool dflt = false);
		int			getInt(const char* name, int dflt = 0) const;
		r::u32		getU32(const char* name, r::u32 dflt = 0) const;
		r::uid		getUID(const char* name, r::uid dflt = 0);
		float		getFloat(const char* name, float dflt = 0.0f) const;
		void*		getPtr(const char* name, void* dflt = nullptr) const;
		r::Color	getColor(const char* name, const r::Color & dflt = r::Color::White) const;

		r::opt<r::Vector3>				getVec3(const char* name) const;
		r::opt<std::vector<void*>>		getPtrArray(const char* name) const;
		r::opt<std::vector<Vector3>>	getVec3Array(const char* name) const;

		int			getMemorySize();
		int			count() const;
	};

	template<typename Ty>
	inline void		setVar(Vars* v, const char* name, const Ty& val) {};
}

