#include "../LangToEnum.h"
#include "../SimpleSynthProject.h"
#include "../SimpleSynthWindow.h"
#include "interface/IChannel.h"
#include "interface/IMixer.h"
#include "events/ChannelConfig.h"
#include "SynthUtil.h"
#include "array/Array.hpp"
#include "dsp/Chorus.h"
#include "dsp/Freeverb.h"
#include "dsp/RMSCompute.h"
using namespace yzrilyzr_lang;
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_array;
void channelSettingWindow(CurrentProjectContext & ctx){
	IMixer & mixer=*ctx.mixer;
	ImGui::Begin(ctx.LANG.getc("window.channel.title"));
	static bool sending=true;
	ImGui::Checkbox(ctx.LANG.getc("window.channel.enable_keyboard_sending"), &sending);
	static int sendToChannel=20;
	static int noteShift=0;
	ImGui::InputInt(ctx.LANG.getc("window.channel.send_to_channel"), &sendToChannel);
	ImGui::InputInt(ctx.LANG.getc("window.channel.note_shift"), &noteShift);
	if(sendToChannel < 0)sendToChannel=0;
	static u_sp<IChannel> ch=nullptr;
	ch=mixer.getMIDIChannel("WM_MIDI_Instant", sendToChannel);
	if(sendToChannel == 20){
		//ch->setAlwaysActive(true);
		//ch->setChorus(0);
	}
	ch->getConfig().NoteShift=noteShift;
	static s_program_id program=0;
	if(ImGui::InputInt(ctx.LANG.getc("window.channel.program"), &program)){
		if(sendToChannel != 20){
			ProgramChange * event=new ProgramChange(program);
			event->groupName="WM_MIDI_Instant";
			event->channelID=sendToChannel;
			mixer.sendInstantEvent(event);
		}
	}
	ImGuiKey start_key=ImGuiKey_0;
	static ImGuiKey keyIDs[28]={
		ImGuiKey_Z, ImGuiKey_X, ImGuiKey_C, ImGuiKey_V, ImGuiKey_B, ImGuiKey_N, ImGuiKey_M,
		ImGuiKey_A, ImGuiKey_S, ImGuiKey_D, ImGuiKey_F, ImGuiKey_G, ImGuiKey_H, ImGuiKey_J,
		ImGuiKey_Q, ImGuiKey_W, ImGuiKey_E, ImGuiKey_R, ImGuiKey_T, ImGuiKey_Y, ImGuiKey_U,
		ImGuiKey_1, ImGuiKey_2, ImGuiKey_3, ImGuiKey_4, ImGuiKey_5, ImGuiKey_6, ImGuiKey_7
	};
	static uint8_t triggerIDShifts[7]={0, 2, 4, 5, 7, 9, 11};
	static bool isKeyDownState[36]={false};
	if(sending){
		for(u_index ii=0;ii < 28;ii++){
			ImGuiKey key=keyIDs[ii];
			bool down=ImGui::IsKeyDown(key);
			s_note_id noteid=36 + (ii / 7) * 12 + triggerIDShifts[ii % 7];
			s_note_vel vel=1;
			if(down && !isKeyDownState[ii]){
				isKeyDownState[ii]=true;
				NoteOn * event=new NoteOn(sendToChannel, noteid, vel);
				event->groupName="WM_MIDI_Instant";
				mixer.sendInstantEvent(event);
			} else if(!down && isKeyDownState[ii]){
				isKeyDownState[ii]=false;
				NoteOff * event=new NoteOff(sendToChannel, noteid, vel);
				event->groupName="WM_MIDI_Instant";
				mixer.sendInstantEvent(event);
			}
		}
	}
	if(ImGui::Button(ctx.LANG.getc("window.channel.reset"))){
		ch->reset();
	}
	static LangToEnum disableNames;
	if(disableNames.empty(ctx.LANG)){
		disableNames.init(ctx.LANG, {
			"window.channel.enable.channel_control",
			"window.channel.enable.program_change"
						  });
	}
	static bool * bVal=nullptr;
	for(u_index i=0;i < disableNames.size();i++){
		if(i > 0)ImGui::SameLine();
		if(i == 0)bVal=&ch->ENABLE_MIDI_CHANNEL_CONTROL;
		else if(i == 1)bVal=&ch->ENABLE_MIDI_PROGRAM_CHANGE;
		ImVec4 color=ImColor::HSV((*bVal)?2.0f / 7.0f:0.0f, 0.8f, 0.8f);
		ImGui::PushStyleColor(ImGuiCol_Button, color);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
		if(ImGui::Button(disableNames[i])){
			*bVal=!(*bVal);
		}
		ImGui::PopStyleColor(3);
	}
	static LangToEnum ccControlNames;
	if(ccControlNames.empty(ctx.LANG)){
		ccControlNames.init(ctx.LANG, {
			"window.channel.cc.soft_pedal",
			"window.channel.cc.sostenuto",
			"window.channel.cc.sustain",
			"window.channel.cc.legato",
			"window.channel.cc.portamento"
							});
	}
	for(u_index i=0;i < ccControlNames.size();i++){
		if(i > 0)ImGui::SameLine();
		if(i == 0)bVal=&ch->getConfig().SoftPedal;
		else if(i == 1)bVal=&ch->getConfig().Sostenuto;
		else if(i == 2)bVal=&ch->getConfig().Sustain;
		else if(i == 3)bVal=&ch->getConfig().Legato;
		else if(i == 4)bVal=&ch->getConfig().Portamento;
		ImVec4 color=ImColor::HSV((*bVal)?2.0f / 7.0f:0.0f, 0.8f, 0.8f);
		ImGui::PushStyleColor(ImGuiCol_Button, color);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
		if(ImGui::Button(ccControlNames[i])){
			*bVal=!(*bVal);
		}
		ImGui::PopStyleColor(3);
	}
	static LangToEnum ccControlNames2;
	if(ccControlNames2.empty(ctx.LANG)){
		ccControlNames2.init(ctx.LANG, {
			"window.channel.cc.portamentotime",
			"window.channel.cc.modulation",
			"window.channel.cc.mod_rate",
			"window.channel.cc.mod_depth",
			"window.channel.cc.mod_delay",
			"window.channel.cc.volume",
			"window.channel.cc.pan",
			"window.channel.cc.pitchbend",
			"window.channel.cc.pitchbendrange",
			"window.channel.cc.expression",
							 });
	}
	static float * fVal=nullptr;
	static const float fMin[]={0, 0, 0, 0, 0, 0, -1, -1, 0, 0};
	static const float fMax[]={1, 1, 1, 1, 1, 1, 1, 1, 10, 1};
	for(u_index i=0;i < 10;i++){
		if(i == 0)fVal=&ch->getConfig().PortamentoTime;
		else if(i == 1)fVal=&ch->getConfig().Modulation;
		else if(i == 2)fVal=&ch->getConfig().ModRate;
		else if(i == 3)fVal=&ch->getConfig().ModDepth;
		else if(i == 4)fVal=&ch->getConfig().ModDelay;
		else if(i == 5)fVal=&ch->getConfig().Volume;
		else if(i == 6)fVal=&ch->getConfig().Pan;
		else if(i == 7)fVal=&ch->getConfig().ChannelPitchBend;
		else if(i == 8)fVal=&ch->getConfig().PitchBendRange;
		else if(i == 9)fVal=&ch->getConfig().Expression;
		ImGui::SliderFloat(ccControlNames2[i], fVal, fMin[i], fMax[i]);
	}
	ImGui::Text(ctx.LANG.getc("window.channel.dsp"));
	ImGui::Text(ctx.LANG.getc("window.channel.chorus"));
	u_sample_rate sampleRate=mixer.getSampleRate();
	static double def_min=0;
	static double def_max=1;
	static double depthMax=100;
	static double rateMax=50;
	static double feedBackMin=-1;
	static double feedBackMax=1;
	for(u_index i=0;i < 2;i++){
		Chorus & c=ch->getChorus(i);
		ImGui::PushID(i + 100);
		ImGui::SliderScalar(ctx.LANG.getc("window.channel.chorus.depth"), ImGuiDataType_Double, &c.depthMs, &def_min, &depthMax);
		ImGui::SliderScalar(ctx.LANG.getc("window.channel.chorus.freq"), ImGuiDataType_Double, &(std::dynamic_pointer_cast<Oscillator>(c.osc)->freq), &def_min, &rateMax);
		ImGui::SliderScalar(ctx.LANG.getc("window.channel.chorus.feedback"), ImGuiDataType_Double, &c.feedback, &feedBackMin, &feedBackMax);
		ImGui::SliderScalar(ctx.LANG.getc("window.channel.chorus.wet_ratio"), ImGuiDataType_Double, &c.wetRatio, &def_min, &def_max);
		ImGui::PopID();
		c.init(sampleRate);
	}
	ImGui::Text(ctx.LANG.getc("window.channel.reverb"));
	//static double rt60Max=10000;
	for(u_index i=0;i < 2;i++){
		Freeverb & c=ch->getReverb(i);
		ImGui::PushID(i + 200);
		ImGui::SliderScalar(ctx.LANG.getc("window.channel.reverb.damper"), ImGuiDataType_Double, &c.damper, &def_min, &def_max);
		ImGui::SliderScalar(ctx.LANG.getc("window.channel.reverb.room_size"), ImGuiDataType_Double, &c.roomSize, &def_min, &def_max);
		ImGui::SliderScalar(ctx.LANG.getc("window.channel.reverb.wet_ratio"), ImGuiDataType_Double, &c.wetRatio, &def_min, &def_max);
		ImGui::PopID();
		c.init(sampleRate);
	}
	ImGui::End();
}