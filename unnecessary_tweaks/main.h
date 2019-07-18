#pragma once

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
#include "SimpleINILibrary/SimpleIni.h"
#define PLUGIN_VERSION 1
#define INI_NAME "unnecessary_tweaks.ini"
HMODULE CSixHandle;

void writePatches();
void handleIniOptions();
void patchPipboyClock(), patchSleepWaitClock(), patchPickupPrompt(), enableDeselectQuests(),
enableContainerRespawnsWarning(), disableMapMarkerPopups(), showUnvisitedOrRespawnedCells(),
fixDisintegrationsStat(), patchDateFormat(), hideEquippedItemsInContainers(), hideEquippedItemsInBarter(),
patchFoodWornOffMessage();
double __fastcall ModifyLightItems(double itemWeight);
bool fmod_d(double weight);
float __fastcall GetExtraCount(TESObjectREFR* crosshairRef);
bool isEncumbered(double itemWeight);
bool __fastcall CheckType(TESObjectREFR* crosshairRef);
SInt32  __fastcall GetDetachTime(TESObjectCELL *cell);
bool __fastcall isFood(MagicItem* magicItem);
int bClockMode = 0;
int bDateFormat = 0;
int bPatchPickupPrompt = 0;
int bEnableDeselectQuests = 0;
int bEnableContainerRespawnsWarning = 0;
char sWarningText[260];
int bDisableMapMarkerPopups = 0;
int bShowUnvisitedCells = 0;
int bShowRespawnedCells = 0;
int bHideEquippedItemsInContainers = 0;
int bHideEquippedItemsInBarter = 0;
int bNoFoodWornOffMessage = 0;
int bFixDisintegrationsStat = 0;
void(*ApplyPerkModifiers)(UInt32 entryPointID, TESObjectREFR *perkOwner, void *arg3, ...) = (void(*)(UInt32, TESObjectREFR*, void*, ...))0x5E58F0;

__declspec(naked) void PipboyClockHook() {
	static const UInt32 retnAddr = 0x79AC5A;
	static const char* fmt = "%s, %d:%02d %s";
	_asm {
		push fmt 

		mov eax, [ebp - 0x3C] // hours
		cmp eax, 12
		je midDay
		jg afterMidday

		/* AM */
		cmp eax, 0
		jne afterMidnight
		add eax, 12
		afterMidnight:
		mov ecx, 0x105E660 // sAMTime
			jmp setHours

			/* PM */
			afterMidday :
		sub eax, 12
			midDay :
			mov ecx, 0x105E66C // sPMTime

							   /* set function paramaters */
			setHours :
			mov[ebp - 0x3C], eax
			mov[ebp - 0x34], ecx  // am/pm

			jmp retnAddr
	}
}

// eax contains new quest. If the active quest is equal to the new quest, clear it
_declspec(naked) void QuestSelectHook() {
	static const UInt32 retnAddr = 0x7970A8;
	_asm {
		mov ecx, dword ptr ds : [0x11DEA3C]
		cmp eax, [ecx + 0x6B8] // g_thePlayer->activeQuest
		jne notEqualQuest

		xor eax, eax

		notEqualQuest :
		push eax
			jmp retnAddr
	}
}

