/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include "api.hpp"
#include "http.hpp"

#include <plugin-support.h>
#include <obs-module.h>

#include <QApplication>
#include <QMetaObject>

namespace scoreboard::api {

namespace {

/** Ответ — в основной поток Qt. */
template<typename Result> void deliver(std::function<void(Result)> done, Result result)
{
	QMetaObject::invokeMethod(
		qApp, [done = std::move(done), result = std::move(result)]() { done(result); }, Qt::QueuedConnection);
}

std::string field(obs_data_t *data, const char *key)
{
	const char *value = data ? obs_data_get_string(data, key) : nullptr;
	return value ? value : "";
}

/** Текст ошибки: сервер кладёт его в `detail` по-русски, для человека. */
std::string errorText(const http::Response &response)
{
	if (response.status == 0)
		return "Нет связи с сервером: " + response.error;
	obs_data_t *data = obs_data_create_from_json(response.body.c_str());
	std::string detail = field(data, "detail");
	obs_data_release(data);
	if (!detail.empty())
		return detail;
	return "Сервер ответил " + std::to_string(response.status);
}

std::string jsonString(const std::string &value)
{
	std::string escaped = "\"";
	for (char c : value) {
		if (c == '"' || c == '\\')
			escaped += '\\';
		if (static_cast<unsigned char>(c) >= 0x20)
			escaped += c;
	}
	return escaped + "\"";
}

std::string cookieOf(const Connection &connection)
{
	return connection.cookie + "=" + connection.token;
}

} // namespace

void pair(const std::string &server, const std::string &code, std::function<void(PairResult)> done)
{
	http::runInBackground([server, code, done = std::move(done)]() mutable {
		PairResult result;
		// Роль называем явно: код пульта поля не должен превратить OBS в
		// пульт, который ведёт счёт. Сервер такой код отклонит и не сожжёт.
		std::string body = "{\"code\":" + jsonString(code) + ",\"returnToken\":true,\"role\":\"broadcast\"}";
		http::Response response = http::request("POST", server + "/api/console/pair", body, "");
		if (response.status != 200) {
			result.error = errorText(response);
			deliver(std::move(done), result);
			return;
		}
		obs_data_t *data = obs_data_create_from_json(response.body.c_str());
		obs_data_t *device = data ? obs_data_get_obj(data, "console") : nullptr;
		result.connection.server = server;
		result.connection.token = field(data, "token");
		result.connection.cookie = field(data, "cookie");
		result.connection.slot = field(device, "slot");
		result.connection.name = field(device, "name");
		obs_data_release(device);
		obs_data_release(data);
		if (result.connection.cookie.empty())
			result.connection.cookie = "board_console";
		result.ok = result.connection.connected();
		if (!result.ok)
			result.error = "Сервер не выдал пропуск — обновите сервер табло";
		deliver(std::move(done), result);
	});
}

void setup(std::function<void(SetupResult)> done)
{
	Connection connection = current();
	http::runInBackground([connection, done = std::move(done)]() mutable {
		SetupResult result;
		if (!connection.connected()) {
			result.error = "OBS не подключён к табло";
			deliver(std::move(done), result);
			return;
		}
		http::Response response =
			http::request("GET", connection.server + "/api/obs/setup", "", cookieOf(connection));
		if (response.status != 200) {
			result.unauthorized = response.status == 401;
			result.error = errorText(response);
			deliver(std::move(done), result);
			return;
		}
		obs_data_t *data = obs_data_create_from_json(response.body.c_str());
		obs_data_t *sources = data ? obs_data_get_obj(data, "sources") : nullptr;
		result.slot = field(data, "slot");
		result.overlay = connection.server + field(sources, "overlay");
		result.broadcast = connection.server + field(sources, "broadcast");
		obs_data_release(sources);
		obs_data_release(data);
		result.ok = !result.slot.empty();
		deliver(std::move(done), result);
	});
}

void action(const std::string &name)
{
	Connection connection = current();
	if (!connection.connected()) {
		obs_log(LOG_WARNING, "горячая клавиша «%s»: OBS не подключён к табло", name.c_str());
		return;
	}
	http::runInBackground([connection, name]() {
		std::string body = "{\"action\":" + jsonString(name) + "}";
		http::Response response =
			http::request("POST", connection.server + "/api/obs/action", body, cookieOf(connection));
		if (response.status != 200)
			obs_log(LOG_WARNING, "горячая клавиша «%s»: %s", name.c_str(), errorText(response).c_str());
	});
}

std::string dockUrl(const Connection &connection)
{
	return connection.server + "/dock?field=" + connection.slot + "#console=" + connection.token;
}

} // namespace scoreboard::api
