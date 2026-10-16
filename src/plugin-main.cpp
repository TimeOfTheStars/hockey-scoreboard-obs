/*
hockey-scoreboard-obs — пульт трансляции хоккейного табло в OBS
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include "browser-panel.hpp"
#include "config.hpp"
#include "dock.hpp"
#include "hotkeys.hpp"
#include "http.hpp"
#include "menu.hpp"

#include <obs-frontend-api.h>
#include <obs-module.h>
#include <plugin-support.h>

#include <QMainWindow>
#include <QPointer>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "ru-RU")

namespace {

QPointer<scoreboard::Dock> dock;

/**
 * Браузер дока поднимается, когда OBS загрузился целиком: obs-browser
 * грузится после нас (модули идут по алфавиту), и раньше панели браузера
 * просто нет.
 */
void onFrontendEvent(enum obs_frontend_event event, void *)
{
	switch (event) {
	case OBS_FRONTEND_EVENT_FINISHED_LOADING: {
		QCef *cef = obs_browser_init_panel();
		if (cef) {
			cef->init_browser();
			cef->wait_for_browser_init();
		} else {
			obs_log(LOG_WARNING, "obs-browser недоступен — док покажет подсказку вместо пульта");
		}
		if (dock)
			dock->browserReady(cef);
		break;
	}
	case OBS_FRONTEND_EVENT_EXIT:
		scoreboard::hotkeys::save();
		if (dock)
			dock->shutdown();
		break;
	default:
		break;
	}
}

} // namespace

const char *obs_module_name(void)
{
	return obs_module_text("Plugin.Name");
}

const char *obs_module_description(void)
{
	return obs_module_text("Plugin.Description");
}

bool obs_module_load(void)
{
	scoreboard::load();
	scoreboard::http::start();
	scoreboard::hotkeys::registerAll();

	auto *main = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	dock = new scoreboard::Dock(main);
	QObject::connect(dock, &scoreboard::Dock::connectRequested, dock, [] {
		if (dock)
			scoreboard::openConnectDialog(dock);
	});
	if (!obs_frontend_add_dock_by_id("hockey-scoreboard", obs_module_text("Dock.Title"), dock))
		obs_log(LOG_WARNING, "не удалось добавить док «Табло»");
	scoreboard::installMenu(dock);

	obs_frontend_add_event_callback(onFrontendEvent, nullptr);
	obs_log(LOG_INFO, "плагин загружен (версия %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_frontend_remove_event_callback(onFrontendEvent, nullptr);
	scoreboard::hotkeys::unregisterAll();
	scoreboard::http::stop();
	obs_log(LOG_INFO, "плагин выгружен");
}
