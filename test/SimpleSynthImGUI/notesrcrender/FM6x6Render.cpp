#include "NoteSrcRender.h"
#include "../ProjectObject.h"
#include "../SimpleSynthProject.h"
#include "synth/composed/Matrix6x6Modulation.h"
#include "array/Array.hpp"
#include "imgui.h"
#include "imgui-knobs.h"
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_util;
using namespace yzrilyzr_array;
using namespace yzrilyzr_lang;
bool renderMatrix(u_sp<Matrix6x6Modulation> paramRegPtr, int type){
	bool change=false;
	const u_index MTX_SIZE=Matrix6x6Modulation::MATRIX_SIZE;
	u_index cols=MTX_SIZE + 2;
	ImGuiKnobFlags knobFlags=ImGuiKnobFlags_NoTitle | ImGuiKnobFlags_NoInput;
	ImGuiKnobVariant knobvariant=ImGuiKnobVariant_Tick;
	if(ImGui::BeginTable("Matrix", cols)){
		ImGui::TableNextRow();
		for(u_index col=0; col < cols; col++){
			ImGui::TableSetColumnIndex(col);
			if(col > 0 && col < cols - 1)ImGui::Text("%d", col);
			else if(col == cols - 1)ImGui::Text("Out");
		}
		for(u_index row=0; row < MTX_SIZE; row++){
			ImGui::TableNextRow();
			for(u_index col=0; col < cols; col++){
				ImGui::TableSetColumnIndex(col);
				u_index opIndex=col - 1;
				if(col == 0){
					ImGui::Text("%d", row + 1);
				} else if(col == cols - 1){
					String lab=String("MatrixOut") + std::to_string(col) + std::to_string(row);
					float fv=paramRegPtr->op[row].output;
					if(ImGuiKnobs::Knob(lab.c_str(UTF8), &fv, -1.0f, 1.0f, 0.01f, "%.2f", knobvariant, 40, knobFlags)){
						change=true;
						paramRegPtr->op[row].output=fv;
					}
				} else{
					if(type == 0){
						u_sample(&matrix)[MTX_SIZE][MTX_SIZE]=paramRegPtr->fmMatrix;
						float fv=matrix[opIndex][row];
						String lab=String("FMMatrix") + std::to_string(col) + std::to_string(row);
						if(opIndex == row){
							if(ImGuiKnobs::Knob(lab.c_str(UTF8), &fv, -1.0f, 1.0f, 0.01f, "%.2f", knobvariant, 40, knobFlags)){
								change=true;
								matrix[opIndex][row]=fv;
							}
						} else{
							if(ImGuiKnobs::Knob(lab.c_str(UTF8), &fv, -16.0f, 16.0f, 0.05f, "%.2f", knobvariant, 40, knobFlags)){
								change=true;
								matrix[opIndex][row]=fv;
							}
						}
					} else if(type == 1){
						u_sample(&matrix)[MTX_SIZE][MTX_SIZE]=paramRegPtr->rmMatrix;
						float fv=matrix[opIndex][row];
						String lab=String("RMMatrix") + std::to_string(col) + std::to_string(row);
						if(opIndex == row){
							if(ImGuiKnobs::Knob(lab.c_str(UTF8), &fv, -1.0f, 1.0f, 0.01f, "%.2f", knobvariant, 40, knobFlags)){
								change=true;
								matrix[opIndex][row]=fv;
							}
						} else{
							if(ImGuiKnobs::Knob(lab.c_str(UTF8), &fv, -1.0f, 1.0f, 0.01f, "%.2f", knobvariant, 40, knobFlags)){
								change=true;
								matrix[opIndex][row]=fv;
							}
						}
					}
				}
			}
		}
		ImGui::EndTable();
	}
	return change;
}
void Matrix6x6ModulationRenderFunc(CurrentProjectContext & ctx, ProjectObject & obj){
	u_sp<Matrix6x6Modulation> paramRegPtr=std::dynamic_pointer_cast<Matrix6x6Modulation, ParamRegister>(obj.paramRegPtr);
	u_index MTX_SIZE=Matrix6x6Modulation::MATRIX_SIZE;
	auto & data=obj.storeData;
	if(obj.loadStoredData){
		obj.loadStoredData=false;
		auto it=data.find("FM_Matrix");
		if(it != data.end()){
			u_sp<SampleArray> arr=std::dynamic_pointer_cast<SampleArray>(it->second);
			u_index k=0;
			for(u_index x=0;x < MTX_SIZE;x++){
				for(u_index y=0;y < MTX_SIZE;y++){
					paramRegPtr->fmMatrix[x][y]=(*arr)[k++];
				}
			}
		}
		it=data.find("RM_Matrix");
		if(it != data.end()){
			u_sp<SampleArray> arr=std::dynamic_pointer_cast<SampleArray>(it->second);
			u_index k=0;
			for(u_index x=0;x < MTX_SIZE;x++){
				for(u_index y=0;y < MTX_SIZE;y++){
					paramRegPtr->rmMatrix[x][y]=(*arr)[k++];
				}
			}
		}
	}
	if(ImGui::BeginTabBar("OpTab", ImGuiTabBarFlags_None)){
		for(u_index i=0;i < MTX_SIZE;i++){
			String tabstr=String("OP ") + std::to_string(i + 1);
			FMOp & op=paramRegPtr->op[i];
			if(ImGui::BeginTabItem(tabstr.c_str(UTF8))){
				ProjectObject::renderParams(ctx, static_cast<ParamRegister &>(op).RegisteredParams);
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}
	ImGui::Separator();
	bool change=false;
	if(ImGui::BeginTabBar("MatrixTab", ImGuiTabBarFlags_None)){
		if(ImGui::BeginTabItem(ctx.LANG.getc("module.matrix_6x6_modulation.fm"))){
			change=renderMatrix(paramRegPtr, 0) || change;
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem(ctx.LANG.getc("module.matrix_6x6_modulation.rm"))){
			change=renderMatrix(paramRegPtr, 1) || change;
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	if(change){
		u_sp<SampleArray> fmArr=mksp<SampleArray>(MTX_SIZE * MTX_SIZE);
		u_index k=0;
		for(u_index x=0;x < MTX_SIZE;x++){
			for(u_index y=0;y < MTX_SIZE;y++){
				(*fmArr)[k++]=paramRegPtr->fmMatrix[x][y];
			}
		}
		u_sp<SampleArray> rmArr=mksp<SampleArray>(MTX_SIZE * MTX_SIZE);
		k=0;
		for(u_index x=0;x < MTX_SIZE;x++){
			for(u_index y=0;y < MTX_SIZE;y++){
				(*rmArr)[k++]=paramRegPtr->rmMatrix[x][y];
			}
		}
		data["FM_Matrix"]=fmArr;
		data["RM_Matrix"]=rmArr;
	}
	ctx.paramChange=change || ctx.paramChange;
}