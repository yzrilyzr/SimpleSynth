#include "ParamHelper.h"
#include "array/SampleProvider.h"
#include "dsp/DSP.h"
#include "array/Array.hpp"
#include "imnodes.h"
#include "interpolator/Interpolator.h"
#include "synth/source/AmplitudeSources.h"
#include "util/Convert.h"

using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_array;

ImU32 getPinColor(ParamReg & param){
	int type=param.type;
	// 振荡器 - 波形生成，常用蓝色系
	if(type == ParamType::OscSrc)
		return IM_COL32(130, 220, 130, 255); // 浅绿色
	// 音符处理器 - MIDI/音符处理，常用绿色系
	else if(type == ParamType::NoteSrc)
		return IM_COL32(90, 220, 90, 255); // 鲜绿色
	// 相位源 - 时间/相位控制，常用黄色系
	else if(type == ParamType::PhaseSrc)
		return IM_COL32(255, 200, 50, 255); // 金黄色
	// 插值器 - 平滑过渡，常用紫色系
	else if(type == ParamType::Interpolator)
		return IM_COL32(180, 100, 220, 255); // 紫色
	// DSP处理器 - 通用音频处理，常用橙色系
	else if(type == ParamType::DSP)
		return IM_COL32(255, 140, 60, 255); // 橙色
	// 采样提供器 - 音频采样，常用青色系
	else if(type == ParamType::SampleData)
		return IM_COL32(80, 200, 220, 255); // 青色
	// 默认颜色 - 未知类型
	return IM_COL32(180, 180, 180, 255); // 灰色
}
ImU32 getPinColor(u_sp<ClassRegister> paramRegPtr){
	if(paramRegPtr == nullptr) return IM_COL32(255, 0, 0, 255); // 红色表示无效/空指针

	// 振荡器 - 波形生成，常用蓝色系
	if(spdc<yzrilyzr_simplesynth::Osc>(paramRegPtr))
		return IM_COL32(130, 220, 130, 255);// 浅绿色

	// 音符处理器 - MIDI/音符处理，常用绿色系
	else if(spdc<yzrilyzr_simplesynth::NoteProcessor>(paramRegPtr))
		return IM_COL32(90, 220, 90, 255); // 鲜绿色

	// 相位源 - 时间/相位控制，常用黄色系
	else if(spdc<yzrilyzr_simplesynth::PhaseSrc>(paramRegPtr))
		return IM_COL32(255, 200, 50, 255); // 金黄色

	// 插值器 - 平滑过渡，常用紫色系
	else if(spdc<yzrilyzr_interpolator::Interpolator>(paramRegPtr))
		return IM_COL32(180, 100, 220, 255); // 紫色

	// DSP处理器 - 通用音频处理，常用橙色系
	else if(spdc<yzrilyzr_dsp::DSP>(paramRegPtr))
		return IM_COL32(255, 140, 60, 255); // 橙色

	// 采样提供器 - 音频采样，常用青色系
	else if(spdc<yzrilyzr_array::SampleProvider>(paramRegPtr))
		return IM_COL32(80, 200, 220, 255); // 青色

	// 默认颜色 - 未知类型
	return IM_COL32(180, 180, 180, 255); // 灰色
}

void renderObjectParamInput(CurrentProjectContext & ctx, ParamReg & param, const char * paramName){
	bool uiInputChange=false;
	int paramId=reinterpret_cast<int>(param.value);
	auto color=getPinColor(param);

	ImNodes::PushColorStyle(ImNodesCol_Pin, color);

	ImNodes::BeginInputAttribute(paramId, ImNodesPinShape_TriangleFilled);
	ImGui::Text(paramName);
	ImNodes::EndInputAttribute();

	ImNodes::PopColorStyle();
}

bool setParamValue(ParamReg & param, u_sp<ClassRegister> paramRegPtr){
	int type=param.type;
	if(type == ParamType::NoteSrc){
		return setVal<yzrilyzr_simplesynth::NoteProcessor>(param, paramRegPtr);
	}
	if(type == ParamType::PhaseSrc){
		return setVal<yzrilyzr_simplesynth::PhaseSrc>(param, paramRegPtr);
	}
	if(type == ParamType::Interpolator){
		return setVal<yzrilyzr_interpolator::Interpolator>(param, paramRegPtr);
	}
	if(type == ParamType::DSP){
		return setVal<yzrilyzr_dsp::DSP>(param, paramRegPtr);
	}
	if(type == ParamType::SampleData){
		return setVal<yzrilyzr_array::SampleProvider>(param, paramRegPtr);
	}
	return false;
}

