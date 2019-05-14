#include "nvse/nvse/PluginAPI.h"
#include "nvse/nvse/nvse_version.h"
#include "nvse/nvse/SafeWrite.h"
#include "nvse/nvse/GameObjects.h"
bool versionCheck(const NVSEInterface* nvse);
void patchMapMenu();
void MapHook();
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
		info->name = "No Map Marker Popups";
		info->version = 1;
		return versionCheck(nvse);
	}

	bool NVSEPlugin_Load(const NVSEInterface *nvse) {
		patchMapMenu();
		return true;
	}

};

void patchMapMenu() {
	WriteRelCall(0x797D82, 0x798840); //calling SetOrRemoveMapMarker instead of ShowMessageBox (move or leave marker message)
	WriteRelCall(0x797DC8, 0x798840); //calling SetOrRemoveMapMarker instead of ShowMessageBox (set or not set marker message)
	WriteRelJump(0x798846, (UInt32)MapHook); 
}
_declspec (naked) void MapHook() {
	static PlayerCharacter* g_thePlayer = *(PlayerCharacter**)0x11DEA3C;
	static const UInt32 isMapMarkerSetSub = 0x798400;
	static const UInt32 retnAddr = 0x798854;
	__asm {
		mov ecx, g_thePlayer
		mov eax, isMapMarkerSetSub 
		call eax // checking if marker is already set
		test eax, eax
		jz isNotSet
		mov byte ptr ss:[ebp-8], 1 //removing marker 
		jmp retnAddr
		isNotSet:
			mov byte ptr ss : [ebp-8], 3 //setting marker
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
