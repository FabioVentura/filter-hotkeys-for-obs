/*
 * OBS Filter Hotkeys
 * Copyright (C) 2026 Fabio Ventura
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 or later.
 */

#include <obs-module.h>
#include <plugin-support.h>

#include "filter-hotkey.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{
	obs_register_source(&filter_hotkey_controller);
	obs_log(LOG_INFO, "Filter Hotkeys loaded successfully (version %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "Filter Hotkeys unloaded");
}
