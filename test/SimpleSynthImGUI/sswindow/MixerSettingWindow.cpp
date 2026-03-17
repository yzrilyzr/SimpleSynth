#include "util/Lang.h"
#include "../SimpleSynthProject.h"
#include "../SimpleSynthWindow.h"
#include "interface/IChannel.h"
#include "interface/IMixer.h"
#include "SynthUtil.h"
#include "array/Array.hpp"
#include "dsp/Chorus.h"
#include "dsp/Freeverb.h"
#include "dsp/RMSCompute.h"
#include "tuning/EqualTemperament.h"
#include "tuning/JustIntonation.h"
#include "tuning/Kirnberger.h"
#include "tuning/Meantone.h"
#include "interface/NoteTuning.h"
#include "tuning/Pythagorean.h"
#include "tuning/Vallotti.h"
#include "tuning/Werckmeister.h"
#include "tuning/Young.h"
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_array;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_util;
void mixerSettingWindow(CurrentProjectContext & ctx){
	ImGui::Begin(ctx.LANG.getc("window.mixer.title"));
	IMixer & mixer=*ctx.mixer;
	ImGui::Text(ctx.LANG.getf("window.mixer.queue_info", mixer.getCurrentProcessingNoteCount(), mixer.getPostedEventCount()).c_str(UTF8));
	ImGui::ProgressBar(ctx.processTime / mixer.getProcessStandardTime());
	static bool limiter=true;
	ImGui::Checkbox(ctx.LANG.getc("window.mixer.limiter"), &limiter);
	mixer.setUseLimiter(limiter);
	if(ImGui::Button(ctx.LANG.getc("window.mixer.reset"))){
		mixer.reset();
	}
	static int tuningType=0;
	static LangToEnum tuningNames;
	if(tuningNames.empty(ctx.LANG)){
		tuningNames.init(ctx.LANG, {
			"tuning.equal_temperament",
			"tuning.just_intonation",
			"tuning.pythagorean",
			"tuning.meantone",
			"tuning.werckmeister",
			"tuning.kirnberger",
			"tuning.vallotti",
			"tuning.young"
						 });
	}
	if(ImGui::Combo(ctx.LANG.getc("window.mixer.tuning"), &tuningType, tuningNames.data(), tuningNames.size())){
		//std::unique_lock<std::shared_mutex> lock(ctx.mixerLock);
		u_sp<NoteTuning> t=nullptr;
		if(tuningType == 0)t=mksp<EqualTemperament>();
		else if(tuningType == 1)t=mksp<JustIntonation>();
		else if(tuningType == 2)t=mksp<Pythagorean>();
		else if(tuningType == 3)t=mksp<Meantone>();
		else if(tuningType == 4)t=mksp<Werckmeister>();
		else if(tuningType == 5)t=mksp<Kirnberger>();
		else if(tuningType == 6)t=mksp<Vallotti>();
		else if(tuningType == 7)t=mksp<Young>();
		mixer.getGlobalConfig().setNoteTuning(t);
		for(auto channel : mixer.getAllChannels()){
			auto event=mkup < TuningChange>(t);
			event->channelID=channel->getChannelID();
			event->groupName=channel->getGroupName();
			mixer.sendInstantEvent(std::move(event));
		}
	}
	ImGui::End();
}