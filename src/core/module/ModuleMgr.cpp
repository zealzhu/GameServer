#include "ModuleMgr.h"
#include <cassert>
#include "config/ConfigMgr.h"

#ifdef linux
#include <dlfcn.h>
#elif WIN32
#include <windows.h>
#endif

IContext * ModuleMgr::pContext = nullptr;

bool ModuleMgr::Initialize(IContext * context)
{
	pContext = context;

	auto & modules_name = ConfigMgr::Instance()->GetCoreConfig().modules_name;

	char path[255] = { 0 };
	for(auto it = modules_name.begin(); it != modules_name.end(); it++) {
#ifdef WIN32
		sprintf(path, "%s.dll", (*it).c_str());
		HINSTANCE hinst = LoadLibrary(path);
		DWORD dwError = 0;
		if (hinst == NULL)
		{
			dwError = GetLastError();
			printf("load library error: %d\n", dwError);
		}
		GetModuleFunc func = (GetModuleFunc)GetProcAddress(hinst, "GetModule");
#elif linux
		sprintf(path, "lib%s.so", (*it).c_str());
		void * handle = dlopen(path, RTLD_LAZY);
		assert(handle);
		GetModuleFunc func = (GetModuleFunc)dlsym(handle, "GetModule");
#endif // WIN32

		assert(func);
		IModule * module = func();
		InfoLog("Init module %s.", module->GetModuleName());
		if (!module->Initialize(context)) {
			assert(false);
		}
		modules_map_[module->GetModuleName()] = module;
		modules_.emplace_back(module);

		InfoLog("Start module %s.", module->GetModuleName());
		module->Start(context);
	}


	return true;
}

void ModuleMgr::Destory(IContext * pContext)
{
	auto temp = modules_.begin();
	for (auto it = modules_.begin(); it != modules_.end();) {
		temp = it;
		auto module = *it;
		module->Destory(pContext);
		modules_map_.erase(module->GetModuleName());
		it = modules_.erase(it);
	}
	InfoLog("Destory all modules");
}

ModulePtr ModuleMgr::FindModule(const char * name)
{
	return ModulePtr();
}

