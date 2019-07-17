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
typedef void(*MainLoopAddCallbackExProc)(void* cmdPtr, void* thisObj, UInt32 callCount, UInt32 callDelay);

PlayerCharacter *g_thePlayer = NULL;
InterfaceManager *g_interfaceManager = NULL;
StartMenu *g_startMenu = NULL;
TESForm* g_capsItem;
HMODULE CSixHandle;
void BuildStateStr(char* stateStr), BuildDetailsStr(char* detailsStr), UpdateRPC();
double __cdecl GetPlayerLevel(), GetPlayerHealthPercentage();
static const char* APPLICATION_ID;
int64_t StartTime;
bool g_isInitialized = 0;
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
bool bShowReading = false;
bool bShowPipboy = false;
bool bShowInMainMenu = false;
bool bAllowCustomActions = false;
char g_customString[256] = { 0 };
static void UpdateDiscordPresence(char* state, char* details)
{
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	if (strlen(state))
		discordPresence.state = state;
	if (strlen(details))
		discordPresence.details = details;
	discordPresence.startTimestamp = StartTime;
	discordPresence.largeImageKey = "fnv-big";
	discordPresence.instance = 0;
	Discord_UpdatePresence(&discordPresence);
	Discord_RunCallbacks();
	
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
	if (!g_isInitialized) {
		g_thePlayer = *(PlayerCharacter**)0x11DEA3C;
		g_interfaceManager = *(InterfaceManager**)0x11D8A80;
		g_startMenu = *(StartMenu**)0x11DAAC0;
		g_capsItem = LookupFormByID(0xF);
		_MESSAGE("Initializing connection to discord");
		DiscordEventHandlers handlers;
		memset(&handlers, 0, sizeof(handlers));
		handlers.ready = handleDiscordReady;
		handlers.errored = handleDiscordError;
		handlers.disconnected = handleDiscordDisconnected;
		Discord_Initialize(APPLICATION_ID, &handlers, 0, NULL);
		StartTime = time(0);
		g_isInitialized = 1;
		
	}
}

UInt32 InterfaceManager::GetTopVisibleMenuID()
{
	if (currentMode < 2) return 0;
	if (activeMenu) return activeMenu->id;
	UInt32 topMenu = 0, *mnStack = menuStack;
	while (*mnStack) topMenu = *mnStack++;
	if (topMenu != 1) return topMenu;
	return 0;
}
