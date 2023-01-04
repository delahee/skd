#pragma once

namespace r {
	template<typename T>
	using opt = typename std::optional<T>;


	
}

//could probably do the same with a clever inline stuff but ain't got no time for a new master
#define LBD_MEM_CBK( obj , func ) [=](){ obj->func(); }
#define LBD_MEM_CBK1( obj , func ) [=](auto a){ obj->func(a); }
#define LBD_MEM_CBK2( obj , func ) [=](auto a,auto b){ obj->func(a,b); }

#define LBD_MEM_CBK_RET( obj , func ) [=]()->auto { return obj->func(); }
#define LBD_MEM_CBK2_RET( obj , func ) [=](auto a,auto b)->auto{ return obj->func(a,b); }
