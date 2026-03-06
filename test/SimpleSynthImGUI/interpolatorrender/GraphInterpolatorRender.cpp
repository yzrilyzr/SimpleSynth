#include "InterpolatorRender.h"
#include "imgui.h"
#include "interpolator/GraphInterpolator.h"
#include "lang/Boxing.h"
#include "implot.h"
#include "../SimpleSynthProject.h"
#include "array/Array.hpp"
using namespace yzrilyzr_util;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_array;
void graphInterpRenderFunc(CurrentProjectContext & ctx, ProjectObject & obj){
	u_sp<GraphInterpolator> paramRegPtr=std::dynamic_pointer_cast<GraphInterpolator, ClassRegister>(obj.paramRegPtr);
	auto & data=obj.storeData;
	if(data.find("PointSize") == data.end()){
		data["PointSize"]=mksp<Integer>(3);
	}
	Integer & PointSize=*spdc<Integer>(data["PointSize"]);
	ImGui::PushItemWidth(200);
	ImGui::InputInt(ctx.LANG.getc("module.interpolator.graph.points"), &PointSize.value);
	ImGui::PopItemWidth();
	if(PointSize.value < 1)PointSize.value=1;
	if(data.find("Points") != data.end()){
		paramRegPtr->xys=*spsc<DoubleArray>(data["Points"]);
	}
	if(ImGui::Button(ctx.LANG.getc("module.interpolator.graph.init"))){
		paramRegPtr->xys=DoubleArray(PointSize.value * 2);
		for(u_index i=0, j=paramRegPtr->xys.length / 2;i < j;i++){
			paramRegPtr->xys[i * 2]=(float)i / (j - 1);
			paramRegPtr->xys[i * 2 + 1]=(float)i / (j - 1);
		}
		data["Points"]=mksp<DoubleArray>(paramRegPtr->xys);
	}
	if(paramRegPtr->xys.empty())return;
	if(ImPlot::BeginPlot(ctx.LANG.getc("module.interpolator.graph.plot.title"), ImVec2(500, 500))){
		ImPlot::SetupAxis(ImAxis_Y1, "Y", ImPlotAxisFlags_NoLabel);
		ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 1.0, ImPlotCond_Always);
		ImPlot::SetupAxis(ImAxis_X1, "X", ImPlotAxisFlags_NoLabel);
		ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, 1.0, ImPlotCond_Always);
		ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
		auto xs=DoubleArray(paramRegPtr->xys.length / 2);
		auto ys=DoubleArray(paramRegPtr->xys.length / 2);
		for(u_index i=0;i < xs.length;i++){
			xs[i]=paramRegPtr->xys[i * 2];
			ys[i]=paramRegPtr->xys[i * 2 + 1];
		}
		ImPlot::PlotLine(ctx.LANG.getc("module.interpolator.graph.plot.line"), xs._array, ys._array, paramRegPtr->xys.length / 2);
		ImPlot::PushStyleVar(ImPlotStyleVar_MarkerSize, 5);
		ImPlot::PlotScatter(ctx.LANG.getc("module.interpolator.graph.plot.point"), xs._array, ys._array, paramRegPtr->xys.length / 2);
		ImPlot::PopStyleVar();
		ImPlotPoint mouse_pos=ImPlot::GetPlotMousePos(ImAxis_X1, ImAxis_Y1);
		ImVec2 mouse_pixel_pos=ImGui::GetMousePos(); // 获取鼠标的屏幕像素位置
		bool is_mouse_in_plot=ImPlot::IsPlotHovered(); // 判断鼠标是否在绘图区域内
		static int selected=-1;
		if(is_mouse_in_plot && ImGui::IsMouseDown(ImGuiMouseButton_Left) && selected == -1){
			double mx=mouse_pos.x;
			double my=mouse_pos.y;
			double disMin=1e9;
			for(u_index i=0, j=paramRegPtr->xys.length / 2;i < j;i++){
				double px=paramRegPtr->xys[i * 2];
				double py=paramRegPtr->xys[i * 2 + 1];
				double dis=std::hypot(px - mx, py - my);
				if(dis < disMin){
					disMin=dis;
					selected=i;
				}
			}
			if(disMin > 0.08){
				selected=-2;//unselected
			}
		} else if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
			selected=-1;
		} else if(ImGui::IsMouseDown(ImGuiMouseButton_Left) && selected >= 0){
			double mx=Util::clamp01(mouse_pos.x);
			double my=Util::clamp01(mouse_pos.y);
			int i=selected;
			paramRegPtr->xys[i * 2]=mx;
			paramRegPtr->xys[i * 2 + 1]=my;
		}
		ImPlot::EndPlot();
	}
}