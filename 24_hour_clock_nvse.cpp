#include "nvse/nvse/PluginAPI.h"
#include "nvse/nvse/nvse_version.h"
#include "nvse/nvse/SafeWrite.h"
bool versionCheck(const NVSEInterface* nvse);
void patchTime();
void PMHook();
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
		info->name = "24 Hour Clock";
		info->version = 1;
		return versionCheck(nvse);
	}

	bool NVSEPlugin_Load(const NVSEInterface *nvse) {
		patchTime();
		return true;
	}

};

void patchTime() {
	WriteRelJump(0x7C0080, 0x7C00D1); //jumping over 24h to 12h conversion
	WriteRelJump(0x7C014E, (UInt32)PMHook); //hiding AM/PM tag
}
_declspec (naked) void PMHook() {
	static const UInt32 retnAddr = 0x7C0153;
	static const UInt32 fmt = (UInt32)"%s, %s, %d:%02d";
	__asm {
		push fmt
		jmp retnAddr
	}
}
bool versionCheck(const NVSEInterface* nvse) {
	if (nvse->isEditor) return false;
	if (nvse->nvseVersion < NVSE_VERSION_INTEGER) {
		_ERROR("NVSE version too old (got %08X expected at least %08X)", nvse->nvseVersion, NVSE_VERSION_INTEGER);
		return false;
	}
	return true;
}
