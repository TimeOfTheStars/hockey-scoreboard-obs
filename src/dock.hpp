/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#pragma once

#include <QLabel>
#include <QPushButton>
#include <QStackedLayout>
#include <QWidget>

#include <string>

struct QCef;
class QCefWidget;

namespace scoreboard {

/**
 * Док «Табло» — пульт трансляции поля прямо в окне OBS.
 *
 * Внутри — страница пульта трансляции с сервера, поэтому пульт обновляется
 * вместе с сервером, без переустановки плагина. Пока OBS не подключён,
 * вместо страницы — объяснение и кнопка «Подключить».
 */
class Dock : public QWidget {
	Q_OBJECT

public:
	explicit Dock(QWidget *parent = nullptr);

	/** obs-browser загружен — можно открывать страницу. */
	void browserReady(QCef *cef);
	/** Подключение изменилось — показать пульт или приглашение подключиться. */
	void refresh();
	/** OBS закрывается: браузер дока нужно закрыть до выгрузки obs-browser. */
	void shutdown();

signals:
	void connectRequested();

private:
	QStackedLayout *stack = nullptr;
	QWidget *placeholder = nullptr;
	QLabel *message = nullptr;
	QPushButton *connectButton = nullptr;
	QCef *cef = nullptr;
	QCefWidget *browser = nullptr;
	std::string loadedUrl;
};

/** Показать док, если его закрыли. */
void showDock();

} // namespace scoreboard
