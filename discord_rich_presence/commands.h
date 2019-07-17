#pragma once
DEFINE_COMMAND_PLUGIN(SendRPCAction, , 0, 1, kParams_OneString);
DEFINE_COMMAND_PLUGIN(RemoveRPCAction, , 0, 0, NULL);

bool Cmd_SendRPCAction_Execute(COMMAND_ARGS) {
	*result = 0;
	char newStr[256] = { 0 };
	if (ExtractArgs(EXTRACT_ARGS, &newStr)) {
		strcpy(g_customString, newStr);
	}
	return true;
}
bool Cmd_RemoveRPCAction_Execute(COMMAND_ARGS) {
	strcpy(g_customString, "");
	return true;
}
void UpdateRPC() {
	g_startMenu = *(StartMenu**)0x11DAAC0;
	if (!g_startMenu || (g_startMenu && !(g_startMenu->flags & 0x80))) //if not in start menu, or in start menu but not 0x80 (main menu flag)
	{
		char stateStr[256] = "";
		char detailsStr[256] = "";
		BuildStateStr(stateStr);
		BuildDetailsStr(detailsStr);
		UpdateDiscordPresence(stateStr, detailsStr);
	}
	else if (bShowInMainMenu) {
		UpdateDiscordPresence("in Main Menu", "");
	}
}

__declspec(naked) void __stdcall GetCellName(const char* nam) {
	__asm {
		mov eax, 0x851B40
		jmp eax
	}
}
void BuildStateStr(char* stateStr) {
	char buffer[256] = "";
	char buffer2[256] = "";
	if (bShowName) strcat(buffer, GetFullName(g_thePlayer));
	if (bShowLevel) {
		if (strlen(buffer) > 0) strcat(buffer, " | ");
		sprintf(buffer2, "LVL: %.f", GetPlayerLevel());
		strcat(buffer, buffer2);
	}
	if (bShowHealth) {
		if (strlen(buffer) > 0) strcat(buffer, " | ");
		sprintf(buffer2, "HP: %.f%%", GetPlayerHealthPercentage());
		strcat(buffer, buffer2);
	}
	if (bShowCaps) {
		if (strlen(buffer) > 0) strcat(buffer, " | ");
		int capsCount = ThisStdCall(0x891D70, g_thePlayer);
		sprintf(buffer2, "%d caps", capsCount);
		strcat(buffer, buffer2);
	}
	strcpy(stateStr, buffer);
}

void BuildDetailsStr(char* detailsStr) {
	*detailsStr = '\0';
	if (g_interfaceManager->currentMode == 2) {
		int menuID = g_interfaceManager->GetTopVisibleMenuID();
		switch (menuID) {
		case 1012:
			if (bShowSleeping) strcat(detailsStr, "Sleeping");
			break;
		case 1013:
			if (bShowPaused) strcat(detailsStr, "Paused");
			break;
		case 1014:
			if (bShowLockpicking) strcat(detailsStr, "Picking a lock");
			break;
		case 1055:
			if (bShowHacking) strcat(detailsStr, "Hacking a terminal");
			break;
		case 1080:
			if (bShowGambling) strcat(detailsStr, "Playing slots");
			break;
		case 1081:
			if (bShowGambling) strcat(detailsStr, "Playing blackjack");
			break;
		case 1082:
			if (bShowGambling) strcat(detailsStr, "Playing roulette");
			break;
		case 1083:
			if (bShowGambling) strcat(detailsStr, "Playing caravan");
			break;
		case 1026:
			if (bShowReading) strcat(detailsStr, "Reading");
			break;
		case 1:
		case 1002:
		case 1003:
		case 1023:
		case 1035:
			if (bShowPipboy) strcat(detailsStr, "Browsing Pip-Boy");
			break;
		default:
			break;
		}
	}
	if (bAllowCustomActions && !strlen(detailsStr) && g_customString) {
		strcpy(detailsStr, g_customString);
	}
	if (bShowLocation) {
		char cellName[260];
		GetCellName(cellName);
		if (strlen(detailsStr) && strlen(cellName)) 
			strcat(detailsStr, " in ");
		strcat(detailsStr, cellName);
	}
}

double __cdecl GetPlayerHealthPercentage() {
	float baseHealth = g_thePlayer->avOwner.GetBaseActorValue(16);
	float health = g_thePlayer->avOwner.GetActorValue(16);
	return (health / baseHealth) * 100;
}

double __cdecl GetPlayerLevel() {
	return ThisStdCall(0x87F9F0, g_thePlayer);
}