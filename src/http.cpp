/*
hockey-scoreboard-obs
Copyright (C) 2026 TimeOfTheStars

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/

#include "http.hpp"

#include <plugin-support.h>
#include <obs-module.h>

#include <curl/curl.h>

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

namespace scoreboard::http {

namespace {

size_t collect(char *data, size_t size, size_t count, void *target)
{
	static_cast<std::string *>(target)->append(data, size * count);
	return size * count;
}

/**
 * Один рабочий поток на все запросы. Запросы короткие и редкие — нажатие
 * горячей клавиши, привязка, — а поток один, чтобы при выгрузке плагина
 * его можно было дождаться и не оставить висящих задач.
 */
struct Worker {
	std::mutex mutex;
	std::condition_variable wake;
	std::deque<std::function<void()>> queue;
	std::thread thread;
	bool stopping = false;

	void run()
	{
		for (;;) {
			std::function<void()> task;
			{
				std::unique_lock<std::mutex> lock(mutex);
				wake.wait(lock, [this] { return stopping || !queue.empty(); });
				if (stopping && queue.empty())
					return;
				task = std::move(queue.front());
				queue.pop_front();
			}
			task();
		}
	}
};

Worker *worker = nullptr;

} // namespace

Response request(const std::string &method, const std::string &url, const std::string &body, const std::string &cookie)
{
	Response response;
	CURL *curl = curl_easy_init();
	if (!curl) {
		response.error = "libcurl не запустился";
		return response;
	}

	struct curl_slist *headers = nullptr;
	headers = curl_slist_append(headers, "Accept: application/json");
	if (!body.empty())
		headers = curl_slist_append(headers, "Content-Type: application/json");

	char error[CURL_ERROR_SIZE] = {0};
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, collect);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	std::string agent = std::string("hockey-scoreboard-obs/") + PLUGIN_VERSION;
	curl_easy_setopt(curl, CURLOPT_USERAGENT, agent.c_str());
	if (!body.empty())
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
	if (!cookie.empty())
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookie.c_str());

	CURLcode code = curl_easy_perform(curl);
	if (code == CURLE_OK) {
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status);
	} else {
		response.error = error[0] ? error : curl_easy_strerror(code);
	}

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
	return response;
}

void runInBackground(std::function<void()> task)
{
	if (!worker)
		return;
	{
		std::lock_guard<std::mutex> lock(worker->mutex);
		if (worker->stopping)
			return;
		worker->queue.push_back(std::move(task));
	}
	worker->wake.notify_one();
}

void start()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
	worker = new Worker();
	worker->thread = std::thread([] { worker->run(); });
}

void stop()
{
	if (!worker)
		return;
	{
		std::lock_guard<std::mutex> lock(worker->mutex);
		worker->stopping = true;
	}
	worker->wake.notify_one();
	if (worker->thread.joinable())
		worker->thread.join();
	delete worker;
	worker = nullptr;
	curl_global_cleanup();
}

} // namespace scoreboard::http
