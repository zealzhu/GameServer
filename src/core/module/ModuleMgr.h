#ifndef _MODULE_MGR_H
#define _MODULE_MGR_H

#include "IModule.h"
#include <vector>
#include <unordered_map>

typedef IModule *(*GetModuleFunc)();

class ModuleMgr
{
public:
	static ModuleMgr * Instance() {
		static ModuleMgr instance;
		return &instance;
	}

	bool Initialize(IContext * context);
	void Destory(IContext * context);

	ModulePtr FindModule(const char * name);

private:

	std::vector<ModulePtr> modules_;
	std::unordered_map <std::string, ModulePtr> modules_map_;
	static IContext * pContext;
};

#endif // _MODULE_MGR_H