#include "../LangToEnum.h"
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
		std::shared_ptr<NoteTuning> t=nullptr;
		if(tuningType == 0)t=std::make_shared<EqualTemperament>();
		else if(tuningType == 1)t=std::make_shared<JustIntonation>();
		else if(tuningType == 2)t=std::make_shared<Pythagorean>();
		else if(tuningType == 3)t=std::make_shared<Meantone>();
		else if(tuningType == 4)t=std::make_shared<Werckmeister>();
		else if(tuningType == 5)t=std::make_shared<Kirnberger>();
		else if(tuningType == 6)t=std::make_shared<Vallotti>();
		else if(tuningType == 7)t=std::make_shared<Young>();
		mixer.getGlobalConfig().setNoteTuning(t);
		for(auto channel : mixer.getAllChannels()){
			TuningChange * event=new TuningChange();
			event->value=t;
			event->channelID=channel->getChannelID();
			event->groupName=channel->getGroupName();
			mixer.sendInstantEvent(event);
		}
	}
	ImGui::End();
}