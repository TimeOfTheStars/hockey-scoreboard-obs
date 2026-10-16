/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include "dock.hpp"
#include "api.hpp"
#include "browser-panel.hpp"
#include "config.hpp"
#include "text.hpp"

#include <obs-frontend-api.h>

#include <QDockWidget>
#include <QMainWindow>
#include <QVBoxLayout>

namespace scoreboard {

Dock::Dock(QWidget *parent) : QWidget(parent)
{
	setMinimumWidth(260);
	stack = new QStackedLayout(this);
	stack->setContentsMargins(0, 0, 0, 0);

	placeholder = new QWidget(this);
	auto *layout = new QVBoxLayout(placeholder);
	layout->setContentsMargins(16, 16, 16, 16);
	layout->setSpacing(12);

	message = new QLabel(placeholder);
	message->setWordWrap(true);
	message->setTextFormat(Qt::RichText);
	layout->addWidget(message);

	connectButton = new QPushButton(T("Dock.Connect"), placeholder);
	connectButton->setMinimumHeight(36);
	connect(connectButton, &QPushButton::clicked, this, &Dock::connectRequested);
	layout->addWidget(connectButton);
	layout->addStretch(1);

	stack->addWidget(placeholder);
	refresh();
}

void Dock::browserReady(QCef *readyCef)
{
	cef = readyCef;
	refresh();
}

void Dock::refresh()
{
	Connection connection = current();
	if (!connection.connected()) {
		message->setText(T("Dock.NotConnected"));
		connectButton->setVisible(true);
		stack->setCurrentWidget(placeholder);
		if (browser && !loadedUrl.empty()) {
			// Отключённый OBS не должен держать открытым пульт со старым пропуском.
			browser->setURL("about:blank");
			loadedUrl.clear();
		}
		return;
	}
	if (!cef) {
		message->setText(T("Dock.NoBrowser"));
		connectButton->setVisible(false);
		stack->setCurrentWidget(placeholder);
		return;
	}

	std::string url = api::dockUrl(connection);
	if (!browser) {
		browser = cef->create_widget(this, url, nullptr);
		stack->addWidget(browser);
	} else if (url != loadedUrl) {
		browser->setURL(url);
	}
	loadedUrl = url;
	stack->setCurrentWidget(browser);
}

void Dock::shutdown()
{
	if (browser) {
		browser->closeBrowser();
		browser = nullptr;
	}
}

void showDock()
{
	auto *main = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	if (!main)
		return;
	// OBS заворачивает наш виджет в QDockWidget с именем, равным id дока.
	if (auto *dock = main->findChild<QDockWidget *>("hockey-scoreboard")) {
		dock->setVisible(true);
		dock->raise();
	}
}

} // namespace scoreboard