__declspec(naked) void WeightHook() {
	static const UInt32 retnAddr = 0x7781C5;
	static const UInt32 twodec = (UInt32)"%.2f";
	static const UInt32 onedec = (UInt32)"%.1f";
	__asm {
		fld[ebp - 0x470]
		sub esp, 8
		fstp qword ptr[esp]
		call ModifyLightItems
		fstp[ebp - 0x470]
		mov ecx, [ebp + 0x08] //crosshairRef
		call GetExtraCount
		fmul[ebp - 0x470] //item weight
		fst[ebp - 0x470]
		sub esp, 8
		fstp qword ptr[esp]
		call fmod_d
		test al, al
		jz oneDecimal
		fld[ebp - 0x470]
		sub esp, 8
		fstp qword ptr[esp]
		push twodec
		jmp retnAddr
		oneDecimal :
		fld[ebp - 0x470]
			sub esp, 8
			fstp qword ptr[esp]
			push onedec
			jmp retnAddr
	}
}
__declspec(naked) void WornOffHook() {
	static const UInt32 retnAddr = 0x823F29;
	static const UInt32 skipAddr = 0x823F49;
	static const UInt32 getName = 0x408DA0;
	__asm {
		mov ecx, [ebp - 0x20]
		call isFood
		test al, al
		jnz skip
		mov ecx, [ebp - 0x20]
		call getName
		jmp retnAddr
		skip:
			mov [ebp-0x25], 0
			jmp skipAddr
	}
}
bool __fastcall isFood(MagicItem* magicItem) {
	TESForm* mgForm = DYNAMIC_CAST(magicItem, MagicItem, TESForm);
	BGSEquipType* pEquipType = DYNAMIC_CAST(mgForm, TESForm, BGSEquipType);
	if (pEquipType) {
		return (pEquipType->equipType == 12);
	}
}
double __fastcall ModifyLightItems(double itemWeight) {
	float perkMod = -1;
	double fPackRatThreshold = -1, fPackRatModifier = -1;
	PlayerCharacter* g_thePlayer = *(PlayerCharacter**)0x11DEA3C;
	ApplyPerkModifiers(kPerkEntry_ModifyLightItems, g_thePlayer, &perkMod);
	if (perkMod > 0.0)
	{
		GameSettingCollection* gmsts = GameSettingCollection::GetSingleton();
		Setting* setting = NULL;
		gmsts->GetGameSetting("fPackRatThreshold", &setting);
		if (setting) setting->Get(&fPackRatThreshold);
		if (fPackRatThreshold >= itemWeight)
		{
			gmsts->GetGameSetting("fPackRatModifier", &setting);
			if (setting) setting->Get(&fPackRatModifier);
			itemWeight = itemWeight * fPackRatModifier;
		}
	}
	return itemWeight;
}

float __fastcall GetExtraCount(TESObjectREFR* crosshairRef) {
	float count = 1;
	if (crosshairRef) {
		ExtraCount* pXCount = GetExtraType(crosshairRef->extraDataList, Count);
		if (pXCount) return pXCount->count;
	}
	return count;
}

bool fmod_d(double weight) {
	double result = (weight - 0.1 * floor(weight / 0.1));
	return (result > 0.01);
}

__declspec(naked) void WeightColorHook() {
	static const UInt32 retnAddr = 0x77849B;
	__asm {
		mov eax, [ebp - 0x2C]
		cmp eax, 1
		je checkEncumbered
		cmp eax, 6
		je checkEncumbered
		jmp notEncumbered
		checkEncumbered :
		fld[ebp - 0x470]
			sub esp, 8
			fstp qword ptr[esp]
			call isEncumbered
			test al, al
			jz notEncumbered
			push 2
			push 0xFF4
			jmp retnAddr
			notEncumbered :
		push 1
			push 0xFF4
			jmp retnAddr
	}
}

bool isEncumbered(double itemWeight) {
	PlayerCharacter* g_thePlayer = *(PlayerCharacter**)0x11DEA3C;
	float avCarryWeight = g_thePlayer->avOwner.GetActorValue(13);
	float avInventoryWeight = g_thePlayer->avOwner.GetActorValue(46);
	return (avInventoryWeight + itemWeight) >= (avCarryWeight + 1);
}

__declspec(naked) void ValueHook() {
	static const UInt32 retnAddr = 0x778073;
	__asm {
		mov ecx, [ebp + 0x08] //crosshairRef
		call GetExtraCount
		fmul[ebp - 0x46C] //item value
		fst[ebp - 0x46C]
		jmp retnAddr
	}
}

__declspec(naked) void ShowMoreTypesHook() {
	static const UInt32 retnAddr = 0x7787A7;
	__asm {
		mov ecx, [ebp + 0x08] //crosshairRef
		call CheckType
		test al, al
		jz done
		mov eax, [ebp - 0x58]
		or eax, 0xFF
		mov[ebp - 0x58], eax
		jmp done
		done :
		mov eax, [ebp - 0x58]
			and eax, 8
			jmp retnAddr
	}
}

