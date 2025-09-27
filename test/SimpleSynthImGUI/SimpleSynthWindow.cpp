#include "LangToEnum.h"
#include "MenuRegister.hpp"
#include "MixerSequence.h"
#include "ProjectObject.h"
#include "SimpleSynth.h"
#include "SimpleSynthProject.h"
#include "SimpleSynthWindow.h"
#include "SynthUtil.h"
#include "array/SampleArray.h"
#include "dsp/Chorus.h"
#include "dsp/Freeverb.h"
#include "dsp/RMSCompute.h"
#include "events/ChannelEvent.h"
#include "imgui.h"
#include "instrument/ChipInstrument.h"
#include "instrument/DLSFormatInstrument.h"
#include "instrument/SF2FormatInstrument.h"
#include "instrument/SimpleMidiInstrument.h"
#include "interface/IChannel.h"
#include "interface/IMixer.h"
#include "interface/NoteTuning.h"
#include "io/FileInputStream.h"
#include "lang/Exception.h"
#include "lang/StringBuilder.h"
#include "tuning/EqualTemperament.h"
#include "tuning/JustIntonation.h"
#include "tuning/Kirnberger.h"
#include "tuning/Meantone.h"
#include "tuning/Pythagorean.h"
#include "tuning/Vallotti.h"
#include "tuning/Werckmeister.h"
#include "tuning/Young.h"
#include "util/ParamRegister.h"
#include "yzrutil.h"
#include <cfloat>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>

using namespace yzrilyzr_array;

using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
using namespace yzrilyzr_io;
using namespace yzrilyzr_collection;
using namespace yzrilyzr_lang;

void fileOpenWindow(CurrentProjectContext & ctx){
	IMixer & mixer=*ctx.mixer;
	ImGui::Begin(ctx.LANG.getc("window.play_file.title"));
	ImGui::Text(ctx.LANG.getc("window.play_file.hint"));
	static char text[1024 * 16]="";
	ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);
	if(ImGui::BeginDragDropTarget()){
		if(const ImGuiPayload * payload=ImGui::AcceptDragDropPayload("FILE_PATHS")){
			const char * filePath=(const char *)payload->Data;
			memcpy(text, filePath, strlen(filePath));
		}
	}
	if(ImGui::Button(ctx.LANG.getc("window.play_file.open"))){
		StringBuilder file(text);
		if(file.contains(".xm")){
			try{
				std::shared_ptr<MixerSequence> seq=SynthUtil::parseXM(FileInputStream(file.tostring()));
				seq->postToMixer(&mixer, 0, "IMGUI_XM");
			} catch(Exception e){
				std::cout << e.what() << std::endl;
			}
		} else if(file.contains(".mid")){
			try{
				std::shared_ptr<MixerSequence> seq=SynthUtil::parseMIDI(FileInputStream(file.tostring()));
				seq->postToMixer(&mixer, 0, "IMGUI_MID");
			} catch(Exception e){
				std::cout << e.what() << std::endl;
			}
		}
	}
	ImGui::End();
}

void mainMenuBar(const char * name, MenuRegister & np, CurrentProjectContext & ctx){
	if(ImGui::BeginMenu(name)){
		for(const auto & category : np.regObjects){
			const char * fir=category.first.c_str();
			bool a=strcmp(fir, "") == 0;
			if(a || ImGui::BeginMenu((np.categoryToShowCategory[category.first] + "##").c_str())){
				for(const auto & entry : category.second){
					if(ImGui::MenuItem(entry.showName.c_str())){
						ProjectObject * proj=new ProjectObject();
						proj->name=entry.name;
						proj->showName=entry.showName;
						proj->category=entry.category;
						proj->paramRegPtr=std::static_pointer_cast<ParamRegister>(entry.cfunc());
						proj->renderFunc=entry.rfunc;
						proj->enableOriginalRender=entry.enableOriginalRender;
						ctx.objects.add(proj);
					}
				}
				if(!a)ImGui::EndMenu();
			}
		}
		ImGui::EndMenu();
	}
}

void instrumentSourceWindow(CurrentProjectContext & ctx){
	IMixer & mixer=*ctx.mixer;
	static File folderPath("instrument");
	static std::vector<File> fileList;
	static bool initRefresh=true;
	static int select=-1;
	ImGui::Begin(ctx.LANG.getc("window.instrument.title"));
	if(ImGui::Button(ctx.LANG.getc("window.instrument.refresh")) || initRefresh){
		initRefresh=false;
		fileList=folderPath.listFiles();
		fileList.emplace_back(File(ctx.LANG.getc("window.instrument.default")));
		fileList.emplace_back(File(ctx.LANG.getc("window.instrument.chip")));
	}
	if(ImGui::BeginListBox(ctx.LANG.getc("window.instrument.list_title"))){
		int i=0;
		for(const auto & file : fileList){
			if(ImGui::Selectable(file.getName().c_str(), select == i)){
				select=i;
				File & f=fileList[i];
				StringBuilder n=f.getName();
				if(n == ctx.LANG.get("window.instrument.default")){
					mixer.setInstrumentProvider(std::make_shared<SimpleMidiInstrument>());
				} else if(n == ctx.LANG.get("window.instrument.chip")){
					mixer.setInstrumentProvider(std::make_shared<ChipInstrument>());
				} else if(n.endsWith(".sf2")){
					mixer.setInstrumentProvider(std::make_shared<SF2FormatInstrument>(FileInputStream(f)));
				} else if(n.endsWith(".dls")){
					mixer.setInstrumentProvider(std::make_shared<DLSFormatInstrument>(FileInputStream(f)));
				}
			}
			i++;
		}
		ImGui::EndListBox();
	}
	ImGui::End();
}
void objectToStringWindow(CurrentProjectContext & ctx){
	ImGui::Begin(ctx.LANG.getc("window.objtostring.title"));
	static char text[1048576]="";
	ImGui::InputTextMultiline("##tostring", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);
	if(ctx.finalProcessor){
		std::string string=ctx.finalProcessor->toString();
		strcpy(text, string.c_str());
	}
	ImGui::End();
}