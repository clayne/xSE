#include "main.h"
#include "commands.h"
IDebugLog		gLog;
void MessageHandler(NVSEMessagingInterface::Message* msg);

BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved) {
	if (dwReason == DLL_PROCESS_ATTACH)
		CSixHandle = (HMODULE)hDllHandle;
	return TRUE;
}

bool NVSEPlugin_Query(const NVSEInterface *nvse, PluginInfo *info)
{
	gLog.Open("discord_integration.log");
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "Discord Integration";
	info->version = PLUGIN_VERSION;

	if (nvse->isNogore)
	{
		_ERROR("Unsupported");
		return false;
	}

	if (nvse->nvseVersion < NVSE_VERSION_INTEGER)
	{
		_ERROR("NVSE version too old (got %08X expected at least %08X)", nvse->nvseVersion, NVSE_VERSION_INTEGER);
		return false;
	}

	if (!nvse->isEditor)
	{
		if (nvse->runtimeVersion < RUNTIME_VERSION_1_4_0_525)
		{
			_ERROR("incorrect runtime version (got %08X need at least %08X)", nvse->runtimeVersion, RUNTIME_VERSION_1_4_0_525);
			return false;
		}
	}
	else
	{
		if (nvse->editorVersion < CS_VERSION_1_4_0_518)
		{
			_ERROR("incorrect editor version (got %08X need at least %08X)", nvse->editorVersion, CS_VERSION_1_4_0_518);
			return false;
		}
	};
	_MESSAGE("DiscordRPC version %d passed NVSE Query.", info->infoVersion);
	return true;
}

bool NVSEPlugin_Load(const NVSEInterface *nvse)
{
	char filename[MAX_PATH];
	GetModuleFileNameA(CSixHandle, filename, MAX_PATH);
	strcpy_s((char *)(strrchr(filename, '\\') + 1), MAX_PATH, "discord_integration.ini");
	bShowCaps = GetPrivateProfileIntA("GENERAL", "bShowCaps", 0, filename);
	bShowLevel = GetPrivateProfileIntA("GENERAL", "bShowLevel", 0, filename);
	bShowHealth = GetPrivateProfileIntA("GENERAL", "bShowHealth", 0, filename);
	bShowName = GetPrivateProfileIntA("GENERAL", "bShowName", 0, filename);
	bShowGambling = GetPrivateProfileIntA("GENERAL", "bShowGambling", 0, filename);
	bShowPaused = GetPrivateProfileIntA("GENERAL", "bShowPaused", 0, filename);
	bShowHacking = GetPrivateProfileIntA("GENERAL", "bShowHacking", 0, filename);
	bShowLockpicking = GetPrivateProfileIntA("GENERAL", "bShowLockpicking", 0, filename);
	bShowLocation = GetPrivateProfileIntA("GENERAL", "bShowLocation", 0, filename);
	bShowSleeping = GetPrivateProfileIntA("GENERAL", "bShowSleeping", 0, filename);
	bShowPipboy = GetPrivateProfileIntA("GENERAL", "bShowPipboy", 0, filename);
	((NVSEMessagingInterface*)nvse->QueryInterface(kInterface_Messaging))->RegisterListener(nvse->GetPluginHandle(), "NVSE", MessageHandler);
	StrIfc = (NVSEStringVarInterface*)nvse->QueryInterface(kInterface_StringVar);
	nvse->SetOpcodeBase(0x37F0);
	REG_CMD(InitRPC);
	REG_CMD(IsRPCInitialized)
	REG_CMD(UpdateRPC);
	_MESSAGE("DiscordRPC loaded.");
	return true;
}
void MessageHandler(NVSEMessagingInterface::Message* msg)
{

	switch (msg->type) {
		case NVSEMessagingInterface::kMessage_ExitGame_Console:
		case NVSEMessagingInterface::kMessage_ExitGame:
			if (bInitialized) {
				_MESSAGE("Closing connection");
				Discord_Shutdown();
				bInitialized = 0;
			}
			break;
		default:
			break;
	}

}