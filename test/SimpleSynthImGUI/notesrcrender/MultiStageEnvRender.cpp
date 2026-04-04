#include "../ProjectObject.h"
#include "../SimpleSynthProject.h"
#include "NoteSrcRender.h"
#include "array/Array.hpp"
#include "events/NoteUpdater.h"
#include "imgui-knobs.h"
#include "imgui.h"
#include "implot.h"
#include "interpolator/LineInterpolator.h"
#include "synth/enveloper/MultiStageEnvelope.h"
#include "tuning/EqualTemperament.h"
#include "util/Lang.h"
#include "lang/Exception.h"
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_util;
using namespace yzrilyzr_array;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_lang;

void MultiStageEnvRenderFunc(CurrentProjectContext & ctx, ProjectObject & obj){
	bool changed=false;
	u_sp<MultiStageEnvelope> paramRegPtr=spsc<MultiStageEnvelope, ClassRegister>(obj.paramRegPtr);
	auto & pts=paramRegPtr->points;
	if(ImPlot::BeginPlot(ctx.LANG.getc("module.multi_stage_envelope.data"), ImVec2(500, 200), ImPlotFlags_NoLegend)){
		ImPlot::SetupAxis(ImAxis_Y1, "Value", ImPlotAxisFlags_NoLabel);
		ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 1.0, ImPlotCond_Always);
		ImPlot::SetupAxis(ImAxis_X1, "Index", ImPlotAxisFlags_NoLabel);
		if(pts.size() > 1){
			auto & last=pts[pts.size() - 1];
			ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, last.x * 1000, ImPlotCond_Always);
		}
		ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
		static constexpr int dataSize=10000;
		static float data[dataSize];
		if(pts.empty()){
			pts.emplace_back(MSEPoint{0, 0, MSEPointType::DEFAULT, MSEPointMode::HOLD, 0, 0});
			pts.emplace_back(MSEPoint{0.5, 1, MSEPointType::DECAY, MSEPointMode::SINGLE_CURVE, 1, 0});
			pts.emplace_back(MSEPoint{1, 0.5, MSEPointType::LOOP_START, MSEPointMode::SINGLE_CURVE, 1, 0});
			pts.emplace_back(MSEPoint{1.5, 0.8, MSEPointType::DEFAULT, MSEPointMode::SINGLE_CURVE, 1, 0});
			pts.emplace_back(MSEPoint{2, 0.3, MSEPointType::SUSTAIN_OR_LOOP_END, MSEPointMode::PULSE, 0.5, 0});
			pts.emplace_back(MSEPoint{3, 0, MSEPointType::DEFAULT, MSEPointMode::SINGLE_CURVE, 1, 0});
			ChannelConfig cfg;
			paramRegPtr->init(cfg);
		}

		if(pts.size() > 1){
			int endX=1000.0 * pts[pts.size() - 1].x;
			endX=Math::max(dataSize - 1, endX);
			MSEPoint * start=&pts[0];
			MSEPoint * end=&pts[1];
			for(u_index i=0;i < endX;i++){
				u_time t=i / 1000.0;
				if(t >= end->x){
					start=end;
					if(start->index + 1 < pts.size()){
						end=&pts[start->index + 1];
					} else{
						break;
					}
				}
				data[i]=MultiStageEnvelope::calcEnv(start->x, start->y, end->x, end->y, end->mode, end->modeValue, t);
			}
			FloatArray xs(pts.size());
			FloatArray ys(pts.size());
			for(u_index i=0;i < pts.size();i++){
				xs[i]=pts[i].x * 1000;
				ys[i]=pts[i].y;
			}
			ImPlot::PlotScatter("Point", xs.data(), ys.data(), xs.length);
			ImPlot::PlotLine(ctx.LANG.getc("module.multi_stage_envelope.data"), data, endX);
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
	if(ImGui::Button(ctx.LANG.getc("module.multi_stage_envelope.add_point"))){
		changed=true;
		float x=0;
		if(currentEdit >= 0 && currentEdit < pts.size()){
			x=pts[currentEdit].x + 0.01;
		}
		pts.insert(pts.begin() + currentEdit + 1, MSEPoint{x, 0.5, MSEPointType::DEFAULT, MSEPointMode::SINGLE_CURVE, 0});
	}
	ImGui::PushItemWidth(200);
	ImGui::InputInt(ctx.LANG.getc("module.multi_stage_envelope.current_edit"), &currentEdit);
	ImGui::PopItemWidth();
	if(currentEdit < 0)currentEdit=0;
	if(currentEdit >= pts.size())currentEdit=pts.size() - 1;
	if(currentEdit >= 0 && currentEdit < pts.size()){
		auto & p=pts[currentEdit];
		float oldX=p.x;
		if(ImGui::Button(ctx.LANG.getc("module.multi_stage_envelope.remove_point"))){
			changed=true;
			pts.erase(pts.begin() + currentEdit);
		}
		ImGui::PushItemWidth(400);
		if(ImGui::DragFloat(ctx.LANG.getc("module.multi_stage_envelope.current_edit_x"), &oldX, 0.001, 0.0, 10.0)){
			changed=true;
			float deltaX=oldX - p.x;
			for(u_index i=currentEdit;i < pts.size();i++){
				pts[i].x+=deltaX;
			}
		}
		changed=ImGui::DragFloat(ctx.LANG.getc("module.multi_stage_envelope.current_edit_y"), &p.y, 0.001, 0.0, 1) || changed;
		static LangToEnum modes;
		if(modes.empty(ctx.LANG)){
			modes.init(ctx.LANG, {
				//"multi_stage_envelope.mode.default",
				"multi_stage_envelope.mode.hold",
				"multi_stage_envelope.mode.smooth",
				"multi_stage_envelope.mode.single_curve",
				"multi_stage_envelope.mode.double_curve",
				"multi_stage_envelope.mode.half_sine",
				"multi_stage_envelope.mode.stage",
				"multi_stage_envelope.mode.smooth_stage",
				"multi_stage_envelope.mode.pulse",
				"multi_stage_envelope.mode.wave"
					   });
		}
		int mode=static_cast<int>(p.mode);
		if(ImGui::Combo(ctx.LANG.getc("module.multi_stage_envelope.current_edit_mode"), &mode, modes.data(), modes.size())){
			p.mode=(MSEPointMode)mode;
			changed=true;
		}
		changed=ImGui::DragFloat(ctx.LANG.getc("module.multi_stage_envelope.current_edit_modeValue"), &p.modeValue, 0.01, -1, 1) || changed;
		ImGui::PopItemWidth();
		ImGui::Text(ctx.LANG.getc("module.multi_stage_envelope.current_edit_type"));
		int type=static_cast<int>(p.type);
		bool changed1=false;
		//ImGui::SameLine();
		//changed1=ImGui::CheckboxFlags(ctx.LANG.getc("multi_stage_envelope.type.default"), &type, static_cast<int>(MSEPointType::DEFAULT)) || changed1;
		ImGui::SameLine();
		changed1=ImGui::CheckboxFlags(ctx.LANG.getc("multi_stage_envelope.type.decay"), &type, static_cast<int>(MSEPointType::DECAY)) || changed1;
		ImGui::SameLine();
		changed1=ImGui::CheckboxFlags(ctx.LANG.getc("multi_stage_envelope.type.loop_start"), &type, static_cast<int>(MSEPointType::LOOP_START)) || changed1;
		ImGui::SameLine();
		changed1=ImGui::CheckboxFlags(ctx.LANG.getc("multi_stage_envelope.type.sustain_or_loop_end"), &type, static_cast<int>(MSEPointType::SUSTAIN_OR_LOOP_END)) || changed1;
		if(changed1){
			p.type=(MSEPointType)type;
			changed=true;
		}
	}
	if(changed){
		ChannelConfig cfg;
		try{
			paramRegPtr->init(cfg);
		} catch(Exception ex){}
		ctx.paramChange=changed;
	}
}