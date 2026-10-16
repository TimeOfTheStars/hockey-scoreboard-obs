/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include "connect-dialog.hpp"
#include "api.hpp"
#include "config.hpp"
#include "text.hpp"

#include <QDialogButtonBox>
#include <QFont>
#include <QFormLayout>
#include <QPointer>
#include <QRegularExpressionValidator>
#include <QVBoxLayout>

namespace scoreboard {

/** Сервер лиги — подставляется, если адрес ещё не вводили. */
static const char *DEFAULT_SERVER = "https://scoreboard.timeofthestars.ru";

ConnectDialog::ConnectDialog(QWidget *parent) : QDialog(parent)
{
	setWindowTitle(T("Connect.Title"));
	setMinimumWidth(440);

	auto *layout = new QVBoxLayout(this);
	auto *intro = new QLabel(T("Connect.Intro"), this);
	intro->setWordWrap(true);
	intro->setTextFormat(Qt::RichText);
	layout->addWidget(intro);

	auto *form = new QFormLayout();
	Connection saved = current();
	server = new QLineEdit(QString::fromStdString(saved.server.empty() ? DEFAULT_SERVER : saved.server), this);
	server->setPlaceholderText(DEFAULT_SERVER);
	form->addRow(T("Connect.Server"), server);

	code = new QLineEdit(this);
	code->setPlaceholderText("123456");
	code->setMaxLength(6);
	code->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d{0,6}"), code));
	QFont big = code->font();
	big.setPointSize(big.pointSize() + 8);
	big.setLetterSpacing(QFont::AbsoluteSpacing, 6);
	code->setFont(big);
	form->addRow(T("Connect.Code"), code);
	layout->addLayout(form);

	status = new QLabel(this);
	status->setWordWrap(true);
	layout->addWidget(status);

	auto *buttons = new QDialogButtonBox(this);
	submitButton = buttons->addButton(T("Connect.Submit"), QDialogButtonBox::AcceptRole);
	buttons->addButton(QDialogButtonBox::Cancel);
	connect(buttons, &QDialogButtonBox::accepted, this, &ConnectDialog::submit);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout->addWidget(buttons);

	// Шесть цифр набраны — подключаемся сразу, как на экране пульта.
	connect(code, &QLineEdit::textChanged, this, [this](const QString &text) {
		if (text.size() == 6)
			submit();
	});
	code->setFocus();
}

void ConnectDialog::setBusy(bool busy)
{
	submitButton->setEnabled(!busy);
	server->setEnabled(!busy);
	code->setEnabled(!busy);
}

void ConnectDialog::submit()
{
	if (!submitButton->isEnabled())
		return;
	std::string address = normalizeServer(server->text().toStdString());
	std::string digits = code->text().toStdString();
	if (address.empty() || digits.size() != 6) {
		status->setText(T("Connect.NeedBoth"));
		return;
	}

	setBusy(true);
	status->setStyleSheet("");
	status->setText(T("Connect.Busy"));
	QPointer<ConnectDialog> self(this);
	api::pair(address, digits, [self](api::PairResult result) {
		if (!self)
			return;
		if (!result.ok) {
			self->setBusy(false);
			self->status->setStyleSheet("color: #e5484d;");
			self->status->setText(QString::fromStdString(result.error));
			self->code->clear();
			self->code->setFocus();
			return;
		}
		store(result.connection);
		self->accept();
	});
}

} // namespace scoreboard
