//------------------------------------------------------------------------
// Copyright(c) 2025 yzrilyzr.
//------------------------------------------------------------------------

#include "controller.h"
#include "cids.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "pluginterfaces/base/ibstream.h"
#include "vstgui/uidescription/cstream.h"
#include "vstgui/uidescription/uicontentprovider.h"
#include "pluginterfaces/base/futils.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/lib/platform/iplatformresourceinputstream.h"
#include "io/File.h"
#include "io/ByteArrayOutputStream.h"
#include "lang/String.h"
#include "util/Locale.h"


using namespace VSTGUI;
using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace yzrilyzr_io;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_util;

namespace yzrilyzr_simplesynth_vst{
//------------------------------------------------------------------------
// SimpleSynthController Implementation
//------------------------------------------------------------------------
	yzrilyzr_lang::String readStreamToString(CResourceInputStream & stream){
		yzrilyzr_io::ByteArrayOutputStream content;
		yzrilyzr_array::ByteArray buffer(4096);
		uint32_t bytesRead=0;
		do{
			bytesRead=stream.readRaw(buffer._array, buffer.length - 1);
			if(bytesRead > 0){				
				content.write(buffer, 0, bytesRead);				
			}
		} while(bytesRead == buffer.length - 1);
		return content.toString(UTF8);
	}
	tresult PLUGIN_API SimpleSynthController::initialize(FUnknown * context){
		// Here the Plug-in will be instantiated

		//---do not forget to call parent ------
		tresult result=EditControllerEx1::initialize(context);
		if(result != kResultOk){
			return result;
		}
		CResourceInputStream resInputStream;
		if(resInputStream.open("lang.properties")){
			LANG.add(std::make_shared<Properties>(readStreamToString(resInputStream)));
		}		
		editorDelegate=std::make_unique<SimpleSynthEditorDelegate>(this, LANG);
		// Here you could register some parameters
		parameters.addParameter(LANG.getc16("param.channel_sustain"), nullptr, 1, 0, ParameterInfo::kCanAutomate, kParamSustain);
		parameters.addParameter(LANG.getc16("param.expression"), nullptr, 0, 1, ParameterInfo::kCanAutomate, kParamExpression);
		parameters.addParameter(LANG.getc16("param.is_drum"), nullptr, 1, 0, ParameterInfo::kCanAutomate, kParamIsDrum );
		parameters.addParameter(LANG.getc16("param.volume"), nullptr, 0, 0.707, ParameterInfo::kCanAutomate, kParamVolume );
		parameters.addParameter(LANG.getc16("param.limiter"), nullptr, 1, 0, ParameterInfo::kCanAutomate, kParamLimiter);
		parameters.addParameter(LANG.getc16("param.pan"), nullptr, 0, 0.5, ParameterInfo::kCanAutomate, kParamPan);
		parameters.addParameter(LANG.getc16("param.chorus"), nullptr, 0, 1, ParameterInfo::kCanAutomate, kParamChorus);
		parameters.addParameter(LANG.getc16("param.reverb"), nullptr, 0, 0, ParameterInfo::kCanAutomate, kParamReverb);
		parameters.addParameter(LANG.getc16("param.portamento"), nullptr, 0, 0, ParameterInfo::kCanAutomate, kParamPortamento);
		parameters.addParameter(LANG.getc16("param.mod"), nullptr, 0, 0, ParameterInfo::kCanAutomate, kParamMod);
		parameters.addParameter(LANG.getc16("param.pitchbend"), nullptr, 0, 0.5, ParameterInfo::kCanAutomate, kParamPitchBend);
		parameters.addParameter(LANG.getc16("param.soft_pedal"), nullptr, 0, 0, ParameterInfo::kCanAutomate, kParamSoftPedal);
		parameters.addParameter(LANG.getc16("param.load_time"), nullptr, 0, 0,0, kParamLoadTime);
		parameters.addParameter(LANG.getc16("param.sostenuto"), nullptr, 1, 0, ParameterInfo::kCanAutomate, kParamSostenuto);
		parameters.addParameter(LANG.getc16("param.phaser"), nullptr, 0, 0, ParameterInfo::kCanAutomate, kParamPhaser);

		StringListParameter * programParam=new StringListParameter(LANG.getc16("param.midi_program"), kParamProgram, nullptr, ParameterInfo::kCanAutomate | ParameterInfo::kIsList);
		for(int32_t i=0; i < 128; ++i){
			const TChar * displayName=LANG.getc16(yzrilyzr_lang::String("midi.program.")+i);
			programParam->appendString(displayName);
		}
		parameters.addParameter(programParam);
		return result;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API SimpleSynthController::terminate(){
		// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

		//---do not forget to call parent ------
		return EditControllerEx1::terminate();
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API SimpleSynthController::setComponentState(IBStream * state){
		// Here you get the state of the component (Processor part)
		if(!state)
			return kResultFalse;
		return kResultOk;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API SimpleSynthController::setState(IBStream * state){
		// Here you get the state of the controller

		return kResultTrue;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API SimpleSynthController::getState(IBStream * state){
		// Here you are asked to deliver the state of the controller (if needed)
		// Note: the real state of your plug-in is saved in the processor

		return kResultTrue;
	}

	//------------------------------------------------------------------------
	IPlugView * PLUGIN_API SimpleSynthController::createView(FIDString name){
		// Here the Host wants to open your editor (if you have one)
		if(FIDStringsEqual(name, ViewType::kEditor)){
			// create your editor here and return a IPlugView ptr of it
			auto * view=new VSTGUI::VST3Editor(this, "view", "editor.uidesc");
			view->setDelegate(editorDelegate.get());
			return view;
		}
		return nullptr;
	}
	//------------------------------------------------------------------------
} // namespace yzrilyzr_simplesynth_vst