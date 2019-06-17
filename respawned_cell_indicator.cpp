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
	WriteRelJump(0x79DA63, UInt32(CellExpiredHook));
	return true;
}
_declspec(naked) void CellExpiredHook() {
	static const UInt32 retnAddr = 0x79DA74;
	static const UInt32 getExtraTeleportData = 0x568E50;
	_asm {
		mov ecx, [ebp - 0x94]
		call getExtraTeleportData
		mov [ebp-0x98], eax
		mov ecx, [ebp-0x98]
		call checkExpired
		test al, al
		jz done
		mov ecx, [ebp - 0x9C] // tile
		push 2
		push kTileValue_systemcolor
		mov eax, 0x700320 // Tile__PropagateFloatValue
		call eax
		jmp done
		done :
			jmp retnAddr
	}
}

bool __fastcall checkExpired(ExtraTeleport::Data* xTeleport) {
			if (xTeleport) {
				TESObjectCELL* cell = xTeleport->linkedDoor->parentCell;
				if (cell) {
				float hoursToRespawn = 0;
				float detachTime = 0;
				float gameHoursPassed = 0;
				detachTime = GetDetachTime(cell);
				if (detachTime == 0) return false;
				else if (detachTime == -1) return true;
				else {
					hoursToRespawn = (float)*(UInt32*)ThisStdCall(0x43D4D0, (char*)0x11CA160); //INISettingCollection_getValue(&gs_iHoursToRespawnCell)
					gameHoursPassed = (float)ThisStdCall(0x867E30, (UInt32*)0x11DE7B8); //TESGlobal__GetHoursPassed(g_gameTimeGlobals)
					if ((gameHoursPassed - detachTime) > hoursToRespawn) return true;
				}
			}
		}		
	return false;
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