#include "nvse/nvse/PluginAPI.h"
#include "nvse/nvse/nvse_version.h"
#include "nvse/nvse/SafeWrite.h"
bool versionCheck(const NVSEInterface* nvse);
void HourConversionHook();
void AMPMHook();
void StringFmtHook();
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
		WriteRelJump(0x79AC28, (UInt32)HourConversionHook);
		WriteRelJump(0x79AC05, (UInt32)StringFmtHook);
		WriteRelJump(0x79AC55, (UInt32)AMPMHook);
		return true;
	}

};

_declspec (naked) void StringFmtHook() {
	static const UInt32 retnAddr = 0x79AC0D;
	static const UInt32 convertAddr = 0xEC62C0;
	static const char* pm = "PM";
	static const char* am = "AM";
	__asm {
		fld [ebp - 0x18]
		call convertAddr
		cmp eax, 12
		jge itspmtime
		push am
		mov [ebp-0x28], eax
		jmp retnAddr
		itspmtime:
			push pm
			mov[ebp - 0x28], eax
			jmp retnAddr

	}
}
_declspec (naked) void HourConversionHook() {
	static const UInt32 retnAddr = 0x79AC30;
	static const UInt32 convertAddr = 0xEC62C0;
	
	__asm {
		fld dword ptr ss : [ebp - 0x18]
		call convertAddr
		cmp eax, 0
		je midnight
		cmp eax, 12
		jg convert
		jmp retnAddr
		convert:
			sub eax, 12
			jmp retnAddr
		midnight:
			mov eax, 12
			jmp retnAddr
	}
}

_declspec (naked) void AMPMHook() {
	static const UInt32 retnAddr = 0x79AC5A;
	static const UInt32 fmt = (UInt32)"%s, %d:%02d %s";
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
