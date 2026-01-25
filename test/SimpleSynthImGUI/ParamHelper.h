#pragma once
#include "ProjectObject.h"
#include "SimpleSynthProject.h"

ImU32 getPinColor(yzrilyzr_util::ParamReg & param);
ImU32 getPinColor(u_sp<yzrilyzr_util::ParamRegister> paramRegPtr);
bool setParamValue(yzrilyzr_util::ParamReg & param, u_sp<yzrilyzr_util::ParamRegister> paramRegPtr);
void renderObjectParamInput(CurrentProjectContext & ctx, yzrilyzr_util::ParamReg & param, const char * paramName);

template<typename T>
static bool setVal(yzrilyzr_util::ParamReg & param, u_sp<yzrilyzr_util::ParamRegister> paramRegPtr){
	u_sp<T> * val=static_cast<u_sp<T>*>(param.value);
	auto ptr=spdc<T>(paramRegPtr);
	if(ptr || paramRegPtr == nullptr){
		*val=ptr;
		return true;
	}
	return false;
}