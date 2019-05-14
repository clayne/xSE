#include "nvse/nvse/PluginAPI.h"
#include "nvse/nvse/nvse_version.h"
#include "nvse/nvse/SafeWrite.h"
#include "nvse/nvse/GameObjects.h"
bool versionCheck(const NVSEInterface* nvse);
void patchContainerMenu();
void ContainerMenuHook1();
void ContainerMenuHook2();
void handleIniOptions();
bool __stdcall GetContainerRespawns(TESObjectREFR* cont);
char g_sWarningText[260];
HMODULE CSixHandle;

extern "C" {

	BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved) {
		if (dwReason == DLL_PROCESS_ATTACH)
			CSixHandle = (HMODULE)hDllHandle;
		return TRUE;
	}


	bool NVSEPlugin_Query(const NVSEInterface *nvse, PluginInfo *info) {
		/* fill out the info structure */
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "Container Respawns NVSE";
		info->version = 1;
		handleIniOptions();
		return versionCheck(nvse);
	}

	bool NVSEPlugin_Load(const NVSEInterface *nvse) {
		patchContainerMenu();
		return true;
	}

};

void patchContainerMenu() {
	WriteRelJump(0x75B545, (UInt32)ContainerMenuHook1);
	WriteRelJump(0x75EB14, (UInt32)ContainerMenuHook2);
}
_declspec (naked) void ContainerMenuHook1() {
	static const UInt32 retnAddr = 0x75B551;
	static const UInt32 g_containerMenu = 0x11D93F8;
	static const UInt32 defaultTextAddr = 0x1011584;
	static const char* text = g_sWarningText;
	__asm {
		mov ecx, dword ptr ds:[g_containerMenu]
		mov edx, dword ptr ss:[ebp+0x08]
		test edx, edx
		jz skip
		mov dword ptr ds:[ecx+0x74], edx
		push edx
		call GetContainerRespawns
		test al, al
		jz skip
		push 1
		push text
		push 0xFC4
		jmp retnAddr
		skip:
			push 1
			push defaultTextAddr
			push 0xFC4
			jmp retnAddr
	}
}
_declspec (naked) void ContainerMenuHook2() {
	static const UInt32 retnAddr = 0x75EB20;
	static const UInt32 g_containerMenu = 0x11D93F8;
	static const UInt32 defaultTextAddr = 0x1011584;
	static const char* text = g_sWarningText;
	__asm {
		mov ecx, g_containerMenu
		mov edx, dword ptr ss : [ecx + 0x74]
		test edx, edx
		jz skip
		push edx
		call GetContainerRespawns
		test al, al
		jz skip
		push 1
		push text
		push 0xFC4
		jmp retnAddr
		skip :
		push 1
			push defaultTextAddr
			push 0xFC4
			jmp retnAddr
	}
}
bool __stdcall GetContainerRespawns(TESObjectREFR* containerRef) {
		TESObjectCONT *container = (TESObjectCONT*)containerRef->baseForm;
		return (container->flags & 2) ? 1 : 0;
}
void handleIniOptions() {
	char filename[MAX_PATH];
	GetModuleFileNameA(CSixHandle, filename, MAX_PATH);
	strcpy((char *)(strrchr(filename, '\\') + 1), "container_respawns_warning.ini");
	GetPrivateProfileStringA("Tweaks", "sWarningText", "Storing items here seems unsafe.", g_sWarningText, 260, filename);
	
}
bool versionCheck(const NVSEInterface* nvse) {
	if (nvse->isEditor) return false;
	if (nvse->nvseVersion < NVSE_VERSION_INTEGER) {
		_ERROR("NVSE version too old (got %08X expected at least %08X)", nvse->nvseVersion, NVSE_VERSION_INTEGER);
		return false;
	}
	return true;
}
