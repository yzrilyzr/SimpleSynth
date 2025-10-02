#include "NotificationManager.h"
using namespace yzrilyzr_lang;
void  NotificationManager::AddNotification(const String & message, float duration=3.0f, ImVec4 color=ImVec4(0.0f, 1.0f, 0.0f, 1.0f)){
	messages.push_back(NotificationMessage{message, ImGui::GetTime() + duration, color});
}
void NotificationManager::Draw(){
	double currentTime=ImGui::GetTime();

	// 从后往前遍历，以便安全删除
	for(int64_t i=messages.size() - 1; i >= 0; i--){
		if(currentTime > messages[i].showTime){
			messages.erase(messages.begin() + i);
			continue;
		}

		// 计算消息的透明度（淡出效果）
		double alpha=std::min(1.0, messages[i].showTime - currentTime);

		// 设置窗口位置（右上角）
		double windowWidth=300.0;  // 消息窗口宽度
		ImGui::SetNextWindowPos(
			ImVec2(ImGui::GetIO().DisplaySize.x - windowWidth - 10,
				   10 + i * 40),  // 每条消息间隔40像素
			ImGuiCond_Always
		);
		ImGui::SetNextWindowSize(ImVec2(windowWidth, 0));

		// 设置窗口样式
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg,
							  ImVec4(0.1f, 0.1f, 0.1f, 0.8f * alpha));

						  // 创建无边框窗口
		ImGui::Begin(
			("##Notification" + std::to_string(i)).c_str(),
			nullptr,
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoNav
		);

		// 显示消息文本
		ImGui::PushStyleColor(ImGuiCol_Text,
							  ImVec4(messages[i].color.x, messages[i].color.y,
									 messages[i].color.z, messages[i].color.w * alpha));
		ImGui::TextWrapped("%s", messages[i].text.c_str(UTF8));
		ImGui::PopStyleColor();

		ImGui::End();

		// 恢复样式
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);
	}
}