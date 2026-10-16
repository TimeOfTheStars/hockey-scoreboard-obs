/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include "menu.hpp"
#include "api.hpp"
#include "config.hpp"
#include "connect-dialog.hpp"
#include "dock.hpp"
#include "sources.hpp"
#include "text.hpp"

#include <obs-frontend-api.h>

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPointer>
#include <QUrl>

namespace scoreboard {

namespace {

QMainWindow *mainWindow()
{
	return static_cast<QMainWindow *>(obs_frontend_get_main_window());
}

QString host(const std::string &server)
{
	QString value = QUrl(QString::fromStdString(server)).host();
	return value.isEmpty() ? QString::fromStdString(server) : value;
}

/** Завести графику поля в текущей сцене и сказать человеку, что сделано. */
void addGraphics()
{
	api::setup([](api::SetupResult result) {
		QWidget *parent = mainWindow();
		if (!result.ok) {
			QMessageBox::warning(parent, T("Menu.Title"), QString::fromStdString(result.error));
			return;
		}
		sources::Outcome outcome = sources::addToCurrentScene(result.slot, result.overlay, result.broadcast);
		if (outcome.scene.empty()) {
			QMessageBox::warning(parent, T("Menu.Title"), T("Sources.NoScene"));
			return;
		}
		QString text = outcome.added > 0 ? T("Sources.Added") : T("Sources.AlreadyThere");
		text.replace("%scene%", QString::fromStdString(outcome.scene));
		text.replace("%field%", QString::fromStdString(result.slot));
		QMessageBox::information(parent, T("Menu.Title"), text);
	});
}

/** После переподключения адреса графики могли смениться — обновляем источники. */
void refreshSourceUrls()
{
	api::setup([](api::SetupResult result) {
		if (result.ok)
			sources::updateExisting(result.slot, result.overlay, result.broadcast);
	});
}

} // namespace

void openConnectDialog(Dock *dock)
{
	ConnectDialog dialog(mainWindow());
	if (dialog.exec() != QDialog::Accepted)
		return;
	dock->refresh();
	showDock();
	refreshSourceUrls();

	Connection connection = current();
	QString question = T("Connect.Done");
	question.replace("%name%", QString::fromStdString(connection.name));
	question.replace("%field%", QString::fromStdString(connection.slot));
	if (QMessageBox::question(mainWindow(), T("Menu.Title"), question) == QMessageBox::Yes)
		addGraphics();
}

void installMenu(Dock *dock)
{
	QMainWindow *main = mainWindow();
	if (!main)
		return;

	auto *menu = new QMenu(T("Menu.Title"), main);
	QAction *status = menu->addAction(QString());
	status->setEnabled(false);
	menu->addSeparator();
	QAction *connectAction = menu->addAction(T("Menu.Connect"));
	QAction *graphicsAction = menu->addAction(T("Menu.AddGraphics"));
	QAction *dockAction = menu->addAction(T("Menu.ShowDock"));
	menu->addSeparator();
	QAction *disconnectAction = menu->addAction(T("Menu.Disconnect"));

	QPointer<Dock> target(dock);
	QObject::connect(connectAction, &QAction::triggered, main, [target] {
		if (target)
			openConnectDialog(target);
	});
	QObject::connect(graphicsAction, &QAction::triggered, main, [] { addGraphics(); });
	QObject::connect(dockAction, &QAction::triggered, main, [] { showDock(); });
	QObject::connect(disconnectAction, &QAction::triggered, main, [target, main] {
		if (QMessageBox::question(main, T("Menu.Title"), T("Menu.DisconnectConfirm")) != QMessageBox::Yes)
			return;
		forget();
		if (target)
			target->refresh();
	});

	// Подписи — по текущему подключению, каждый раз при открытии меню.
	QObject::connect(menu, &QMenu::aboutToShow, main, [=] {
		Connection connection = current();
		bool connected = connection.connected();
		QString text = connected ? T("Menu.StatusConnected") : T("Menu.StatusDisconnected");
		text.replace("%name%", QString::fromStdString(connection.name));
		text.replace("%host%", host(connection.server));
		status->setText(text);
		connectAction->setText(connected ? T("Menu.Reconnect") : T("Menu.Connect"));
		graphicsAction->setEnabled(connected);
		disconnectAction->setEnabled(connected);
	});

	// Перед «Справкой», если она найдётся; иначе — в конец.
	QMenuBar *bar = main->menuBar();
	QAction *before = nullptr;
	for (QAction *action : bar->actions()) {
		if (action->menu() && action->menu()->objectName() == "menuBasic_MainMenu_Help") {
			before = action;
			break;
		}
	}
	if (before)
		bar->insertMenu(before, menu);
	else
		bar->addMenu(menu);
}

} // namespace scoreboard
