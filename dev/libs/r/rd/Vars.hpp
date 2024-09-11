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
		rd::Anon*	safeGet(const char* name);

		rd::Anon*	get(const char* name) const;
		rd::Anon*	safeGet(const char* name) const;

		rd::Anon*	getOrCreate(const char* name);

		rd::Anon*	add(rd::Anon* nmeta);

		rd::Anon*	set(const char* name, const r::u8* content, int nbBytes);
		rd::Anon*	set(const char* name, const float* content, int nbFloats);
		rd::Anon*	set(const char* name, const int* content, int nbInts);
		rd::Anon*	set(const char* name, const Vector3* content, int nbVec3);
		rd::Anon*	set(const char* name, const void** content, int nbPtr);
		rd::Anon*	set(const char* name, const char* content);
		rd::Anon*	set(const char* name, const Str& content) { return set(name, content.c_str()); };
		rd::Anon*	set(const char* name, const std::string& content);
		rd::Anon*	set(const char* name, float content);
		rd::Anon*	set(const char* name, double content);
		rd::Anon*	set(const char* name, int content);
		rd::Anon*	set(const char* name, bool content);
		rd::Anon*	set(const char* name, r::u64 content);
		rd::Anon*	set(const char* name, r::u32 content);
		rd::Anon*	set(const char* name, void* content);
		rd::Anon*	set(const char* name, const r::Vector2& content);
		rd::Anon*	set(const char* name, const r::Vector2i& content);
		rd::Anon*	set(const char* name, const r::Vector3& content);
		rd::Anon*	set(const char* name, const r::Vector4i& content);
		rd::Anon*	set(const char* name, const r::Vector4& content);
		rd::Anon*	set(const char* name, const r::Color& content);
		rd::Anon*	set(const char* name, rd::Agent* ptr);
		rd::Anon*	set(const char* name, r2::Node* ptr);
		rd::Anon*	setPtr(const char* name, void* ptr);

		void		setUntyped(const char* name, const char* content);

		template<typename T>
		inline void	ensure(const char* name, const T& dflt) {
			auto v = get(name);
			if (!v) set(name, dflt);
		};

		void		incr(const char* name);

		rd::Anon*	searchWithPrefix(const char* prefix);
		void		cleanByPrefix(const char* prefix);

		bool		im(bool showAsHeader = true,const char * name = 0);
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
		void		traverse(std::function<void(const rd::Anon*)> f) const;

		const char*	getString(const char* name, const char* dflt = 0) const;
		StrRef		getStr(const char* name) const;
		bool		getBool(const char* name, bool dflt = false) const;
		int			getInt(const char* name, int dflt = 0) const;
		r::u32		getU32(const char* name, r::u32 dflt = 0) const;
		r::uid		getUID(const char* name, r::uid dflt = 0);
        float		getFloat(const char* name, float dflt = 0.0f) const;
        double		getDouble(const char* name, double dflt = 0.0f) const;
		void*		getPtr(const char* name, void* dflt = nullptr) const;
		r::Color	getColor(const char* name, const r::Color & dflt = r::Color::White) const;
		bool		getFloatBuffer(const char* name, std::vector<float> & out) const;
		bool		getBuffer(const char* name, r::u8 * out, int & sz) const;

		r::opt<r::Vector2>				getVec2(const char* name) const;
		r::opt<r::Vector2i>				getVec2i(const char* name) const;
		r::opt<r::Vector3>				getVec3(const char* name) const;
		r::opt<r::Vector4i>				getVec4i(const char* name) const;
		r::opt<r::Vector4>				getVec4(const char* name) const;
		r::Vector3						getVec3(const char* name,Vector3 dflt) const;
		r::Vector4						getVec4(const char* name,Vector4 dflt) const;
		r::opt<std::vector<void*>>		getPtrArray(const char* name) const;
		r::opt<std::vector<Vector3>>	getVec3Array(const char* name) const;

		int			getMemorySize();
		int			count() const;
		bool		isEmpty() const;

		template<typename T>
		inline T	operator[](const char* name);

		template<>
		inline bool	operator[]<bool>(const char* name) {
			return getBool(name, false);
		};

		template<>
		inline r::uid operator[]<r::uid>(const char* name) {
			return getUID(name, false);
		};

		bool		hasTag(const char * name) const;
		void		setTag(const char * name, bool onOff = true);
		void		addTag(const char * name);
		void		removeTag(const char * name);
		Str			getTags(); 

		bool		isEqual(const rd::Vars&ov)const;

		static const char* TAGS_NAME;
	};


	struct IntProperty {
		Str			fieldname;
		rd::Vars* v = 0;

		IntProperty(const char* fieldname, rd::Vars* v);

		int			get();
		void		set(int val);
	};

	template<typename Ty>
	inline void		setVar(Vars* v, const char* name, const Ty& val) {};
}
