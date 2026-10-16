/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include "config.hpp"

#include <obs-module.h>
#include <util/platform.h>

#include <algorithm>
#include <cctype>
#include <mutex>

namespace scoreboard {

namespace {

std::mutex mutex;
Connection connection;

std::string configPath()
{
	char *dir = obs_module_config_path("");
	if (dir) {
		os_mkdirs(dir);
		bfree(dir);
	}
	char *path = obs_module_config_path("config.json");
	std::string result = path ? path : "";
	bfree(path);
	return result;
}

std::string getString(obs_data_t *data, const char *key)
{
	const char *value = obs_data_get_string(data, key);
	return value ? value : "";
}

} // namespace

Connection current()
{
	std::lock_guard<std::mutex> lock(mutex);
	return connection;
}

void store(const Connection &next)
{
	{
		std::lock_guard<std::mutex> lock(mutex);
		connection = next;
	}
	std::string path = configPath();
	if (path.empty())
		return;
	obs_data_t *data = obs_data_create();
	obs_data_set_string(data, "server", next.server.c_str());
	obs_data_set_string(data, "token", next.token.c_str());
	obs_data_set_string(data, "cookie", next.cookie.c_str());
	obs_data_set_string(data, "slot", next.slot.c_str());
	obs_data_set_string(data, "name", next.name.c_str());
	// Пишем через временный файл: оборванная запись не должна стереть привязку.
	obs_data_save_json_safe(data, path.c_str(), "tmp", "bak");
	obs_data_release(data);
}

void forget()
{
	Connection next = current();
	next.token.clear();
	next.slot.clear();
	next.name.clear();
	store(next);
}

void load()
{
	std::string path = configPath();
	if (path.empty())
		return;
	obs_data_t *data = obs_data_create_from_json_file_safe(path.c_str(), "bak");
	if (!data)
		return;
	Connection loaded;
	loaded.server = getString(data, "server");
	loaded.token = getString(data, "token");
	loaded.cookie = getString(data, "cookie");
	loaded.slot = getString(data, "slot");
	loaded.name = getString(data, "name");
	if (loaded.cookie.empty())
		loaded.cookie = "board_console";
	obs_data_release(data);
	std::lock_guard<std::mutex> lock(mutex);
	connection = loaded;
}

std::string normalizeServer(const std::string &input)
{
	std::string value = input;
	auto notSpace = [](unsigned char c) {
		return !std::isspace(c);
	};
	value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
	value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
	while (!value.empty() && value.back() == '/')
		value.pop_back();
	if (!value.empty() && value.find("://") == std::string::npos)
		value = "https://" + value;
	return value;
}

} // namespace scoreboard
