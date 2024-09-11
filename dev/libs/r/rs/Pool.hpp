#pragma once

namespace rs {
	
	template< typename T >
	class Pool {
	public:
		bool						initialized = true;//for static alloc correctness
		eastl::vector<T*>			active;
		eastl::vector<T*>			repo;

		std::function<T*(void)>		allocator;
		std::function<void(T*)>		deallocator;
		std::function<void(T*)>		reset;

		std::function<void(T*)>		onAlloc;
		std::function<void(T*)>		onFree;

	public:
		Pool()				{}
		Pool(const Pool& p) = delete;

		T* alloc();
		void reserve( int nb );
		void release(T* elem);
		void fastFree(T* elem);//faster but unsafe ( esp for tools or debug stuff )
		void safeFree(T* elem);//slower but safe ( esp for tools or debug stuff )
		void resetAll();
		
		virtual ~Pool();
	};

	template<typename T>
	Pool<T>::~Pool() {
		try{
			//Sources of crash 
			//1- manual new but sent to pool or vice versa
			//2- was free to the wrong pool by the generic free dispatcher

			bool tryDeallocateActives = false;
			if (tryDeallocateActives) {
				int idx = 0;
				for (auto e : active) {
					if (idx == 45) 
						int here = 0;
					{//because of course sometime we are stupid
						//am bored let's avoid these possibilities
						auto pos = eastl::find(repo.begin(), repo.end(), e);
						while (pos != repo.end()) {
							repo.erase(pos);
							pos = eastl::find(repo.begin(), repo.end(), e);
						}
					}

					{
						auto pos = eastl::find(active.begin(), active.end(), e);
						while (pos != active.end()) {//if we get here this may be bad
							active.erase(pos);
							pos = eastl::find(active.begin(), active.end(), e);
						}
					}

					(deallocator) ? deallocator(e) : delete e;//was already deleted?

					idx++;
					if (idx >= active.size())
						break;
				}
			}
			active.clear();
			int pos = 0;
			for (auto e : repo) {
				if (pos == 0) {
					int breakOnDelete = 0;
				}
				//if you are getting here, an active probably was freed and deleted ( see int breakOnWrongUse = 0;)
				(deallocator) ? deallocator(e) : delete e;//was already deleted?
				pos++;//help debug
			}
			repo.clear();
		}
		catch ( std::exception e) {
			printf("[Pool] delete exception\n");
		}
		initialized = false;
	}

	template<typename T>
	T* Pool<T>::alloc(){
		if(!initialized)
			return new T();

		T * elem = nullptr;
		if (repo.size() == 0)
			elem = (allocator != nullptr) ? allocator() : new T();
		else{
			elem = repo.back();
			repo.pop_back();
		}
		if (reset ) reset(elem);
		if (onAlloc) onAlloc(elem);

		active.push_back(elem);
		return elem;
	};

	template<typename T>
	void Pool<T>::resetAll() {
		for (auto e : active){
			if (onFree) onFree(e);
			if (reset) reset(e);
			repo.push_back(e);
		}
		active.clear();
	};

	template<typename T>
	void Pool<T>::fastFree(T* elem) {
		using namespace std;
		if (elem == nullptr) return;

		if (onFree) onFree(elem);
		if (reset) reset(elem);

#ifdef _DEBUG
		auto alreadyPos = std::find(repo.begin(), repo.end(), elem);
		if (alreadyPos != repo.end()) {
			std::cout << "ERROR was already in repo!\n";
			return;
		}
#endif

		repo.push_back(elem);
	};

	
	template<typename T>
	void Pool<T>::release(T* elem){
		using namespace std;
		if (elem == nullptr) return;

		if (onFree) onFree(elem);
		if (reset) reset(elem);

		auto alreadyPos = std::find(repo.begin(), repo.end(), elem);
		if (alreadyPos != repo.end()) {
#ifdef _DEBUG
			std::cout << "ERROR was already in repo!\n";
#endif
			return;
		}

		repo.push_back(elem);
		auto pos = std::find(active.begin(),active.end(),elem);
		if( pos != active.end())
			active.erase(pos);
		else {
#ifdef _DEBUG
			std::cout << "ERROR was not in actives!\n";
#endif
		}
	};

	template<typename T>
	void Pool<T>::safeFree(T* elem) {
		if (elem == nullptr) return;

		if (onFree) onFree(elem);
		if (reset) reset(elem);

		auto alreadyPos = std::find(repo.begin(), repo.end(), elem);
		if (alreadyPos != repo.end())
			return;//already in 

		repo.push_back(elem);

		auto pos = std::find(active.begin(), active.end(), elem);
		if (pos != active.end())
			active.erase(pos);
		else{
			return;//already in 
		}
	};

	template<typename T>
	void Pool<T>::reserve(int nb) {
		if(repo.size() < nb ) repo.reserve(nb);
		for (int i = 0; i < nb; ++i)
			repo.push_back((allocator != nullptr) ? allocator() : new T());
	}

#if 0
	//to have at least one compiled pool at project start
	static Pool<int> __intpool;
#endif
}