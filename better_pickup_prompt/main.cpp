#include "main.h"

bool NVSEPlugin_Query(const NVSEInterface *nvse, PluginInfo *info)
{
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "Better Pickup Prompt";
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
	WriteRelJump(0x7781B4, UInt32(WeightHook));
	WriteRelJump(0x778494, UInt32(WeightColorHook));
	WriteRelJump(0x77806D, UInt32(ValueHook));
	WriteRelJump(0x7787A1, UInt32(ShowMoreTypesHook));
	return true;
}