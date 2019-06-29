#include "nvse/nvse/PluginAPI.h"
#include "nvse/nvse/nvse_version.h"
#include "nvse/nvse/SafeWrite.h"
#include "nvse/nvse/GameObjects.h"
#include "nvse/nvse/GameTypes.h"
bool versionCheck(const NVSEInterface* nvse);
HMODULE CSixHandle;
void QuestSelectHook();
extern "C" {

	BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved) {
		if (dwReason == DLL_PROCESS_ATTACH)
			CSixHandle = (HMODULE)hDllHandle;
		return TRUE;
	}


	bool NVSEPlugin_Query(const NVSEInterface *nvse, PluginInfo *info) {
		/* fill out the info structure */
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "Deselect Quests NVSE";
		info->version = 1;
		return versionCheck(nvse);
	}

	bool NVSEPlugin_Load(const NVSEInterface *nvse) {
		WriteRelJump(0x797090, UInt32(QuestSelectHook));
		return true;
	}

};

//credits to JazzIsParis
void __fastcall ClearActiveQuest(PlayerCharacter* g_thePlayer)
{
	if (g_thePlayer->quest != NULL) { 
		g_thePlayer->quest = NULL;
		if (g_thePlayer->questTargetList.Count() != 0) //6C4
			g_thePlayer->questTargetList.RemoveAll();
	}
}

_declspec(naked) void QuestSelectHook() {
	static const UInt32 retnAddr = 0x7970AD;
	static const UInt32 notEqualAddr = 0x7970A2;
	static const UInt32 getSelected = 0x7A1910;
	_asm {
		mov ecx, [ebp-0x1F8]
		add ecx, 0x130
		call getSelected
		mov ecx, dword ptr ds:[0x11DEA3C]
		cmp eax, [ecx + 0x6B8]
		jne notEqual
		push ecx
		call ClearActiveQuest
		jmp retnAddr
		notEqual:
			push eax
			jmp notEqualAddr
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
