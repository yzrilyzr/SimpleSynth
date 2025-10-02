#include "../SimpleSynthProject.h"
#include "../SimpleSynthWindow.h"
#include "interface/IMixer.h"
#include "interface/IChannel.h"
#include "array/Array.hpp"
#include "dsp/RMSCompute.h"
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_array;
using namespace yzrilyzr_lang;
void oscilloscopeWindow(CurrentProjectContext & ctx){
	IMixer & mixer=*ctx.mixer;
	ImGui::Begin(ctx.LANG.getc("window.oscilloscope.title"));
	static float * floatValues[2];
	static u_index length=0;
	static RMSCompute rmsL(2048), rmsR(2048);
	if(length == 0){
		length=mixer.getBufferSize();
		floatValues[0]=new float[length];
		floatValues[1]=new float[length];
	}
	u_sample* ch0=mixer.getOutput(0);
	u_sample* ch1=mixer.getOutput(1);
	for(u_index i=0;i < length;i++){
		floatValues[0][i]=(float)ch0[i];
		floatValues[1][i]=(float)ch1[i];
		rmsL.procDsp(ch0[i]);
		rmsR.procDsp(ch1[i]);
	}
	//rmsL.procBlock(mixer.output[0], length);
	//rmsR.procBlock(mixer.output[1], length);
	ImGui::PlotLines(ctx.LANG.getc("window.oscilloscope.master_l"), floatValues[0], length, 0, "", -1.0f, 1.0f, ImVec2(500, 80.0f));
	ImGui::PlotLines(ctx.LANG.getc("window.oscilloscope.master_r"), floatValues[1], length, 0, "", -1.0f, 1.0f, ImVec2(500, 80.0f));
	ImGui::ProgressBar(rmsL.rms());
	ImGui::ProgressBar(rmsR.rms());
	if(ImGui::BeginTable("table1", 2)){
		for(auto &channel : mixer.getAllChannels()){
			u_sample* cch0=channel->getOutput(0);
			u_sample* cch1=channel->getOutput(1);
			for(u_index i=0;i < length;i++){
				floatValues[0][i]=(float)cch0[i];
				floatValues[1][i]=(float)cch1[i];
			}
			ImGui::TableNextColumn();
			ImGui::PlotLines((channel->getName() + " L").c_str(UTF8), floatValues[0], length, 0, "", -1.0f, 1.0f, ImVec2(200, 40.0f));
			ImGui::PlotLines((channel->getName() + " R").c_str(UTF8), floatValues[1], length, 0, "", -1.0f, 1.0f, ImVec2(200, 40.0f));
		}
		ImGui::EndTable();
	}

	ImGui::End();
}