/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#pragma once

#include <obs-module.h>

#include <QString>

namespace scoreboard {

/** Строка интерфейса из data/locale — на языке OBS, по умолчанию по-русски. */
inline QString T(const char *key)
{
	return QString::fromUtf8(obs_module_text(key));
}

} // namespace scoreboard
