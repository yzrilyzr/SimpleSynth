#pragma once
#include "MenuRegister.hpp"
#include "imgui.h"
#include <memory>
#include "nlohmann/json.hpp"
#include <string>
namespace yzrilyzr_util{
	class ParamRegister;
	struct ParamReg;
}
namespace yzrilyzr_lang{
	class Object;
}
class CurrentProjectContext;
using json=nlohmann::json;

EBCLASS(ProjectObject){
	public:
	yzrilyzr_lang::String category;
	yzrilyzr_lang::String name;
	yzrilyzr_lang::String showName;
	json fromJSON=nullptr;
	std::shared_ptr<yzrilyzr_util::ParamRegister> paramRegPtr=nullptr;
	bool showWindow=true;
	bool isSelected=false;
	bool loadStoredData=false;
	ImVec2 windowPos;
	ImVec2 windowSize;
	ImVec2 leftButtonCenter;
	ImVec2 rightButtonCenter;
	MenuRegister::RenderFunc renderFunc;
	bool enableOriginalRender=true;
	std::unordered_map<yzrilyzr_lang::String, std::shared_ptr<yzrilyzr_lang::Object>> storeData;
	json to_json() const;
	void from_json(const json & j);
	void renderWindow(CurrentProjectContext & ctx);

	static void paramToJSON(json & a, const yzrilyzr_util::ParamRegister & paramReg);
	static void JSONToParam(const json & a, yzrilyzr_util::ParamRegister & paramReg);
	static bool renderParams(CurrentProjectContext & ctx, std::vector<yzrilyzr_util::ParamReg> &paramReg);

	template<typename T>
	static bool renderProjectObjectParam(CurrentProjectContext & ctx, yzrilyzr_util::ParamReg & param, const char * paramName, std::vector<const char *> payloadType){
		bool uiInputChange=false;
		std::shared_ptr<T> * paramValue=static_cast<std::shared_ptr<T> *>(param.value);
		bool currentHighLightType=false;
		if(ctx.dragPayloadType){
			for(auto type : payloadType){
				if(strcmp(ctx.dragPayloadType, type) == 0){
					currentHighLightType=true;
					break;
				}
			}
		}
		bool paramValueIsNull=*paramValue == nullptr;
		bool changeColor=currentHighLightType || paramValueIsNull;
		if(changeColor){
			ImVec4 color;
			if(paramValueIsNull)color=ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
			if(currentHighLightType)color=ImVec4(0.8f, 0.8f, 0.2f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Button, color);
		}
		ImGui::Button(paramName);
		if(changeColor)ImGui::PopStyleColor();
		ImVec2 button2Min=ImGui::GetItemRectMin();
		ImVec2 button2Max=ImGui::GetItemRectMax();
		ctx.paramUIBindings.push_back(ParamUIBinding{param.value, ImVec2((button2Min.x + button2Max.x) * 0.5f, (button2Min.y + button2Max.y) * 0.5f)});
		if(ImGui::BeginDragDropTarget()){
			for(auto type : payloadType){
				if(const ImGuiPayload * payload=ImGui::AcceptDragDropPayload(type)){
					IM_ASSERT(payload->DataSize == sizeof(ProjectObject));
					ProjectObject * draggingObject=(ProjectObject *)(payload->Data);
					*paramValue=std::dynamic_pointer_cast<T>(draggingObject->paramRegPtr);//目标赋值
					uiInputChange=true;
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		ImGui::PushID(reinterpret_cast<int>(&param));
		if(ImGui::Button("X")){
			*paramValue=nullptr;
			uiInputChange=true;
		}
		ImGui::PopID();
		return uiInputChange;
	}
};
