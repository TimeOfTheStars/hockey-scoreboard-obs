/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#pragma once

namespace scoreboard {

class Dock;

/**
 * Меню «Табло» в главном окне OBS: подключить, добавить графику в сцену,
 * показать пульт, отключить. Стоит перед «Справкой» — там, где его ищут.
 */
void installMenu(Dock *dock);

/** Окно подключения — из меню и из кнопки в доке. */
void openConnectDialog(Dock *dock);

} // namespace scoreboard
