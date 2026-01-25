#include "ParamHelper.h"
#include "array/Array.hpp"
#include "array/SampleProvider.h"
#include "dsp/DSP.h"
#include "imnodes.h"
#include "interpolator/Interpolator.h"
#include "interpolator/Interpolator.h"
#include "lang/Boxing.h"
#include "synth/source/AmplitudeSources.h"

ImU32 getPinColor(yzrilyzr_util::ParamReg & param){
	int type=param.type;
	// 振荡器 - 波形生成，常用蓝色系
	if(type == yzrilyzr_util::ParamType::OscSrc)
		return IM_COL32(130, 220, 130, 255); // 浅绿色
	// 音符处理器 - MIDI/音符处理，常用绿色系
	else if(type == yzrilyzr_util::ParamType::NoteSrc)
		return IM_COL32(90, 220, 90, 255); // 鲜绿色
	// 相位源 - 时间/相位控制，常用黄色系
	else if(type == yzrilyzr_util::ParamType::PhaseSrc)
		return IM_COL32(255, 200, 50, 255); // 金黄色
	// 插值器 - 平滑过渡，常用紫色系
	else if(type == yzrilyzr_util::ParamType::Interpolator)
		return IM_COL32(180, 100, 220, 255); // 紫色
	// DSP处理器 - 通用音频处理，常用橙色系
	else if(type == yzrilyzr_util::ParamType::DSP)
		return IM_COL32(255, 140, 60, 255); // 橙色
	// 采样提供器 - 音频采样，常用青色系
	else if(type == yzrilyzr_util::ParamType::SampleData)
		return IM_COL32(80, 200, 220, 255); // 青色
	// 默认颜色 - 未知类型
	return IM_COL32(180, 180, 180, 255); // 灰色
}
ImU32 getPinColor(u_sp<yzrilyzr_util::ParamRegister> paramRegPtr){
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

void renderObjectParamInput(CurrentProjectContext & ctx, yzrilyzr_util::ParamReg & param, const char * paramName){
	bool uiInputChange=false;
	int paramId=reinterpret_cast<int>(param.value);
	auto color=getPinColor(param);

	ImNodes::PushColorStyle(ImNodesCol_Pin, color);

	ImNodes::BeginInputAttribute(paramId, ImNodesPinShape_TriangleFilled);
	ImGui::Text(paramName);
	ImNodes::EndInputAttribute();

	ImNodes::PopColorStyle();
}

bool setParamValue(yzrilyzr_util::ParamReg & param, u_sp<yzrilyzr_util::ParamRegister> paramRegPtr){
	int type=param.type;
	if(type == yzrilyzr_util::ParamType::NoteSrc){
		return setVal<yzrilyzr_simplesynth::NoteProcessor>(param, paramRegPtr);
	}
	if(type == yzrilyzr_util::ParamType::PhaseSrc){
		return setVal<yzrilyzr_simplesynth::PhaseSrc>(param, paramRegPtr);
	}
	if(type == yzrilyzr_util::ParamType::Interpolator){
		return setVal<yzrilyzr_interpolator::Interpolator>(param, paramRegPtr);
	}
	if(type == yzrilyzr_util::ParamType::DSP){
		return setVal<yzrilyzr_dsp::DSP>(param, paramRegPtr);
	}
	if(type == yzrilyzr_util::ParamType::SampleData){
		return setVal<yzrilyzr_array::SampleProvider>(param, paramRegPtr);
	}
	return false;
}
