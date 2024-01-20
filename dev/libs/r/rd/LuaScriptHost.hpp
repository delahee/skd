#pragma once

#include <sol/sol.hpp>
#include "Agent.hpp"

namespace rd {
	template <typename T>
	class PointerWrapper {
		T* ptr;
		int maxSize = 1;
	public:
		PointerWrapper() { ptr = new T(); maxSize = 1; }
		PointerWrapper(int size) { ptr = new T[size]; maxSize = size; }
		~PointerWrapper() { delete ptr; }

		T* GetPtr() { return ptr; }
		void* GetVoidPtr() { return ptr; }
		int GetSize() { return maxSize; }

		T Get() { return *ptr; }
		T GetAt(int index) { return ptr[index]; }
		void Set(T value) { *ptr = value; }
		void SetAt(int index, T value) { ptr[index] = value; }

		void MemSet(T value) { memset(ptr, value, maxSize * sizeof(T)); }
		void MemSet(T value, int size) { memset(ptr, value, size * sizeof(T)); }
		void MemCpy(const T* src, int size) { memcpy(ptr, src, size * sizeof(T)); }
		void StrCpy(const T* src) { strcpy(ptr, src); }
	};

	class LuaScriptHost : public Agent {
		bool					deleteAsked = false;
	public:
		static bool				globalStateInit;
		std::string				defaultFunc;

		static sol::state		luaEngine;
		sol::environment		localEnvironment;
		sol::unsafe_function	funcRun;
		bool					evalEachFrame = false;

		bool					init = false;
		bool					watchFile = true;
		Pasta::File *			file = nullptr;
		std::string				filePath;
		Pasta::u64				lastReadTime = 0;

		float					DELTA_CHECK = 0.1f;
		float					checkProgress = 0.0f;

		rd::Tweener				tw;

		Sig						onBeforeEval;
		Sig						onAfterContextCreation;
		Sig						onAfterEval;

		std::function<void(const std::string & err)>					log;
	public:
								LuaScriptHost(const std::string & _scriptPath, AgentList * _al, std::string _defaultFunc = "run");
		virtual					~LuaScriptHost();

		virtual void			buildContext();

		static	void			injectR(sol::state & luaSol);

		virtual void			update(double dt);
		virtual void			eval();
		virtual void			reeval();

		void					im();
	};
}//end namespace rd