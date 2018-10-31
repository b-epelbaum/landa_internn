#pragma once

namespace LandaJune
{
	#define REG_COMPANY_NAME	"Landa Corp"
	#define REG_ROOT_KEY		"June QCS"
	#define DEF_RECIPE_FOLDER	"/" REG_COMPANY_NAME "/QCS Configuration"

	#define CLIENT_ROOT_KEY			"UIClient"
	#define CLIENT_KEY_LAST_RECIPE	CLIENT_ROOT_KEY"/lastConfigFile"

	#define ROITOOLS_ROOT_KEY				"ROITools"
	#define ROITOOLS_KEY_LAST_REG_FILE		ROITOOLS_ROOT_KEY"/lastRegFilesFolder"
	#define ROITOOLS_KEY_LAST_WAVE_FILE		ROITOOLS_ROOT_KEY"/lastWaveFilesFolder"
	#define ROITOOLS_KEY_LAST_FULL_FILE		ROITOOLS_ROOT_KEY"/lastFullFilesFolder"
}

