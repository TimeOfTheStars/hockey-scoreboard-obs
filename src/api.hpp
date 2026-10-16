/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#pragma once

#include "config.hpp"

#include <functional>
#include <string>

/**
 * Запросы к серверу табло.
 *
 * Все — в фоновом потоке, ответ приходит в основной поток Qt: из обработчика
 * можно сразу трогать окна и источники OBS.
 */
namespace scoreboard::api {

struct PairResult {
	bool ok = false;
	std::string error;
	Connection connection;
};

/** Привязать OBS кодом с экрана подготовки («Подключить OBS» у поля). */
void pair(const std::string &server, const std::string &code, std::function<void(PairResult)> done);

struct SetupResult {
	bool ok = false;
	bool unauthorized = false; // пульт отключили на подготовке
	std::string error;
	std::string slot;
	std::string overlay;   // полный адрес плашки
	std::string broadcast; // полный адрес полноэкранной графики
};

/** Адреса графики своего поля — для источников OBS. */
void setup(std::function<void(SetupResult)> done);

/**
 * Действие горячей клавиши: `scorebug-toggle`, `lineups-toggle`,
 * `final-toggle`, `clear`, `goal-replay`. Ошибки — в журнал OBS.
 */
void action(const std::string &name);

/** Адрес дока: пульт трансляции с пропуском после `#`, на сервер он не уходит. */
std::string dockUrl(const Connection &connection);

} // namespace scoreboard::api
