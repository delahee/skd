#pragma once

namespace r {
	template<typename T>
	using opt = typename std::optional<T>;
	using proc = typename std::function<void(void)>;
}

//could probably do the same with a clever inline stuff but ain't got no time for a new master
#define LBD_THIS( func ) [this](){ this->func(); }
#define LBD_THIS1( func ) [this](auto a){ this->func(a); }
#define LBD_THIS2( func ) [this](auto a, auto b){ this->func(a,b); }

#define LBD_MEM_CBK( obj , func ) [=](){ obj->func(); }
#define LBD_MEM_CBK1( obj , func ) [=](auto a){ obj->func(a); }
#define LBD_MEM_CBK2( obj , func ) [=](auto a,auto b){ obj->func(a,b); }

#define LBD_MEM_CBK_RET( obj , func ) [=]()->auto { return obj->func(); }
#define LBD_MEM_CBK2_RET( obj , func ) [=](auto a,auto b)->auto{ return obj->func(a,b); }

#define SINGLETON_IMPL( T ) T* T::me = 0;  T& T::get() { if (!T::me) T::me = new T(); return *T::me; }
#define SINGLETON_DECL( T ) static T* me; static T& get(); T(const T&) = delete;
