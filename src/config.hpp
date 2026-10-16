/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#pragma once

#include <string>

namespace scoreboard {

/**
 * Настройки плагина: к какому серверу и полю он привязан.
 *
 * Хранятся в папке настроек OBS для плагина (`config.json`). Пропуск —
 * то же, что cookie пульта трансляции в браузере: с ним сервер узнаёт этот
 * OBS и даёт ему управлять графикой только своего поля.
 */
struct Connection {
	std::string server; // https://scoreboard.example.ru, без слеша на конце
	std::string token;  // пропуск пульта трансляции
	std::string cookie; // имя cookie пропуска (board_console)
	std::string slot;   // поле: A или B
	std::string name;   // как пульт назван на подготовке

	bool connected() const { return !server.empty() && !token.empty() && !slot.empty(); }
};

/** Текущие настройки. Безопасно из любого потока. */
Connection current();

/** Сохранить и запомнить. */
void store(const Connection &connection);

/** Забыть привязку, оставив адрес сервера. */
void forget();

/** Прочитать с диска при запуске. */
void load();

/** Адрес как его ввёл человек → `https://host` без слеша на конце. */
std::string normalizeServer(const std::string &input);

} // namespace scoreboard
