#include "util/Lang.h"
#include "../SimpleSynthProject.h"
#include "../SimpleSynthWindow.h"
#include "interface/IChannel.h"
#include "interface/IMixer.h"
#include "dsp/DSP.h"
#include "interface/InstrumentProvider.h"
#include "interface/NoteProcessor.h"
#include "synth/composed/NonInterpolateAmpSet.h"
#include "util/MIDIFile.h"
#include "../NodeAutoLayout.h"

using namespace yzrilyzr_lang;
using namespace yzrilyzr_util;
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_array;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_collection;

extern MenuRegister allNoteProcessor;
extern MenuRegister allDSP;
extern MenuRegister allInterpolator;
extern MenuRegister allPhaseSrc;
extern MenuRegister allSubModule;

void buildProjectFromInstrument(CurrentProjectContext & ctx, u_sp<ClassRegister> np, int parType){
	ProjectObject * obj=new ProjectObject();
	obj->paramRegPtr=np;
	np->registerParams();
	MenuRegister::MenuRegisterObject * menuObj=nullptr;
	String clzName=np->getClassName();
	System::out.println(clzName);
	if(clzName == "BaseObject"){
		System::out.println();
	}
	if(parType == ParamType::NoteSrc || parType == ParamType::SampleData){
		for(auto & reg : allNoteProcessor.allRegObjects){
			if(reg.name == clzName){
				menuObj=&reg;
				break;
			}
		}
	} else if(parType == ParamType::DSP){
		for(auto & reg : allDSP.allRegObjects){
			if(reg.name == clzName){
				menuObj=&reg;
				break;
			}
		}
	} else if(parType == ParamType::Interpolator){
		for(auto & reg : allInterpolator.allRegObjects){
			if(reg.name == clzName){
				menuObj=&reg;
				break;
			}
		}
	} else if(parType == ParamType::PhaseSrc){
		for(auto & reg : allPhaseSrc.allRegObjects){
			if(reg.name == clzName){
				menuObj=&reg;
				break;
			}
		}
	}
	if(menuObj){
		obj->name=menuObj->name;
		obj->category=menuObj->category;
		obj->showName=menuObj->showName;
		obj->renderFunc=menuObj->rfunc;
		obj->enableOriginalRender=menuObj->enableOriginalRender;
	}
	ctx.objects.add(obj);
	for(auto & par : obj->paramRegPtr->RegisteredParams){
		if(!par.value)continue;
		if((par.type & 0xff) == ParamType::ObjectArray){
			ArrayList<u_sp<Object>> & arr=*static_cast<ArrayList<u_sp<Object>> *>(par.value);
			int subParType=(par.type >> 8) & 0xffffff;
			for(auto nptr : arr){
				if(nptr)buildProjectFromInstrument(ctx, spsc<ClassRegister>(nptr), subParType);
			}
			continue;
		}
		switch(par.type){
			case ParamType::DSP:
			case ParamType::NoteSrc:
			case ParamType::Interpolator:
			case ParamType::PhaseSrc:
			case ParamType::SampleData:
			{
				u_sp<ClassRegister> nptr=*static_cast<u_sp<ClassRegister> *>(par.value);
				if(nptr)buildProjectFromInstrument(ctx, spsc<ClassRegister>(nptr), par.type);
				break;
			}
		}
	}
}

void importFromBankWindow(CurrentProjectContext & ctx){
	IMixer & mixer=*ctx.mixer;
	//
	static LangToEnum programLang;
	if(programLang.empty(ctx.LANG)){
		std::vector<String> keys;
		for(int i=MIDIFile::Instruments::PIANO_ACOUSTIC_GRAND_PIANO;i <= MIDIFile::Instruments::EFFECTS_GUNSHOT;i++){
			keys.push_back(String("midi.program.") + i);
		}
		programLang.init(ctx.LANG, keys);
	}
	//
	static LangToEnum drumsetLang;
	if(drumsetLang.empty(ctx.LANG)){
		std::vector<String> keys;
		for(int i=MIDIFile::DrumSet::HIGH_Q;i <= MIDIFile::DrumSet::SURDO_OPEN;i++){
			keys.push_back(String("midi.drumset.") + i);
		}
		drumsetLang.init(ctx.LANG, keys);
	}
	//
	ImGui::Begin(ctx.LANG.getc("window.import_bank.title"));
	static s_program_id program=MIDIFile::Instruments::PIANO_ACOUSTIC_GRAND_PIANO;
	//ImGui::InputInt(ctx.LANG.getc("window.import_bank.program_num"), &program);
	ImGui::Combo(ctx.LANG.getc("window.import_bank.program_num"), &program, programLang.data(), programLang.size());
	//ImGui::SameLine();
	static NodeAutoLayout autoLayout;
	static int requestLayout=0;
	if(ImGui::Button(ctx.LANG.getc("window.import_bank.import_program"))){
		ctx.objects.clear();
		auto ptr=mixer.getGlobalConfig().instrument->get(0, program, mixer.getSampleRate());
		ChannelConfig cfg;
		cfg.sampleRate=mixer.getSampleRate();
		ptr->init(cfg);
		ctx.finalProcessor=ptr;
		if(ctx.finalProcessor){
			buildProjectFromInstrument(ctx, ctx.finalProcessor, ParamType::NoteSrc);
			requestLayout=1;
		}
	}
	static int drumset=MIDIFile::DrumSet::BASS_DRUM_ACOUSTIC;
	int drumsetOffset=drumset - MIDIFile::DrumSet::HIGH_Q;

	//ImGui::InputInt(ctx.LANG.getc("window.import_bank.drumset_num"), &drumset);
	if(ImGui::Combo(ctx.LANG.getc("window.import_bank.drumset_num"), &drumsetOffset, drumsetLang.data(), drumsetLang.size())){
		drumset=drumsetOffset + MIDIFile::DrumSet::HIGH_Q;
	}
	//ImGui::SameLine();
	drumset=Util::clamp(drumset, 0, 127);
	if(ImGui::Button(ctx.LANG.getc("window.import_bank.import_drumset"))){
		ctx.objects.clear();
		auto ptr=mixer.getGlobalConfig().instrument->getDrumSet(0, mixer.getSampleRate());
		ChannelConfig cfg;
		cfg.sampleRate=mixer.getSampleRate();
		ptr->init(cfg);
		if(auto set=U_INSTANCE_OF_PTR(NonInterpolateAmpSet, ptr)){
			ctx.finalProcessor=(*set).set[drumset];
		}
		if(ctx.finalProcessor){
			buildProjectFromInstrument(ctx, ctx.finalProcessor, ParamType::NoteSrc);
			requestLayout=1;
		}
	}
	ImGui::End();
	if(requestLayout != 0){
		requestLayout++;
		if(requestLayout >= 5){
			requestLayout=0;
			autoLayout.start(&ctx, NodeAutoLayout::SPRING);
		}
	}
	if(!autoLayout.finished()){
		autoLayout.doLayout();
	}
}