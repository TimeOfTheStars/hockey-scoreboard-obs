/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include "hotkeys.hpp"
#include "api.hpp"

#include <obs-module.h>
#include <util/platform.h>

#include <string>
#include <vector>

namespace scoreboard::hotkeys {

namespace {

struct Hotkey {
	const char *name;        // ключ в файле настроек
	const char *description; // ключ строки в data/locale
	const char *action;      // действие на сервере
	obs_hotkey_id id = OBS_INVALID_HOTKEY_ID;
};

std::vector<Hotkey> hotkeys = {
	{"hockey_scoreboard.scorebug", "Hotkey.Scorebug", "scorebug-toggle"},
	{"hockey_scoreboard.lineups", "Hotkey.Lineups", "lineups-toggle"},
	{"hockey_scoreboard.final", "Hotkey.Final", "final-toggle"},
	{"hockey_scoreboard.clear", "Hotkey.Clear", "clear"},
	{"hockey_scoreboard.goal", "Hotkey.Goal", "goal-replay"},
};

std::string path()
{
	char *dir = obs_module_config_path("");
	if (dir) {
		os_mkdirs(dir);
		bfree(dir);
	}
	char *file = obs_module_config_path("hotkeys.json");
	std::string result = file ? file : "";
	bfree(file);
	return result;
}

void pressed(void *data, obs_hotkey_id, obs_hotkey_t *, bool down)
{
	// Срабатываем на нажатие, а не на отпускание: так же, как кнопки в доке.
	if (!down)
		return;
	api::action(static_cast<Hotkey *>(data)->action);
}

} // namespace

void registerAll()
{
	obs_data_t *saved = nullptr;
	std::string file = path();
	if (!file.empty())
		saved = obs_data_create_from_json_file_safe(file.c_str(), "bak");

	for (Hotkey &hotkey : hotkeys) {
		hotkey.id = obs_hotkey_register_frontend(hotkey.name, obs_module_text(hotkey.description), pressed,
							 &hotkey);
		if (saved) {
			obs_data_array_t *keys = obs_data_get_array(saved, hotkey.name);
			if (keys) {
				obs_hotkey_load(hotkey.id, keys);
				obs_data_array_release(keys);
			}
		}
	}
	obs_data_release(saved);
}

void save()
{
	std::string file = path();
	if (file.empty())
		return;
	obs_data_t *data = obs_data_create();
	for (const Hotkey &hotkey : hotkeys) {
		if (hotkey.id == OBS_INVALID_HOTKEY_ID)
			continue;
		obs_data_array_t *keys = obs_hotkey_save(hotkey.id);
		obs_data_set_array(data, hotkey.name, keys);
		obs_data_array_release(keys);
	}
	obs_data_save_json_safe(data, file.c_str(), "tmp", "bak");
	obs_data_release(data);
}

void unregisterAll()
{
	for (Hotkey &hotkey : hotkeys) {
		if (hotkey.id != OBS_INVALID_HOTKEY_ID)
			obs_hotkey_unregister(hotkey.id);
		hotkey.id = OBS_INVALID_HOTKEY_ID;
	}
}

} // namespace scoreboard::hotkeys
