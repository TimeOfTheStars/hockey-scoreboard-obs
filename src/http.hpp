/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#pragma once

#include <functional>
#include <string>

namespace scoreboard::http {

struct Response {
	long status = 0;   // 0 — до сервера не достучались
	std::string body;  // тело ответа, обычно JSON
	std::string error; // текст ошибки сети, если status == 0
};

/**
 * Запрос к серверу табло. Синхронный — вызывать только из рабочего потока
 * (см. `runInBackground`), иначе OBS замрёт на время запроса.
 */
Response request(const std::string &method, const std::string &url, const std::string &body, const std::string &cookie);

/** Запустить задачу в фоновом потоке плагина: по одной, по очереди. */
void runInBackground(std::function<void()> task);

/** Подготовка libcurl и рабочего потока; остановка при выгрузке плагина. */
void start();
void stop();

} // namespace scoreboard::http
