/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include "sources.hpp"

#include <obs-frontend-api.h>
#include <obs-module.h>

namespace scoreboard::sources {

namespace {

std::string overlayName(const std::string &slot)
{
	return std::string(obs_module_text("Source.Overlay")) + " " + slot;
}

std::string broadcastName(const std::string &slot)
{
	return std::string(obs_module_text("Source.Broadcast")) + " " + slot;
}

/** Браузерный источник с нужным адресом: заводим или обновляем существующий. */
obs_source_t *ensureSource(const std::string &name, const std::string &url)
{
	obs_source_t *source = obs_get_source_by_name(name.c_str());
	if (source) {
		obs_data_t *settings = obs_source_get_settings(source);
		if (std::string(obs_data_get_string(settings, "url")) != url) {
			obs_data_set_string(settings, "url", url.c_str());
			obs_source_update(source, settings);
		}
		obs_data_release(settings);
		return source;
	}

	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "url", url.c_str());
	obs_data_set_bool(settings, "is_local_file", false);
	// Кадр графики — ровно Full HD: плашка и сцены свёрстаны под него.
	obs_data_set_int(settings, "width", 1920);
	obs_data_set_int(settings, "height", 1080);
	// Графика живёт постоянно: если выключать её вне эфира, титр гола,
	// пришедший в этот момент, пропал бы.
	obs_data_set_bool(settings, "shutdown", false);
	obs_data_set_bool(settings, "restart_when_active", false);
	source = obs_source_create("browser_source", name.c_str(), settings, nullptr);
	obs_data_release(settings);
	return source;
}

obs_source_t *targetScene()
{
	if (obs_frontend_preview_program_mode_active())
		return obs_frontend_get_current_preview_scene();
	return obs_frontend_get_current_scene();
}

} // namespace

Outcome addToCurrentScene(const std::string &slot, const std::string &overlay, const std::string &broadcast)
{
	Outcome outcome;
	obs_source_t *sceneSource = targetScene();
	obs_scene_t *scene = sceneSource ? obs_scene_from_source(sceneSource) : nullptr;
	if (!scene) {
		obs_source_release(sceneSource);
		return outcome;
	}
	outcome.scene = obs_source_get_name(sceneSource);

	// Сначала плашка, потом полноэкранная графика: добавленное позже ложится
	// выше, а составы и итог должны закрывать плашку, а не наоборот.
	const std::pair<std::string, std::string> wanted[] = {
		{overlayName(slot), overlay},
		{broadcastName(slot), broadcast},
	};
	for (const auto &[name, url] : wanted) {
		obs_source_t *source = ensureSource(name, url);
		if (!source)
			continue;
		if (obs_scene_find_source(scene, name.c_str())) {
			outcome.existing += 1;
		} else {
			obs_scene_add(scene, source);
			outcome.added += 1;
		}
		obs_source_release(source);
	}

	obs_source_release(sceneSource);
	return outcome;
}

void updateExisting(const std::string &slot, const std::string &overlay, const std::string &broadcast)
{
	const std::pair<std::string, std::string> wanted[] = {
		{overlayName(slot), overlay},
		{broadcastName(slot), broadcast},
	};
	for (const auto &[name, url] : wanted) {
		obs_source_t *existing = obs_get_source_by_name(name.c_str());
		if (!existing)
			continue;
		obs_source_release(existing);
		obs_source_release(ensureSource(name, url));
	}
}

} // namespace scoreboard::sources
