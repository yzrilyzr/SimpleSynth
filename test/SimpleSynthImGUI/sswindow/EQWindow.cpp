#include "../LangToEnum.h"
#include "../SimpleSynthProject.h"
#include "../SimpleSynthWindow.h"
#include "dsp/BiquadIIR.h"
#include "dsp/DSPChain.h"
#include "dsp/FFT.h"
#include "dsp/FilterPassType.h"
#include "dsp/IIR.h"
#include "dsp/IIRUtil.h"
#include "imgui.h"
#include "implot.h"
#include "interface/IChannel.h"
#include "interface/IMixer.h"
#include <shared_mutex>
using namespace yzrilyzr_array;
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
using namespace yzrilyzr_io;
using namespace yzrilyzr_collection;
using namespace yzrilyzr_lang;

double calculateDbPerOctave(double * frequencies, double * magnitudes, int index, int size){
	if(index <= 0 || index >= size){
		std::cerr << "Index out of bounds for dB/oct calculation." << std::endl;
		return 0.0;
	}

	// 选择相邻的频率点
	double f1=frequencies[index - 1];
	double f2=frequencies[index];
	double f3=frequencies[index + 1];

	// 选择对应的dB值
	double mag1=magnitudes[index - 1];
	double mag2=magnitudes[index];
	double mag3=magnitudes[index + 1];

	// 计算倍频程变化
	double octaveChange1=log2(f2 / f1);
	double octaveChange2=log2(f3 / f2);

	// 计算dB变化
	double dbChange1=mag2 - mag1;
	double dbChange2=mag3 - mag2;

	// 计算dB/oct
	double dbPerOctave1=dbChange1 / octaveChange1;
	double dbPerOctave2=dbChange2 / octaveChange2;

	// 计算平均的dB/oct
	return (dbPerOctave1 + dbPerOctave2) / 2.0;
}

