#include "f4se/f4se/PluginAPI.h"
#include "f4se/f4se_common/f4se_version.h"
#include "f4se/f4se_common/SafeWrite.h"
#include "f4se/xbyak/xbyak.h"
#include "f4se/f4se/GameData.h"
#include <f4se/f4se_common/BranchTrampoline.h>
#define ModuleBase ((uintptr_t)GetModuleHandleA(nullptr))
IDebugLog	gLog;
PluginHandle	g_pluginHandle = kPluginHandle_Invalid;
RelocAddr <uintptr_t*> startAddr = 0x0D1DC5D;
RelocAddr <uintptr_t*> endAddr = 0x0D1DC7A;
RelocAddr <uintptr_t*> hookAddr = 0x0D1DCCA;
uintptr_t fmt = (uintptr_t)"%d.%02d.2%u        %u:%02u"; //new string to hide AM/PM tag
class jobhook : public Xbyak::CodeGenerator
{
public:
	jobhook() : Xbyak::CodeGenerator(1024)
	{
		Xbyak::Label label;
		mov(r8, ptr[rip + label]);
		jmp(ptr[rip]); // jmp retnAddr
		dq(ModuleBase + 0x0D1DCD1);
		L(label);
			dq(fmt);
		
	}
} static jobhookInstance;

extern "C"
{

bool F4SEPlugin_Query(const F4SEInterface * f4se, PluginInfo * info)
{

	info->infoVersion =	PluginInfo::kInfoVersion;
	info->name =		"24 Hour Clock";
	info->version =		1;

	g_pluginHandle = f4se->GetPluginHandle();

	if(f4se->isEditor)
	{
		_FATALERROR("loaded in editor, marking as incompatible");
		return false;
	}

	return true;
}


bool F4SEPlugin_Load(const F4SEInterface * f4se)
{
	if (!g_branchTrampoline.Create(1024 * 1)) {
		_ERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
		return false;
	}

	g_branchTrampoline.Write6Branch(hookAddr.GetUIntPtr(), (uintptr_t)jobhookInstance.getCode()); 
	SafeWriteJump(startAddr.GetUIntPtr(), endAddr.GetUIntPtr()); //jumping over the 24h-12h conversion
	return true;
}

};