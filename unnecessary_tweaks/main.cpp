#include "main.h"
#include <fstream>
extern "C" {
	BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved) {
		if (dwReason == DLL_PROCESS_ATTACH)
			CSixHandle = (HMODULE)hDllHandle;
		return TRUE;
	}
	bool NVSEPlugin_Query(const NVSEInterface *nvse, PluginInfo *info)
	{
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "Unnecessary Tweaks";
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
	handleIniOptions();
	writePatches();
	return true;
}
};

void handleIniOptions() {
	char iniPath[MAX_PATH];
	GetModuleFileNameA(CSixHandle, iniPath, MAX_PATH);
	strcpy_s((char*)(strrchr(iniPath, '\\') + 1), MAX_PATH, INI_NAME);
	CSimpleIniA ini;
	ini.SetUnicode();
	ini.LoadFile(iniPath);
	bClockMode = ini.GetOrCreateInt("Tweaks", "bClockMode", 0, "; Clock mode for Pip-Boy and Sleep Wait Menu - 0 is 12-hour clock, 1 is 24-hour clock");
	bDateFormat = ini.GetOrCreateInt("Tweaks", "bDateFormat", 0, "; Date format for Pip-Boy and Sleep Wait Menu - 0 is MM.DD.YY, 1 is DD.MM.YY");
	bPatchPickupPrompt = ini.GetOrCreateInt("Tweaks", "bPatchPickupPrompt", 0, "; Adds the following features to the pickup prompt:\n"
		";- If you pick up a stack of items, Weight and Value are multiplied by the number of items in it\n"
		";- If an item is going to over-encumber you, Weight is displayed in red\n"
		";- Weight now properly accounts for Pack Rat perk\n"
		";- Weight is displayed with one decimal, two if needed\n"
		";- Weight and Value are now displayed for item types that were ignored.");
	bEnableDeselectQuests = ini.GetOrCreateInt("Tweaks", "bEnableDeselectQuests", 0, "; Lets you deselect quests");
	bEnableContainerRespawnsWarning = ini.GetOrCreateInt("Tweaks", "bEnableContainerRespawnsWarning", 0, "; Displays a warning message if you're using an unsafe (respawning) container");
	if (!ini.HasValue("Tweaks", "sWarningText"))
		ini.SetValue("Tweaks", "sWarningText", "Storing items here seems unsafe.", "; Warning message for respawning containers", false);
	const char* warningText = ini.GetValue("Tweaks", "sWarningText", "Storing items here seems unsafe.");
	strcpy(sWarningText, warningText);
	bDisableMapMarkerPopups = ini.GetOrCreateInt("Tweaks", "bDisableMapMarkerPopups", 0, "; Disables confirmation pop-ups when setting or removing map markers");
	bShowUnvisitedCells = ini.GetOrCreateInt("Tweaks", "bShowUnvisitedCells", 0, "; Displays unvisited cells in white color on local map");
	bShowRespawnedCells = ini.GetOrCreateInt("Tweaks", "bShowRespawnedCells", 0, "; Displays respawned cells in red color on local map - by default, all interior cells respawn every 3 days");
	bHideEquippedItemsInContainers = ini.GetOrCreateInt("Tweaks", "bHideEquippedItemsInContainers", 0, "; Hides equipped items in container menu");
	bHideEquippedItemsInBarter = ini.GetOrCreateInt("Tweaks", "bHideEquippedItemsInBarter", 0, "; Hides equipped items when bartering");
	bFixDisintegrationsStat = ini.GetOrCreateInt("Bugfixes", "bFixDisintegrationsStat", 1, "; Fixes a bug that makes any ash piles increase Enemies Disintegrated stat upon entering a cell");
	bNoFoodWornOffMessage = ini.GetOrCreateInt("Tweaks", "bNoFoodWornOffMessage", 0, "; Disables \"Worn Off\" message for food");
	ini.SaveFile(iniPath, 0);
}

void writePatches() {
	bClockMode ? patchSleepWaitClock() : patchPipboyClock();
	if (bDateFormat) patchDateFormat();
	if (bPatchPickupPrompt) patchPickupPrompt();
	if (bEnableDeselectQuests) enableDeselectQuests();
	if (bEnableContainerRespawnsWarning) enableContainerRespawnsWarning();
	if (bDisableMapMarkerPopups) disableMapMarkerPopups();
	if (bShowUnvisitedCells || bShowRespawnedCells) showUnvisitedOrRespawnedCells();
	if (bFixDisintegrationsStat) fixDisintegrationsStat();
	if (bHideEquippedItemsInBarter) hideEquippedItemsInBarter();
	if (bHideEquippedItemsInContainers) hideEquippedItemsInContainers();
	if (bNoFoodWornOffMessage) patchFoodWornOffMessage();
}
void patchFoodWornOffMessage() {
	WriteRelJump(0x823F21, UInt32(WornOffHook));
}
void hideEquippedItemsInContainers() {
	WriteRelJump(0x75E847, UInt32(ContainerMenuFilterHook));
}

void hideEquippedItemsInBarter() {
	WriteRelJump(0x730658, UInt32(BarterMenuFilterHook));
}
void patchSleepWaitClock() {
	SafeWrite8(0x7C0093, 0xEB);
	SafeWrite8(0x7C00B2, 0xEB);
	SafeWrite8(0x1076497, '\0');
}

void patchPipboyClock() {
	WriteRelJump(0x79AC55, UInt32(PipboyClockHook));
}
void patchDateFormat() {
	SafeWriteBuf(0x8679C8, "\x40\x90\x50\x90", 4);
	SafeWriteBuf(0x8679D4, "\x90\x90\x90", 3); 
	WriteRelCall(0x8679C3, 0x867D20);
	WriteRelCall(0x8679CF, 0x867D60);
}
void enableDeselectQuests() {
	WriteRelJump(0x7970A1, UInt32(QuestSelectHook));
}

void patchPickupPrompt()
{
	WriteRelJump(0x7781B4, UInt32(WeightHook));
	WriteRelJump(0x778494, UInt32(WeightColorHook));
	WriteRelJump(0x77806D, UInt32(ValueHook));
	WriteRelJump(0x7787A1, UInt32(ShowMoreTypesHook));
}

void enableContainerRespawnsWarning() {
	WriteRelJump(0x75EB16, UInt32(containerRespawnsHook1));
	WriteRelJump(0x75B547, UInt32(containerRespawnsHook2));
}

void disableMapMarkerPopups() {
	WriteRelCall(0x797D82, 0x798840);
	WriteRelCall(0x797DC8, 0x798840);
	WriteRelJump(0x798846, (UInt32)MapHook);
}

void showUnvisitedOrRespawnedCells() {
	WriteRelJump(0x79DA6E, UInt32(CellExpiredHook));
}

void fixDisintegrationsStat() {
	WriteRelJump(0x8A1B4D, 0x8A1B68); //critical stage 1 jumps to incPCMiscStat
	WriteRelJump(0x8A1B2C, (UInt32)CriticalStage3Hook); // critical stage 3 jumps to IncPCMiscStat
	WriteRelJump(0x8A1B5E, (UInt32)CriticalStage24Hook); // critical stage 2 or 4 skips IncPCMiscStat
}