void eqWindow(CurrentProjectContext & ctx){
	IMixer & mixer=*ctx.mixer;
	ImGui::Begin(ctx.LANG.getc("window.eq.title"));
	struct StructEQ{
		std::shared_ptr<IIR> iir=nullptr;
		double q=0;
		double freq=0;
		double gain=0;
		FilterPassType type=BANDPASS;
	};
	static ArrayList<StructEQ *> eqs;
	u_sample_rate sampleRate=mixer.getSampleRate();
	u_sample_rate test_sampleRate=44100;
	const u_index siz=1 << 16;
	const u_index fsiz=siz >> 1;
	static SampleArray input_data(siz);
	static SampleArray mag_data(siz);
	static SampleArray pha_data(siz);
	static double * x_data=new double[fsiz];
	static FFT fft(siz);
	static bool changed=true;
	static StructEQ * currentEdit=nullptr;
	if(changed){
		changed=false;
		DSPChain dsp;
		for(StructEQ * s : eqs){
			if(s->iir == nullptr){
				s->iir=std::make_shared<IIR>(2, 3);
			}
			//s->iir=IIRUtil::newButterworthIIRFilter(test_sampleRate, FilterPassType::LOWPASS, 16, s->freq, 0);
			IIRUtil::biquad(*s->iir->aCoeff, *s->iir->bCoeff, s->freq, test_sampleRate, s->q, s->type, s->gain);
			//IIRUtil::designThiranFilter(*s->iir->aCoeff, *s->iir->bCoeff, s->freq, 15);
			dsp.add(s->iir);
		}
		Arrays::fill(input_data, 1, siz,static_cast<u_sample>(0));
		input_data[0]=fsiz;
		for(u_index i=0;i < siz;i++){
			input_data[i]=dsp.procDsp(input_data[i]);
		}
		for(u_index i=0;i < fsiz;i++){
			x_data[i]=(double)i * test_sampleRate / fsiz / 2.0;
		}
		fft.input(input_data);
		fft.fft();
		fft.outputMagnitude(mag_data);
		fft.outputPhase(pha_data);
		for(u_index i=0;i < fsiz;i++){
			mag_data[i]=20.0 * log10(mag_data[i]);
			pha_data[i]=pha_data[i] * 180.0 / PI;
		}
		//
		mixer.setUseEQ(true);
		std::shared_ptr<DSPChain> * c=mixer.getEQ();
		std::mutex & dspLock=mixer.getDSPLock();
		std::unique_lock <std::mutex > lock(dspLock);
		for(u_index ii=0;ii < 2;ii++){
			std::shared_ptr<DSPChain> cc=c[ii];
			cc->clear();
			for(StructEQ * s : eqs){
				std::shared_ptr<IIR> b=std::make_shared<IIR>();
				b->cloneParam(s->iir.get());
				cc->add(b);
			}
		}
	}
	static LangToEnum filterType;
	if(filterType.empty(ctx.LANG)){
		filterType.init(ctx.LANG, {
			"filter.type.lowpass",
			"filter.type.highpass",
			"filter.type.bandpass",
			"filter.type.bandstop",
			"filter.type.notch",
			"filter.type.lowshelf",
			"filter.type.highshelf",
			"filter.type.bell",
			"filter.type.allpass"
						});
	}
	if(ImPlot::BeginPlot(ctx.LANG.getc("window.eq.freq_response"), ImVec2(1200, 700))){
		ImPlot::SetupAxis(ImAxis_Y1, ctx.LANG.getc("window.eq.freq_response.magnitude"), ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoLabel);
		ImPlot::SetupAxisLimits(ImAxis_Y1, -30, 30);
		ImPlot::SetupAxis(ImAxis_Y2, ctx.LANG.getc("window.eq.freq_response.phase"), ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoLabel);
		ImPlot::SetupAxisLimits(ImAxis_Y2, -180, 180);
		ImPlot::SetupAxis(ImAxis_X1, ctx.LANG.getc("window.eq.freq_response.freq"), ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoLabel);
		ImPlot::SetupAxisLimits(ImAxis_X1, 1, 21000);
		ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
		ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
		auto mag1=Arrays::cast<double>(mag_data);
		ImPlot::PlotLine(ctx.LANG.getc("window.eq.freq_response.magnitude"), x_data, mag1->_array, fsiz);
		ImPlotPoint point=ImPlot::GetPlotMousePos(ImAxis_X1, ImAxis_Y1);
		bool is_mouse_in_plot=ImPlot::IsPlotHovered();
		bool isLeftDown=ImGui::IsMouseDown(ImGuiMouseButton_Left);
		double minDistance=1e9;
		static int dragState=0;//0 默认 1进入查找 2找到 3未找到
		if(is_mouse_in_plot && isLeftDown && dragState == 0){
			dragState=1;
			for(StructEQ * s : eqs){
				ImVec2 point1=ImPlot::PlotToPixels(point.x, point.y, ImAxis_X1, ImAxis_Y1);
				ImVec2 point2=ImPlot::PlotToPixels(s->freq, s->gain, ImAxis_X1, ImAxis_Y1);
				double dis=std::hypot(point1.x - point2.x, point1.y - point2.y);
				if(dis < minDistance){
					minDistance=dis;
					currentEdit=s;
				}
			}
		}
		if(dragState == 1){
			if(minDistance < 10){
				dragState=2;
			} else{
				currentEdit=nullptr;
				dragState=3;
			}
		}
		if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
			dragState=0;
		}
		if(ImGui::IsMouseDragging(ImGuiMouseButton_Left) && dragState == 3){
			dragState=2;
			changed=true;
			StructEQ * str=new StructEQ();
			str->type=FilterPassType::BELL;
			str->freq=1000;
			str->gain=1;
			str->q=0.707;
			eqs.add(str);
			currentEdit=str;
		}
		for(StructEQ * s : eqs){
			double * px=&s->freq;
			double * py=&s->gain;
			ImPlot::PushStyleVar(ImPlotStyleVar_MarkerSize, currentEdit == s?10:5);
			ImPlot::PlotScatter(filterType[s->type], px, py, 1);
			ImPlot::PopStyleVar();
		}

		if(currentEdit != nullptr && dragState == 2){
			changed=true;
			currentEdit->freq=point.x;
			ImVec2 dragDelta=ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
			if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl))currentEdit->q=pow(10, -dragDelta.y / 100.0);
			else currentEdit->gain=point.y;
		}
		if(ImPlot::IsPlotHovered()){
			ImGui::BeginTooltip();
			double slope=calculateDbPerOctave(x_data, mag1->_array, point.x, siz);
			ImGui::Text(ctx.LANG.getf("window.eq.freq_response.hover", point.x, slope).c_str(UTF8));
			ImGui::EndTooltip();
		}

		ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
		auto pha1=Arrays::cast<double>(pha_data);
		ImPlot::PlotLine(ctx.LANG.getc("window.eq.freq_response.phase"), x_data, pha1->_array, fsiz);
		ImPlot::EndPlot();
	}
	static double freq_min=5;
	static double freq_max=20000;
	static double q_min=0.01;
	static double q_max=10;
	static double gain_min=-50;
	static double gain_max=50;
	if(currentEdit != nullptr){
		static int type=0;
		type=currentEdit->type;
		if(ImGui::Combo(ctx.LANG.getc("window.eq.filter.type"), &type, filterType.data(), filterType.size())){
			currentEdit->type=(FilterPassType)type;
			changed=true;
		}
		changed=ImGui::SliderScalar(ctx.LANG.getc("window.eq.filter.freq"), ImGuiDataType_Double, &currentEdit->freq, &freq_min, &freq_max, "%.2f", ImGuiSliderFlags_Logarithmic) || changed;
		changed=ImGui::SliderScalar(ctx.LANG.getc("window.eq.filter.q"), ImGuiDataType_Double, &currentEdit->q, &q_min, &q_max, "%.2f") || changed;
		changed=ImGui::SliderScalar(ctx.LANG.getc("window.eq.filter.gain"), ImGuiDataType_Double, &currentEdit->gain, &gain_min, &gain_max, "%.2f") || changed;
		bool removeEq=ImGui::Button(ctx.LANG.getc("window.eq.filter.remove"));
		changed=changed || removeEq;
		if(removeEq){
			eqs.remove(currentEdit);
			currentEdit=nullptr;
		}
	}
	ImGui::End();
}