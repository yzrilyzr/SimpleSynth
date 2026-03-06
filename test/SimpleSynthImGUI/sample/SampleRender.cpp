#include "SampleRender.h"
#include "array/SampleProvider.h"
#include "lang/Boxing.h"
#include "util/Random.h"
#include "dsp/PWM.h"
#include "imgui.h"
#include "implot.h"
#include "dsp/IIRUtil.h"
#include "dsp/RingBuffer.h"
#include "../SimpleSynthProject.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_array;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
void gaussianSmooth(SampleArray & data, SampleArray & smoothed, u_sample sigma, int windowSize){
	int length=data.length;
	if(length == 0 || windowSize <= 0) throw IllegalStateException();
	int halfWindow=windowSize / 2;
	// 预计算高斯核
	std::vector<u_sample> kernel(windowSize);
	float kernelSum=0.0f;
	for(u_index j=-halfWindow; j <= halfWindow; j++){
		u_sample x=j;
		kernel[j + halfWindow]=std::exp(-x * x / (2 * sigma * sigma));
		kernelSum+=kernel[j + halfWindow];
	}
	// 归一化核
	for(u_index j=0; j < windowSize; j++){
		kernel[j]/=kernelSum;
	}
	// 应用高斯滤波
	for(u_index i=0; i < length; i++){
		u_sample sum=0.0f;
		for(u_index j=-halfWindow; j <= halfWindow; j++){
			int idx=RingBufferUtil::mod(i + j, length);
			sum+=data[idx] * kernel[j + halfWindow];
		}
		smoothed[i]=sum;
	}
}
void sampleDataRenderFunc(CurrentProjectContext & ctx, ProjectObject & obj){
	// TODO fix
	u_sp<SampleArrayProvider> paramRegPtr=spdc<SampleArrayProvider, ClassRegister>(obj.paramRegPtr);
	auto & data=obj.storeData;
	if(data.find("SampleLength") == data.end()){
		data["SampleLength"]=mksp<Integer>(256);
	}
	Integer & SampleLength=*spdc<Integer>(data["SampleLength"]);
	ImGui::PushItemWidth(200);
	ImGui::InputInt(ctx.LANG.getc("module.sample.length"), &SampleLength.value);
	ImGui::PopItemWidth();
	if(SampleLength.value < 1)SampleLength.value=1;
	if(data.find("SampleData") != data.end()){
		paramRegPtr->array=*spsc<SampleArray>(data["SampleData"]);
		paramRegPtr->length=paramRegPtr->array.length;
	}
	if(ImGui::Button(ctx.LANG.getc("module.sample.init"))){
		paramRegPtr->array=SampleArray(SampleLength.value);
		paramRegPtr->length=SampleLength.value;
		data["SampleData"]=mksp<SampleArray>(paramRegPtr->array);
	}
	if(paramRegPtr->array==nullptr)return;
	if(ImPlot::BeginPlot(ctx.LANG.getc("module.sample.data"), ImVec2(500, 500))){
		ImPlot::SetupAxis(ImAxis_Y1, ctx.LANG.getc("module.sample.wave"), ImPlotAxisFlags_NoLabel);
		ImPlot::SetupAxisLimits(ImAxis_Y1, -1.0, 1.0, ImPlotCond_Always);
		ImPlot::SetupAxis(ImAxis_X1, "Index", ImPlotAxisFlags_NoLabel);
		ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, paramRegPtr->length - 1, ImPlotCond_Always);
		ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
		ImPlot::PlotLine(ctx.LANG.getc("module.sample.wave"), paramRegPtr->array._array, paramRegPtr->length);

		ImPlotPoint mouse_pos=ImPlot::GetPlotMousePos(ImAxis_X1, ImAxis_Y1);
		bool is_mouse_in_plot=ImPlot::IsPlotHovered(); // 判断鼠标是否在绘图区域内
		if(is_mouse_in_plot && ImGui::IsMouseDown(ImGuiMouseButton_Left)){
			int index=mouse_pos.x;
			if(index >= 0 && index < paramRegPtr->array.length){
				paramRegPtr->array._array[index]=mouse_pos.y;
			}
		}
		ImPlot::EndPlot();
	}
	ImGui::Text(ctx.LANG.getc("module.sample.fast_fill"));

	if(ImGui::Button(ctx.LANG.getc("module.sample.fill.random"))){
		Random random;
		for(u_index i=0;i < paramRegPtr->length;i++){
			paramRegPtr->array._array[i]=random.nextGaussian();
		}
	}
	ImGui::SameLine();
	if(ImGui::Button(ctx.LANG.getc("module.sample.fill.sine"))){
		for(u_index i=0;i < paramRegPtr->length;i++){
			paramRegPtr->array._array[i]=Math::sin( Math::TAU * i / paramRegPtr->length);
		}
	}
	ImGui::SameLine();
	if(ImGui::Button(ctx.LANG.getc("module.sample.fill.saw"))){
		for(u_index i=0;i < paramRegPtr->length;i++){
			paramRegPtr->array._array[i]=PWM::pwm((double)i / paramRegPtr->length, 0, 1, 0, 0);
		}
	}
	ImGui::SameLine();
	if(ImGui::Button(ctx.LANG.getc("module.sample.fill.tri"))){
		for(u_index i=0;i < paramRegPtr->length;i++){
			paramRegPtr->array._array[i]=PWM::pwm((double)i / paramRegPtr->length, 0, 0.5, 0.5, 0);
		}
	}
	ImGui::SameLine();
	if(ImGui::Button(ctx.LANG.getc("module.sample.fill.square"))){
		for(u_index i=0;i < paramRegPtr->length;i++){
			paramRegPtr->array._array[i]=PWM::pwm((double)i / paramRegPtr->length, 0.5, 0, 0, 0);
		}
	}

	ImGui::Text(ctx.LANG.getc("module.sample.utils"));

	if(ImGui::Button(ctx.LANG.getc("module.sample.utils.dc_filter"))){
		u_sample sum=0;
		for(u_index i=0;i < paramRegPtr->length;i++){
			sum+=paramRegPtr->array._array[i];
		}
		sum/=(u_sample)paramRegPtr->length;
		for(u_index i=0;i < paramRegPtr->length;i++){
			paramRegPtr->array._array[i]-=sum;
		}
	}
	ImGui::SameLine();
	if(ImGui::Button(ctx.LANG.getc("module.sample.utils.light_smooth"))){
		auto sm=mksp<SampleArray>(paramRegPtr->length);
		gaussianSmooth(paramRegPtr->array, *sm, 0.5, 3);
		data["SampleData"]=sm;
	}
	ImGui::SameLine();
	if(ImGui::Button(ctx.LANG.getc("module.sample.utils.strong_smooth"))){
		auto sm=mksp<SampleArray>(paramRegPtr->length);
		gaussianSmooth(paramRegPtr->array, *sm, 5, 13);
		data["SampleData"]=sm;
	}
	ImGui::SameLine();
	if(ImGui::Button(ctx.LANG.getc("module.sample.utils.zoom_in"))){
		for(u_index i=0;i < paramRegPtr->length;i++){
			paramRegPtr->array._array[i]*=1.1;
		}
	}
	ImGui::SameLine();
	if(ImGui::Button(ctx.LANG.getc("module.sample.utils.zoom_out"))){
		for(u_index i=0;i < paramRegPtr->length;i++){
			paramRegPtr->array._array[i]*=0.9;
		}
	}
	ImGui::SameLine();
	if(ImGui::Button(ctx.LANG.getc("module.sample.utils.move_left"))){
		u_sample first=paramRegPtr->array._array[0];
		for(u_index i=0;i < paramRegPtr->length - 1;i++){
			paramRegPtr->array._array[i]=paramRegPtr->array._array[i + 1];
		}
		paramRegPtr->array._array[paramRegPtr->length - 1]=first;
	}
	ImGui::SameLine();
	if(ImGui::Button(ctx.LANG.getc("module.sample.utils.move_right"))){
		u_sample last=paramRegPtr->array._array[paramRegPtr->length - 1];
		for(int32_t i=paramRegPtr->length - 1;i >= 0;i--){
			paramRegPtr->array._array[i]=paramRegPtr->array._array[i - 1];
		}
		paramRegPtr->array._array[0]=last;
	}
}