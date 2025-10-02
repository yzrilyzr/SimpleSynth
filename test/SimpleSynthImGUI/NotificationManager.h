#pragma once
#include "imgui.h"
#include <string>
#include <vector>
#include "lang/String.h"
struct NotificationMessage{
	yzrilyzr_lang::String text;
	double showTime;
	ImVec4 color;
};

class NotificationManager{
	private:
	std::vector<NotificationMessage> messages;
	public:
	void AddNotification(const yzrilyzr_lang::String & message, float duration, ImVec4 color);
	void Draw();
};