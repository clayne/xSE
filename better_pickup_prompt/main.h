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
#include "nvse/GameUI.cpp"
#include "nvse/SafeWrite.h"
#include "internal/decoding.h"
#define PLUGIN_VERSION 1
bool fmod_d(double weight);
bool isEncumbered(double itemWeight);
float __fastcall GetExtraCount(TESObjectREFR* crosshairRef);
bool __fastcall CheckType(TESObjectREFR* crosshairRef);
double __fastcall ModifyLightItems(double itemWeight);
void(*ApplyPerkModifiers)(UInt32 entryPointID, TESObjectREFR *perkOwner, void *arg3, ...) = (void(*)(UInt32, TESObjectREFR*, void*, ...))0x5E58F0;

__declspec(naked) void WeightHook() {
	static const UInt32 retnAddr = 0x7781C5;
	static const UInt32 twodec = (UInt32)"%.2f";
	static const UInt32 onedec = (UInt32)"%.1f";
	__asm {
		fld [ebp-0x470]
		sub esp, 8
		fstp qword ptr [esp]
		call ModifyLightItems
		fstp [ebp-0x470]
		mov ecx, [ebp+0x08] //crosshairRef
		call GetExtraCount
		fmul [ebp - 0x470] //item weight
		fst [ebp-0x470]
		sub esp, 8
		fstp qword ptr [esp]
		call fmod_d
		test al, al
		jz oneDecimal
		fld [ebp-0x470]
		sub esp, 8
		fstp qword ptr [esp]
		push twodec
		jmp retnAddr
		oneDecimal:
			fld [ebp - 0x470]
			sub esp, 8
			fstp qword ptr [esp]
			push onedec
			jmp retnAddr
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
		fld [ebp-0x470]
		sub esp, 8
		fstp qword ptr[esp]
		call isEncumbered
		test al, al
		jz notEncumbered
		push 2
		push 0xFF4
		jmp retnAddr
		notEncumbered:
			push 1
			push 0xFF4
			jmp retnAddr
	}
}

bool isEncumbered(double itemWeight) {
	PlayerCharacter* g_thePlayer = *(PlayerCharacter**)0x11DEA3C;
	float avCarryWeight = g_thePlayer->avOwner.GetActorValue(13);
	float avInventoryWeight = g_thePlayer->avOwner.GetActorValue(46);
	return (avInventoryWeight + itemWeight) > avCarryWeight;
}

__declspec(naked) void ValueHook() {
	static const UInt32 retnAddr = 0x778073;
	__asm {
		mov ecx, [ebp + 0x08] //crosshairRef
		call GetExtraCount
		fmul [ebp - 0x46C] //item value
		fst [ebp - 0x46C]
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
		mov eax, [ebp-0x58]
		or eax, 0xFF
		mov [ebp-0x58], eax
		jmp done
		done:
		mov eax, [ebp-0x58]
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

