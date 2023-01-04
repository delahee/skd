#include "stdafx.h"
#include "GAMEApplication.h"

int main(int argc, char* argv[])
{
	GAMEApplication app;

	for (int i = 1; i < argc; i++) {
		if (std::string(argv[i]).rfind("--", 0) == 0 && i + 1 < argc) {
			app.params.emplace(std::string(argv[i] + 2), std::string(argv[i + 1])); i++;
		} else if (std::string(argv[i]).rfind("-", 0) == 0 )
			app.params.emplace(std::string(argv[i] + 1), "");
	}


	app.setResourcePath("res/");
	
	app.init();

	app.executeAll();

	app.close();

	return 0;
}

#ifdef PASTA_WIN
// This is the entry point used if not in a console application
#include <windows.h>
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	// TODO foward command line arguments
	return main(0, NULL);
}
#endif
#ifdef PASTA_NX
#include "2-application/NNUserMgr.h"
extern "C" void nnMain()
{
	auto userMgr = new Pasta::NNUserMgr();
	userMgr->init(Pasta::CUserManagementDesign::SingleFixedUser);
	main(0, NULL);
}
#endif