bool renderOneParam(CurrentProjectContext & ctx, ParamReg & param){
	String guiName=param.name;
	if(param.aliasName != ""){
		guiName=param.aliasName + "(" + guiName + ")";
	}
	const char * cstrName=guiName.c_str(UTF8);
	bool localChange=false;
	switch(param.type){
		case ParamType::Bool:
			localChange=ImGui::Checkbox(cstrName, static_cast<bool *>(param.value));
			break;
		case ParamType::Double:
			ImGui::PushItemWidth(150.0f);
			localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_Double, param.value, param.valueMin, param.valueMax);
			ImGui::PopItemWidth();
			break;
		case ParamType::Float:
			ImGui::PushItemWidth(150.0f);
			localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_Float, param.value, param.valueMin, param.valueMax);
			ImGui::PopItemWidth();
			break;
		case ParamType::Freq:
			ImGui::PushItemWidth(150.0f);
			localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_Double, param.value, param.valueMin, param.valueMax, "%.3f Hz", ImGuiSliderFlags_Logarithmic);
			ImGui::PopItemWidth();
			break;
		case ParamType::Time:
			ImGui::PushItemWidth(150.0f);
			localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_Double, param.value, param.valueMin, param.valueMax, "%.3f s", ImGuiSliderFlags_Logarithmic);
			ImGui::PopItemWidth();
			break;
		case ParamType::TimeMs:
			ImGui::PushItemWidth(150.0f);
			localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_Double, param.value, param.valueMin, param.valueMax, "%.3f ms", ImGuiSliderFlags_Logarithmic);
			ImGui::PopItemWidth();
			break;
		case ParamType::Int:
			ImGui::PushItemWidth(150.0f);
			localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_S32, param.value, param.valueMin, param.valueMax);
			ImGui::PopItemWidth();
			break;
		case ParamType::UInt:
			ImGui::PushItemWidth(150.0f);
			localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_U32, param.value, param.valueMin, param.valueMax);
			ImGui::PopItemWidth();
			break;
		case ParamType::Long:
			ImGui::PushItemWidth(150.0f);
			localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_S64, param.value, param.valueMin, param.valueMax);
			ImGui::PopItemWidth();
			break;
		case ParamType::ULong:
			ImGui::PushItemWidth(150.0f);
			localChange=ImGui::SliderScalar(cstrName, ImGuiDataType_U64, param.value, param.valueMin, param.valueMax);
			ImGui::PopItemWidth();
			break;
		case ParamType::Size:
			ImGui::PushItemWidth(150.0f);
			localChange=ImGui::SliderScalar(cstrName, sizeof(u_index) == 8?ImGuiDataType_U64:ImGuiDataType_U32, param.value, param.valueMin, param.valueMax);
			ImGui::PopItemWidth();
			break;
		case ParamType::Gain:
			ImGui::PushItemWidth(150.0f);
			localChange=ImGui::SliderScalar(cstrName, sizeof(u_sample) == 8?ImGuiDataType_Double:ImGuiDataType_Float, param.value, param.valueMin, param.valueMax, "%.3f mul", ImGuiSliderFlags_Logarithmic);
			ImGui::PopItemWidth();
			break;
		case ParamType::Sample:
			ImGui::PushItemWidth(150.0f);
			localChange=ImGui::SliderScalar(cstrName, sizeof(u_sample) == 8?ImGuiDataType_Double:ImGuiDataType_Float, param.value, param.valueMin, param.valueMax, "%.3f");
			ImGui::PopItemWidth();
			break;
		case ParamType::Enum:
			ImGui::PushItemWidth(150.0f);
			localChange=ImGui::Combo(cstrName, static_cast<int *>(param.value), static_cast<const char **>(param.valueMin), *static_cast<int *>(param.valueMax));
			ImGui::PopItemWidth();
			break;
		case ParamType::Sub:
			ImGui::Text(cstrName);
			localChange=renderParams(ctx, static_cast<ClassRegister *>(param.value)->RegisteredParams);
			break;
		case ParamType::NoteSrc:
			renderObjectParamInput(ctx, param, cstrName);
			break;
		case ParamType::PhaseSrc:
			renderObjectParamInput(ctx, param, cstrName);
			break;
		case ParamType::OscSrc:
			renderObjectParamInput(ctx, param, cstrName);
			break;
		case ParamType::SampleData:
			renderObjectParamInput(ctx, param, cstrName);
			break;
		case ParamType::Interpolator:
			renderObjectParamInput(ctx, param, cstrName);
			break;
		case ParamType::DSP:
			renderObjectParamInput(ctx, param, cstrName);
			break;
		case ParamType::IntArray:
		{
			ImGui::PushItemWidth(150.0f);
			static char buf[10240];
			IntArray & arr=*static_cast<IntArray *>(param.value);
			if(arr != nullptr){
				for(int i=0;i < arr.length;i++){
					ImGui::PushItemWidth(500.0f);
					ImGui::PushID(&arr[i]);
					ImGui::SliderInt("v", &arr[i], 1, 20000, "%d", ImGuiSliderFlags_Logarithmic);
					ImGui::PopID();
					ImGui::PopItemWidth();
				}
				auto str=arr.toString();
				str=str.substring(1, str.length() - 1);
				strcpy(buf, str.c_str(UTF8));
			}
			if(ImGui::InputText(cstrName, buf, 10240, 0)){
				localChange=true;
				try{
					auto inputText=String(buf, UTF8).split(",");
					auto narr=IntArray(inputText.length);
					for(int i=0;i < narr.length;i++){
						narr[i]=parseInt(inputText[i].trim());
					}
					arr=narr;
				} catch(...){}
			}
			ImGui::PopItemWidth();
		}
		break;
		default:
			std::cout << "ImGui Type " << param.type << std::endl;
			break;
	}
	return localChange;
}
bool renderParams(CurrentProjectContext & ctx, std::vector<ParamReg> & paramReg){
	bool change=false;
	for(auto & param : paramReg){
		change=renderOneParam(ctx, param) || change;
	}
	return change;
}