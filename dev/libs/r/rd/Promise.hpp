#pragma once

#include <functional>
#include <any>

namespace rd {
	/**
	* avoid passing pointers that needs destructors calling upon termination
	* but since any can be copy constructible safe, you may want 
	*/
	class Promise {

	public:
		/**
		* std::any f(Promise*self, std::any result){ return nullptr; }
		*/
		typedef std::function<std::any(Promise*self, std::any result)>	Handler;
		
		/**
		* At end of frame the promise will be garbaged 
		*/
		bool						_autoRelease = false;

		std::vector<Handler>		thens;
		std::vector<Handler>		rejects;
		std::vector<Handler>		finals;

		std::any					curSuccess;
		std::any					curFailure;

	public:
							//gives out an unsettled promise
							Promise();

		/**
		* let's make promises virtualisable to allow "delete" result madness occur
		*/
		virtual				~Promise();
		bool				isSettled() { return _succeeded || _failed; };
		Promise*			reset();

		Promise*			then(Handler accept, Handler reject);
		Promise*			then(Handler accept);
		Promise*			catchError(Handler reject);
		/**
		* Alias for catchError
		*/
		Promise*			error(Handler reject);


		/**
		* finally
		*/
		Promise*			finally(Handler both);

		/** Do accept the promise with this data */
		Promise*			resolve(std::any d);
		Promise*			accept(std::any d);

		/** Do reject the promise with this data */
		Promise*			reject(std::any d);

		//promise will be destroyed at end of frame
		Promise*			release();

		/**after then or err, push promise into release() queue
		*/ 
		Promise*			autoReleased();

		/**for future pooling if bandwidth goes boom
		*/
		static Promise *	alloc();
		static void			exitFrame();

		/** create a new failed promise which will stand in error */
		static Promise *	getFailure(std::any data);

		/** create a new successful promise which will stand in error */
		static Promise *	getSuccess(std::any data);
	protected:

		/**
		* the promise has already 
		*/ 
		bool				_succeeded = false; 

		bool				_failed = false;

		static std::any		self_release(Promise* p, std::any data);

		static eastl::vector<Promise*>	deleteList;
	};
}
