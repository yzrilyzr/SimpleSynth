#pragma once
#include "imgui.h"
#include <string>
#include <vector>
struct NotificationMessage{
	std::string text;
	double showTime;
	ImVec4 color;
};

class NotificationManager{
	private:
	std::vector<NotificationMessage> messages;
	public:
	void AddNotification(const std::string & message, float duration, ImVec4 color);
	void Draw();
};