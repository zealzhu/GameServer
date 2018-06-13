#include "Context.h"
#include <thread>

int main()
{
	IContext * pContext = new Context();
	std::thread command_thread = std::thread([pContext]() {
		char buff[255] = { 0 };
		scanf("%s", buff);
		while (strcmp(buff, "exit") != 0) {}
		pContext->Stop();
	});

    pContext->Initialize();
    pContext->Run();
    pContext->Destory();

	command_thread.join();
	InfoLog("Exit server.");
	delete pContext;
    return 0;
}
