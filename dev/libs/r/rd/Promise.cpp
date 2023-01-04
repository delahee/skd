#include "stdafx.h"

#include "Promise.hpp"

using namespace std;
using namespace rd;

eastl::vector<Promise*> rd::Promise::deleteList;

static bool init = false;
static void promiseInit() {
	auto ap = new AnonAgent(Promise::exitFrame);
	ap->name = "Promise GC";
	rs::Sys::exitFrameProcesses.push_back(ap);
}

Promise::Promise() {
	if (!init) {
		init = true;
		promiseInit();
	}
}

Promise::~Promise() {
	
}

Promise * rd::Promise::reset() {
	_succeeded = false;
	_failed = false;

	if (thens.size())	thens.clear();
	if (rejects.size()) rejects.clear();

	curSuccess.reset();
	curFailure.reset();

	return this;
}

 Promise * rd::Promise::then(Handler accept, Handler reject) {
	 if (_succeeded) {
		 if (accept) {
			 try {
				 curSuccess = accept(this, curSuccess);
			 }
			 catch (std::any ex) {
				 _succeeded = false;
				 _failed = true;
				 curFailure = ex;
				 curFailure = reject(this, curFailure);
			 }
		 }
	 }
	 else if (_failed) {
		 if (reject)
			 curFailure = reject(this, curFailure);
	 }
	 else {
		 thens.push_back(accept);
		 if (reject) rejects.push_back(reject);
	 }
	 return this;
 }

 Promise* rd::Promise::autoReleased() {
	 _autoRelease = true;
	 finally(self_release);
	 return this;
 }

 Promise * rd::Promise::then(Handler accept) {
	 if (_succeeded) {
		 if (accept) {
			 try {
				 curSuccess = accept(this, curSuccess);
			 }
			 catch (std::any ex) {
				 _succeeded = false;
				 _failed = true;
				 curFailure = ex;
				 curFailure = reject( curFailure);
			 }
		 }
	 }
	 else if (_failed) {
		 //skip
	 }
	 else {
		 thens.push_back(accept);
	 }
	 return this;
 }

 Promise * rd::Promise::catchError(Handler reject) {
	 if (_failed)
		 curFailure = reject(this, curFailure);
	 rejects.push_back(reject);
	 return this;
 }

Promise * rd::Promise::error(Handler reject){
	return catchError(reject);
}

Promise * rd::Promise::finally(Handler both){
	if (isSettled()) {
		auto& cur = _succeeded ? curSuccess : curFailure;
		cur = both(this, cur);
	}
	else
		finals.push_back(both);
	return this;
}

 Promise* rd::Promise::accept(std::any d) {
	 return resolve(d);
 }

Promise * rd::Promise::resolve(std::any d) {
	 _failed = false;
	 _succeeded = true;
	 curSuccess = d;

	 for (Handler & s : thens) {
		 try {
			 curSuccess = s(this, curSuccess);
		 }
		 catch (std::any ex) {
			 reject(ex);
			 return this;
		 }
	 }

	 thens.clear();
	 rejects.clear();

	 for (Handler& s : finals) {
		 try {
			 curSuccess = s(this, curSuccess);
		 }
		 catch (std::any ex) {
			 //do nothing
			 return this;
		 }
	 }
	 finals.clear();
	 return this;
}

Promise * rd::Promise::reject(std::any d) {
	 _succeeded = false;
	 _failed = true;
	 curFailure = d;
	 for (Handler & s : rejects)
		 curFailure = s(this, curFailure);
	 thens.clear();
	 rejects.clear();

	 for (Handler& s : finals) {
		 try {
			 curFailure = s(this, curFailure);
		 }
		 catch (std::any ex) {
			 //do nothing
			 return this;
		 }
	 }
	 finals.clear();
	 return this;
}

rd::Promise* rd::Promise::release(){
	if (std::find(deleteList.begin(),deleteList.end(),this) == deleteList.end())
		deleteList.push_back(this);
	return this;
}


Promise * rd::Promise::alloc(){
	return new Promise();
}

void rd::Promise::exitFrame(){
	if (deleteList.size())
		for(auto elem = deleteList.begin(); elem != deleteList.end();) {
			Promise * prom = (*elem);
			elem = deleteList.erase(elem);
			delete prom;
		}
}

Promise * rd::Promise::getFailure(std::any data) {
	auto p = new Promise();
	p->reject(data);
	return p;
}

Promise * rd::Promise::getSuccess(std::any data) {
	auto p = new Promise();
	p->resolve(data);
	return p;
}

std::any rd::Promise::self_release(Promise * p, std::any data){
	p->release();
	return data;
}