bool __fastcall CheckType(TESObjectREFR* crosshairRef) {
	switch (crosshairRef->baseForm->typeID) {
	case kFormType_TESObjectBOOK:
	case kFormType_TESKey:
	case kFormType_TESCaravanCard:
	case kFormType_TESCaravanMoney:
	case kFormType_TESCasinoChips:
		return true;
	default:
		return false;
	}
}

_declspec(naked) void containerRespawnsHook1() {
	static const UInt32 retnAddr = 0x75EB1B;
	static const char* text = sWarningText;
	_asm {
		mov eax, dword ptr ds : [0x11D93F8]
		mov eax, ss : [eax + 0x74]
		test eax, eax
		jz noRespawn

		mov eax, [eax + 0x20]
		movzx eax, byte ptr[eax + 0x98]
		shr eax, 1
		and eax, 1
		test al, al
		jz noRespawn

		push text
		jmp retnAddr

		noRespawn :
		push 0x1011584
			jmp retnAddr

	}
}

_declspec(naked) void containerRespawnsHook2() {
	static const UInt32 retnAddr = 0x75B54C;
	static const char* text = sWarningText;
	_asm {
		mov eax, [ebp + 8]
		test eax, eax
		jz noRespawn

		mov eax, [eax + 0x20]
		movzx eax, byte ptr[eax + 0x98]
		shr eax, 1
		and eax, 1
		test al, al
		jz noRespawn

		push text
		jmp retnAddr

		noRespawn :
		push 0x1011584
			jmp retnAddr

	}
}

_declspec (naked) void MapHook() {
	static PlayerCharacter* g_thePlayer = *(PlayerCharacter**)0x11DEA3C;
	static const UInt32 isMapMarkerSetSub = 0x798400;
	static const UInt32 retnAddr = 0x798854;
	__asm {
		mov ecx, g_thePlayer
		mov eax, isMapMarkerSetSub
		call eax
		test eax, eax
		jz isNotSet
		mov byte ptr ss : [ebp - 8], 1
		jmp retnAddr
		isNotSet :
		mov byte ptr ss : [ebp - 8], 3
			jmp retnAddr
	}
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
			if (bShowRespawnedCells)
				ThisStdCall(0x700320, tile, kTileValue_systemcolor, 2);  // Tile__PropagateFloatValue
		}
	}
	else
	{
		/* unvisited cell */
		if (bShowUnvisitedCells)
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

_declspec (naked) void CriticalStage3Hook() {
	static const UInt32 retnAddr = 0x8A1BCA;
	static const UInt32 incStatAddr = 0x8A1B68;
	__asm {
		cmp[ebp - 0xC], 0x3
		je incStat
		jmp retnAddr
		incStat :
		jmp incStatAddr

	}
}

_declspec (naked) void CriticalStage24Hook() {
	static const UInt32 retnAddr = 0x8A1BCA;
	static const UInt32 callAddr = 0x450F90;
	__asm {
		push 1
		mov ecx, [ebp - 0x04]
		call callAddr
		jmp retnAddr

	}
}
__declspec (naked) void ContainerMenuFilterHook() {
	static const UInt32 retnAddr = 0x75E857;
	static const UInt32 hideAddr = 0x75E850;
	static const UInt32 getWorn = 0x4BDDD0;
	__asm {
		push    0
		mov     ecx, [ebp + 0x08]
		call    getWorn
		movzx   edx, al
		test    edx, edx
		jnz hide
		mov [ebp-0x30], 0
		jmp retnAddr
		hide:
			jmp hideAddr
	}
}

__declspec (naked) void BarterMenuFilterHook() {
	static const UInt32 retnAddr = 0x730668;
	static const UInt32 hideAddr = 0x730661;
	static const UInt32 getWorn = 0x4BDDD0;
	__asm {
		push    0
		mov     ecx, [ebp + 0x08]
		call    getWorn
		movzx   edx, al
		test    edx, edx
		jnz hide
		mov[ebp - 0x30], 0
		jmp retnAddr
		hide :
		jmp hideAddr
	}
}