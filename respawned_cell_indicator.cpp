#include "nvse/PluginAPI.h"
#include "nvse/GameObjects.h"
#include "nvse/GameData.h"
#include "nvse/GameProcess.h"
#include "nvse/GameEffects.h"
#include "nvse/GameSettings.h"
#include "nvse/GameTasks.h"
#include "nvse/GameRTTI.h"
#include "nvse/GameOSDepend.h"
#include "nvse/GameUI.h"
#include "nvse/SafeWrite.h"
#include "internal/decoding.h"
#define PLUGIN_VERSION 1

bool NVSEPlugin_Query(const NVSEInterface *nvse, PluginInfo *info)
{
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "Respawned Cell Indicator";
	info->version = PLUGIN_VERSION;

	if (nvse->isEditor)
	{
		return false;
	}
	if (nvse->runtimeVersion != RUNTIME_VERSION_1_4_0_525)
	{
		_MESSAGE("ERROR: Unsupported runtime version (%08X).", nvse->runtimeVersion);
		return false;
	}
	if (nvse->nvseVersion < 0x5010040)
	{
		_MESSAGE("ERROR: NVSE version is outdated. This plugin requires v5.14 minimum.");
		return false;
	}
	return true;
}

bool NVSEPlugin_Load(const NVSEInterface *nvse)
{
	WriteRelJump(0x79DA6E, UInt32(CellExpiredHook));
	return true;
}
void __fastcall DoCellColorIcon(Tile* tile, ExtraTeleport::Data* xTeleport) {
	if (!xTeleport) return;

	TESObjectCELL* cell = xTeleport->linkedDoor->parentCell;
	if (!cell) return;

	int detachTime = GetDetachTime(cell);

	if (detachTime)
	{
		int hoursToRespawn = *(int*)0x11CA164;
		int gameHoursPassed = ThisStdCall(0x867E30, (UInt32*)0x11DE7B8); //TESGlobal__GetHoursPassed(g_gameTimeGlobals);
		if ((gameHoursPassed - detachTime) > hoursToRespawn)
		{
			/* respawned cell */
			ThisStdCall(0x700320, tile, kTileValue_systemcolor, 2);  // Tile__PropagateFloatValue
		}
	}
	else
	{
		/* unvisited cell */
		ThisStdCall(0x700320, tile, kTileValue_systemcolor, 0);  // Tile__PropagateFloatValue
	}
}

_declspec(naked) void CellExpiredHook() {
	static const UInt32 retnAddr = 0x79DA74;
	_asm {
	originalCode:
		mov[ebp - 0x98], eax

			mov ecx, [ebp - 0x9C] // tile
			mov edx, eax
			call DoCellColorIcon
			jmp retnAddr
	}
}

__declspec(naked) SInt32 __fastcall GetDetachTime(TESObjectCELL *cell) {
	__asm {
		push	kExtraData_DetachTime
		add		ecx, 0x28
		call	BaseExtraList::GetByType
		test	eax, eax
		jz done
		mov eax, [eax + 0xC]
		done:
		retn
	}
}