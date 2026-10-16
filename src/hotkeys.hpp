/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#pragma once

/**
 * Горячие клавиши OBS для графики поля: плашка, составы, итог, убрать
 * графику, повтор титра гола. Назначаются в «Настройки → Горячие клавиши»,
 * раздел «Табло», и работают, даже когда док закрыт.
 */
namespace scoreboard::hotkeys {

void registerAll();
/** Назначенные клавиши — в файл настроек плагина: OBS сам их не сохраняет. */
void save();
void unregisterAll();

} // namespace scoreboard::hotkeys
