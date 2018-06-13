#ifndef _IMODULE_H
#define _IMODULE_H

#include <IContext.h>

#define MODULE_MAX_NAME_LENGTH 64

class IModule
{
public:
	virtual bool Initialize(IContext * pContext) = 0;
	virtual void Start(IContext * pContext) = 0;
	virtual void Destory(IContext * pContext) = 0;

public:
	inline void SetModuleName(const char * name) {
		strcpy(name_, name);
	}

	inline const char * GetModuleName() {
		return name_;
	}

private:
	char name_[MODULE_MAX_NAME_LENGTH];
};

#ifdef WIN32
#define GET_DLL_ENTRANCE \
	static IModule * pModule = nullptr; \
	extern "C" __declspec(dllexport) IModule * __cdecl GetModule() { \
		return pModule; \
	} \
	BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) { \
		return true; \
	}
#else
#define GET_DLL_ENTRANCE \
	static IModule * pModule = nullptr; \
	extern "C" IModule * GetModule() { \
		return pModule; \
	}
#endif //WIN32

#define CREATE_MODULE(name) \
	class factory##name \
	{ \
	public: \
		factory##name(IModule * module) \
		{ \
			IModule * module##name = new name; \
			module##name->SetModuleName(#name); \
			pModule = module##name; \
		} \
	}; \
	factory##name factory(pModule);

#endif