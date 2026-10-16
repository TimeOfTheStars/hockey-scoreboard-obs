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

/**
 * Источники графики в OBS: плашка со счётом и полноэкранная графика поля.
 *
 * Плагин заводит их сам — браузерные источники 1920×1080 с прозрачным фоном
 * и правильными адресами, — чтобы режиссёру не пришлось копировать ссылки и
 * вспоминать размеры.
 */
namespace scoreboard::sources {

struct Outcome {
	std::string scene; // в какую сцену добавили
	int added = 0;     // сколько источников добавили в сцену
	int existing = 0;  // сколько уже были в ней
};

/**
 * Завести (или обновить адреса) источники поля и добавить в текущую сцену —
 * в режиме студии в сцену предпросмотра. Полноэкранная графика ложится
 * поверх плашки.
 */
Outcome addToCurrentScene(const std::string &slot, const std::string &overlay, const std::string &broadcast);

/** Только обновить адреса уже заведённых источников — после переподключения. */
void updateExisting(const std::string &slot, const std::string &overlay, const std::string &broadcast);

} // namespace scoreboard::sources
