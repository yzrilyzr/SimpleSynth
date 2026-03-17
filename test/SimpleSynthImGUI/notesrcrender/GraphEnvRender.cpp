#include "../ProjectObject.h"
#include "../SimpleSynthProject.h"
#include "NoteSrcRender.h"
#include "array/Array.hpp"
#include "events/NoteUpdater.h"
#include "imgui-knobs.h"
#include "imgui.h"
#include "implot.h"
#include "interpolator/LineInterpolator.h"
#include "synth/envelopers/GraphEnvelop.h"
#include "tuning/EqualTemperament.h"
#include "util/Lang.h"
#include "lang/Exception.h"
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_util;
using namespace yzrilyzr_array;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_lang;

void GraphEnvRenderFunc(CurrentProjectContext & ctx, ProjectObject & obj){
	bool changed=false;
	u_sp<GraphEnvelop> paramRegPtr=spsc<GraphEnvelop, ClassRegister>(obj.paramRegPtr);
	auto & pts=paramRegPtr->points;
	if(ImPlot::BeginPlot(ctx.LANG.getc("module.graph_envelope.data"), ImVec2(500, 200))){
		ImPlot::SetupAxis(ImAxis_Y1, "Value", ImPlotAxisFlags_NoLabel);
		ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 1.0, ImPlotCond_Always);
		ImPlot::SetupAxis(ImAxis_X1, "Time", ImPlotAxisFlags_NoLabel);
		if(pts.size() > 1){
			auto & last=pts[pts.size() - 1];
			ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, last.time * 1000, ImPlotCond_Always);
		}
		ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
		static constexpr int dataSize=10000;
		static float data[dataSize];
		if(pts.empty()){
			pts.emplace_back(GraphPoint{0, 0});
			pts.emplace_back(GraphPoint{0.1, 1});
			pts.emplace_back(GraphPoint{0.3, 0.2});
			pts.emplace_back(GraphPoint{0.4, 0.7});
			pts.emplace_back(GraphPoint{0.5, 0.2});
			pts.emplace_back(GraphPoint{0.6, 0.0});
			paramRegPtr->sustainPointIndex=1;
			paramRegPtr->loopStartPointIndex=2;
			paramRegPtr->loopEndPointIndex=4;
			ChannelConfig cfg;
			paramRegPtr->init(cfg);
		}

		if(pts.size() > 1){
			int endX=1000.0 * pts[pts.size() - 1].time;
			endX=Math::max(dataSize - 1, endX);
			GraphPoint * start=&pts[0];
			GraphPoint * end=&pts[1];
			u_index ptsIndex=0;
			for(u_index i=0;i < endX;i++){
				u_time t=i / 1000.0;
				if(t >= end->time){
					ptsIndex++;
					if(ptsIndex + 1 >= pts.size())break;
					start=&pts[ptsIndex];
					end=&pts[ptsIndex+1];
				}
				data[i]=Util::linearMap(start->time, end->time, start->env, end->env, t);
			}
			ImPlot::PlotLine(ctx.LANG.getc("module.graph_envelope.data"), data, endX);
		}
		ImPlotPoint mouse_pos=ImPlot::GetPlotMousePos(ImAxis_X1, ImAxis_Y1);
		bool is_mouse_in_plot=ImPlot::IsPlotHovered(); // 判断鼠标是否在绘图区域内
		if(is_mouse_in_plot && ImGui::IsMouseDown(ImGuiMouseButton_Left)){
			int index=mouse_pos.x;
			//if(index >= 0 && index < paramRegPtr->array->length){
				//paramRegPtr->array->_array[index]=mouse_pos.y;
			//}
		}
		ImPlot::EndPlot();
	}
	static int currentEdit=0;
	if(ImGui::Button(ctx.LANG.getc("module.graph_envelope.add_point"))){
		changed=true;
		float x=0;
		if(currentEdit >= 0 && currentEdit < pts.size()){
			x=pts[currentEdit].time + 0.01;
		}
		pts.insert(pts.begin() + currentEdit + 1, GraphPoint{x, 0.5});
	}
	ImGui::PushItemWidth(200);
	ImGui::InputInt(ctx.LANG.getc("module.graph_envelope.current_edit"), &currentEdit);
	ImGui::PopItemWidth();
	if(currentEdit < 0)currentEdit=0;
	if(currentEdit >= pts.size())currentEdit=pts.size() - 1;
	if(currentEdit >= 0 && currentEdit < pts.size()){
		auto & p=pts[currentEdit];
		float oldX=p.time;
		if(ImGui::Button(ctx.LANG.getc("module.graph_envelope.remove_point"))){
			changed=true;
			pts.erase(pts.begin() + currentEdit);
		}
		ImGui::PushItemWidth(400);
		if(ImGui::DragFloat(ctx.LANG.getc("module.graph_envelope.current_edit_x"), &oldX, 0.001, 0.0, 10.0)){
			changed=true;
			float deltaX=oldX - p.time;
			for(u_index i=currentEdit;i < pts.size();i++){
				pts[i].time+=deltaX;
			}
		}
		static double minDrag=0, maxDrag=1;
		changed=ImGui::DragScalar(ctx.LANG.getc("module.graph_envelope.current_edit_y"), ImGuiDataType_Double, &p.env, 0.001, &minDrag, &maxDrag) || changed;
	}
	changed=ImGui::InputInt(ctx.LANG.getc("module.graph_envelope.sustain"), &paramRegPtr->sustainPointIndex)|| changed;
	changed=ImGui::InputInt(ctx.LANG.getc("module.graph_envelope.start_loop"), &paramRegPtr->loopStartPointIndex)|| changed;
	changed=ImGui::InputInt(ctx.LANG.getc("module.graph_envelope.end_loop"), &paramRegPtr->loopEndPointIndex)|| changed;
	if(changed){
		ChannelConfig cfg;
		try{
			paramRegPtr->init(cfg);
		} catch(Exception ex){}
		ctx.paramChange=changed;
	}
}