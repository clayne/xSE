#include "nvse/ParamInfos.h"
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
#include <time.h>
#include "discord_rpc.h"
#define PLUGIN_VERSION 2 // 1.1
#define REG_CMD(name) nvse->RegisterCommand(&kCommandInfo_##name);

NVSEStringVarInterface *StrIfc = NULL;
PlayerCharacter *g_thePlayer = NULL;
InterfaceManager *g_interfaceManager = NULL;
UInt32 locationNameID = NULL;
HMODULE CSixHandle;

static const char* APPLICATION_ID;
int64_t StartTime;
int SendPresence = 1;
bool bInitialized = 0;
bool bShowCaps = false;
bool bShowLevel = false;
bool bShowName = false;
bool bShowHealth = false;
bool bShowGambling = false;
bool bShowPaused = false;
bool bShowHacking = false;
bool bShowLockpicking = false;
bool bShowLocation = false;
bool bShowSleeping = false;
bool bShowPipboy = false;
static void UpdateDiscordPresence(char* state, char* details)
{
	if (bInitialized) {
		if (SendPresence) {
			DiscordRichPresence discordPresence;
			memset(&discordPresence, 0, sizeof(discordPresence));
			if (strlen(state) > 0)
			discordPresence.state = state;
			if (strlen(details) > 0)
			discordPresence.details = details;
			discordPresence.startTimestamp = StartTime;
			discordPresence.largeImageKey = "fnv-big";
			discordPresence.instance = 0;
			Discord_UpdatePresence(&discordPresence);
		//	_MESSAGE("Updating Rich Presence");
		}
		else {
			Discord_ClearPresence();
		}
		Discord_RunCallbacks();
	}
}

static void handleDiscordReady(const DiscordUser* connectedUser)
{
	_MESSAGE("Connected to user %s#%s", connectedUser->username, connectedUser->discriminator);
}

void handleDiscordDisconnected(int errcode, const char* message)
{
	_MESSAGE("Discord disconnected: %s %d", message, errcode);
}

void handleDiscordError(int errcode, const char* message)
{
	_MESSAGE("Discord error: %s errorcode %d", message, errcode);
}

static void DiscordInit()
{
	if (!bInitialized) {
		_MESSAGE("Initializing connection to discord");
		DiscordEventHandlers handlers;
		memset(&handlers, 0, sizeof(handlers));
		handlers.ready = handleDiscordReady;
		handlers.errored = handleDiscordError;
		handlers.disconnected = handleDiscordDisconnected;
		Discord_Initialize(APPLICATION_ID, &handlers, 0, NULL);
		StartTime = time(0);
		bInitialized = 1;
		g_thePlayer = *(PlayerCharacter**)0x11DEA3C;
		g_interfaceManager = *(InterfaceManager**)0x11D8A80;
	}
}

ParamInfo kParams_TwoInts_OneFloat_OneInt[4] =
{
	{ "Integer", kParamType_Integer, 0 },
	{"Integer", kParamType_Integer, 0 },
	{ "Float", kParamType_Float, 0 },
	{ "Integer", kParamType_Integer, 0 }
};

UInt32 InterfaceManager::GetTopVisibleMenuID()
{
	if (currentMode < 2) return 0;
	if (activeMenu) return activeMenu->id;
	UInt32 topMenu = 0, *mnStack = menuStack;
	while (*mnStack) topMenu = *mnStack++;
	if (topMenu != 1) return topMenu;
	return 0;
}