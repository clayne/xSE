#pragma once
DEFINE_COMMAND_PLUGIN(IsRPCInitialized, , 0, 0, NULL);
DEFINE_COMMAND_PLUGIN(InitRPC, , 0, 0, NULL);
DEFINE_COMMAND_PLUGIN(UpdateRPC, , 0, 4, kParams_TwoInts_OneFloat_OneInt);

void __stdcall GetCellName(const char* nam);
void BuildStateStr(int caps, int level, float health, char* stateStr);
void BuildDetailsStr(int menumode, UInt32 id, char* detailsStr);

bool Cmd_IsRPCInitialized_Execute(COMMAND_ARGS) {
	*result = bInitialized;
	return true;
}

bool Cmd_InitRPC_Execute(COMMAND_ARGS) {
	*result = 0;
	DiscordInit();
	*result = bInitialized;
	return true;
}

bool Cmd_UpdateRPC_Execute(COMMAND_ARGS) {
	*result = 0;
	int caps = -1;
	float health = -1;
	int level = -1;
	int menumode = 0;
	char stateStr[256] = "";
	char detailsStr[256] = "";
	if (ExtractArgs(EXTRACT_ARGS, &caps, &level, &health, &menumode) && bInitialized) {
		
		BuildStateStr(caps, level, health, stateStr);
		if (bShowLocation) {
			if (!locationNameID)
				locationNameID = StrIfc->CreateString("Mojave Wasteland", scriptData);
			const char* str = StrIfc->GetString(locationNameID);
			GetCellName(str);
		}
		BuildDetailsStr(menumode, locationNameID, detailsStr);
		UpdateDiscordPresence(stateStr, detailsStr);
	}
	return true;
}

__declspec(naked) void __stdcall GetCellName(const char* nam) {
	static const UInt32 getName = 0x851B40;
	__asm {
		jmp getName
	}
}
void BuildStateStr(int caps, int level, float health, char* stateStr) {
	char buffer[256] = "";
	char buffer2[256] = "";
	if (bShowName) strcat(buffer, GetFullName(g_thePlayer));
	if (bShowLevel) {
		if (strlen(buffer) > 0) strcat(buffer, " | ");
		sprintf(buffer2, "LVL: %d", level);
		strcat(buffer, buffer2);
	}
	if (bShowHealth) {
		if (strlen(buffer) > 0) strcat(buffer, " | ");
		sprintf(buffer2, "HP: %.f%%", health * 100);
		strcat(buffer, buffer2);
	}
	if (bShowCaps) {
		if (strlen(buffer) > 0) strcat(buffer, " | ");
		sprintf(buffer2, "%d caps", caps);
		strcat(buffer, buffer2);
	}
	strcpy(stateStr, buffer);
}
void BuildDetailsStr(int menumode, UInt32 id, char* detailsStr) {
	char buffer[256] = "";
	if (menumode) menumode = g_interfaceManager->GetTopVisibleMenuID();
	switch (menumode) {
		case 0:
			break;
		case 1012:
			if (bShowSleeping) strcat(buffer, "Sleeping");
			break;
		case 1013:
			if (bShowPaused) strcat(buffer, "Paused");
			break;
		case 1014:
			if (bShowLockpicking) strcat(buffer, "Picking a lock");
			break;
		case 1055:
			if (bShowHacking) strcat(buffer, "Hacking a terminal");
			break;
		case 1080:
			if (bShowGambling) strcat(buffer, "Playing slots");
			break;
		case 1081:
			if (bShowGambling) strcat(buffer, "Playing blackjack");
			break;
		case 1082:
			if (bShowGambling) strcat(buffer, "Playing roulette");
			break;
		case 1083:
			if (bShowGambling) strcat(buffer, "Playing caravan");
			break;
		case 1:
		case 1002:
		case 1003:
		case 1023:
		case 1035:
			if (bShowPipboy) strcat(buffer, "Browsing Pip-Boy");
			break;
		default:
			break;
	}
	if (id) {
		if (strlen(buffer) > 0) strcat(buffer, " in ");
		strcat(buffer, StrIfc->GetString(id));
	}
	strcpy(detailsStr, buffer);
}