/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace scoreboard {

/**
 * Подключить OBS к табло: адрес сервера и шестизначный код с экрана
 * подготовки («Пульты и OBS» → у поля «Подключить OBS»). Код одноразовый;
 * после подключения OBS помнит привязку сам.
 */
class ConnectDialog : public QDialog {
	Q_OBJECT

public:
	explicit ConnectDialog(QWidget *parent = nullptr);

private:
	void submit();
	void setBusy(bool busy);

	QLineEdit *server = nullptr;
	QLineEdit *code = nullptr;
	QLabel *status = nullptr;
	QPushButton *submitButton = nullptr;
};

} // namespace scoreboard
