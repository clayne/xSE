#include "nvse/nvse/PluginAPI.h"
#include "nvse/nvse/nvse_version.h"
#include "nvse/nvse/SafeWrite.h"
bool versionCheck(const NVSEInterface* nvse);
void CriticalStage3Hook();
void CriticalStage24Hook();
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
		info->name = "Disintegration Stat Fix";
		info->version = 1;
		return versionCheck(nvse);
	}

	bool NVSEPlugin_Load(const NVSEInterface *nvse) {
		WriteRelJump(0x8A1B4D, 0x8A1B68); //critical stage 1 jumps to incPCMiscStat
		WriteRelJump(0x8A1B2C, (UInt32)CriticalStage3Hook); // critical stage 3 jumps to IncPCMiscStat
		WriteRelJump(0x8A1B5E, (UInt32)CriticalStage24Hook); // critical stage 2 or 4 skips IncPCMiscStat
		return true;
	}

};

_declspec (naked) void CriticalStage3Hook() {
	static const UInt32 retnAddr = 0x8A1BCA;
	static const UInt32 incStatAddr = 0x8A1B68;
	__asm {
		cmp [ebp-0xC], 0x3
		je incStat
		jmp retnAddr
		incStat:
			jmp incStatAddr

	}
}
_declspec (naked) void CriticalStage24Hook() {
	static const UInt32 retnAddr = 0x8A1BCA;
	static const UInt32 callAddr = 0x450F90;
	__asm {
		push 1
		mov ecx, [ebp-0x04]
		call callAddr
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
