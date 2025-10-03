//------------------------------------------------------------------------
// Copyright(c) 2025 yzrilyzr.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "util/Lang.h"
#include "SimpleSynthEditorDelegate.h"

namespace yzrilyzr_simplesynth_vst{
	enum SimpleSynthParams : Steinberg::Vst::ParamID{
		kParamSustain=100,  // Sustain 开关参数
		kParamProgram=101,  // MIDI 乐器号
		kParamExpression=102,  // MIDI 表情
		kParamIsDrum=103,  //是否鼓组
		kParamVolume=104,  // 音量
		kParamLimiter=105,  // 限制器
		kParamPan=106,
		kParamChorus=107,
		kParamReverb=108,
		kParamPortamento=109,
		kParamMod=110,
		kParamPitchBend=111,
		kParamSoftPedal=112,
		kParamLoadTime=113,
		kParamSostenuto=114,
		kParamPhaser=115,
		kProgramSelectorBaseTag=10000
	};

//------------------------------------------------------------------------
//  SimpleSynthController
//------------------------------------------------------------------------
	class SimpleSynthController : public Steinberg::Vst::EditControllerEx1{
		public:
		//------------------------------------------------------------------------
		SimpleSynthController()=default;
		~SimpleSynthController() SMTG_OVERRIDE=default;

		// Create function
		static Steinberg::FUnknown * createInstance(void * /*context*/){
			return (Steinberg::Vst::IEditController *)new SimpleSynthController;
		}

		//--- from IPluginBase -----------------------------------------------
		Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown * context) SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

		//--- from EditController --------------------------------------------
		Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream * state) SMTG_OVERRIDE;
		Steinberg::IPlugView * PLUGIN_API createView(Steinberg::FIDString name) SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream * state) SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream * state) SMTG_OVERRIDE;
		//---Interface---------
		DEFINE_INTERFACES
			// Here you can add more supported VST3 interfaces
			// DEF_INTERFACE (Vst::IXXX)
			END_DEFINE_INTERFACES(EditController)
			DELEGATE_REFCOUNT(EditController)

		//------------------------------------------------------------------------
		protected:
		yzrilyzr_util::Lang LANG;
		std::unique_ptr<SimpleSynthEditorDelegate> editorDelegate;
	};

	//------------------------------------------------------------------------
} // namespace yzrilyzr_simplesynth_vst
