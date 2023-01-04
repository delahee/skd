#pragma once

#include "2-application/PLATFORMApplication.h"
#include "2-application/GameState.h"

class GAMEApplication : public PLATFORMApplication 
{
public:
	std::unordered_map<std::string, std::string> params;

	GAMEApplication();
	virtual ~GAMEApplication();

	virtual void init();
	virtual void close();

	void		winInit();

	virtual Pasta::GameState *createGameState(int state);
	virtual const char* getAppName() const { return "Deathtower - Editor"; }

	virtual int executeState(Pasta::Duration deltaTime );
	virtual void onResolutionChange(int _windowID)override;

};

