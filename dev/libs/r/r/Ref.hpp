#pragma once

namespace r {

	//for aggregation
	class Ref {
	public:
		static bool		DEBUG;
		Ref()			{ nbRef = 1; };
		Ref(const Ref&) { nbRef = 1; };

		virtual			~Ref();

		virtual int		incrRef();
		inline int		addRef() { return incrRef(); }
		inline int		acquire() { return incrRef(); }
		inline int		lock() { return incrRef(); }

		virtual void	decRef();
		inline void		release() { decRef(); };
		inline void		unlock() { decRef(); };

		inline int		refCount() const { return nbRef; }

		void			reset() { nbRef = 1; };
		void			clear() { nbRef = 0; };
		void			im(const char * prefix);
	protected:
		int				nbRef = 0;
		bool			debug = false;
	};

	class SoftRef {
	public:
		static bool		DEBUG;
						SoftRef() { nbRef = 1; };
						SoftRef(const SoftRef&) { nbRef = 1; };

		virtual			~SoftRef();

		virtual int		incrRef() { return ++nbRef; }
		virtual void	decRef();
		inline int		refCount() const { return nbRef; }

		void			reset() { nbRef = 1; };
		void			clear() { nbRef = 0; };
		void			im(const char* prefix);
	protected:
		int				nbRef = 0;
	};

	template<typename T>
	class PtrRef {
	public:
		T*				ptr = 0;
		SoftRef			ref;

						PtrRef();
		void			set(T*p);
		void			clear();
		inline int		addRef() { ref.incrRef(); }
		inline auto		refCount()const { return ref.refCount(); };

		void			decRef() { //if you want to go bellow zero test refcount
			ref.decRef(); 
			if (ref.refCount() == 0) { 
				delete ptr; 
				ptr = 0; 
			} 
			if (ref.refCount() < 0)
				PASTA_ASSERT(ref.refCount()<0);
		};
	};

	template<typename T>
	inline PtrRef<T>::PtrRef(){
		ref.clear();
	}

	template<typename T>
	inline void PtrRef<T>::set(T* p) {
		ref.reset();
		ptr = p;
	};

	template<typename T>
	inline void PtrRef<T>::clear() {
		ptr = 0;
		ref.clear();
	};
}