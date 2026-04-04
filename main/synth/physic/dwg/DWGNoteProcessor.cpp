#include "DWGNoteProcessor.h"
#include "events/ChannelConfig.h"
#include "DigitalWaveGuide.h"
#include "interface/PhaseSrc.h"
#include "synth/util/AmplitudeSources.h"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_collection;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_util;

namespace yzrilyzr_simplesynth{
	void DWGNoteProcessor::onRegisterParam(){
		RegisterUtil::registerParamSrc(*this, "Load", &load);
		RegisterUtil::registerParamSrc(*this, "Left", &left);
		RegisterUtil::registerParamSrc(*this, "Right", &right);
		static u_sample Z_min=0.001, Z_max=99;
		static u_sample damper_min=0, damper_max=1;
		registerParam("Z", ParamType::Sample, &Z, &Z_min, &Z_max);
		registerParam("Damper", ParamType::Sample, &damper, &damper_min, &damper_max);
		RegisterUtil::registerParamPhaseSrc(*this, "Delay1", &delayFreq1);
		RegisterUtil::registerParamPhaseSrc(*this, "Delay2", &delayFreq2);
		static int atMin=0, atMax=1;
		registerParam("LoadAt", ParamType::Int, &loadAt, &atMin, &atMax);
		registerParam("SignalAt", ParamType::Int, &signalAt, &atMin, &atMax);
		RegisterUtil::registerParamDSP(*this, "LowPass", &lowpass);
		registerParamArray("Dispersion", ParamType::DSP, &dispersion);
		RegisterUtil::registerParamPhaseSrc(*this, "FracDelay", &fracDelay);
	}
	void DWGNoteProcessor::init(ChannelConfig & cfg){
		if(delayFreq1 == nullptr)delayFreq1=NotePhase;
		if(delayFreq2 == nullptr)delayFreq2=NotePhase;
		if(fracDelay == nullptr)fracDelay=NotePhase;
		if(load)load->init(cfg);
		if(left != nullptr){
			if(left.get() == this)throw IllegalArgumentException("left == this");
			if(!U_INSTANCE_OF_PTR(DWGNoteProcessor, left))throw IllegalArgumentException("left != DWGNoteProcessor");
			left->init(cfg);
		}
		if(right != nullptr){
			if(right.get() == this)throw IllegalArgumentException("left == this");
			if(!U_INSTANCE_OF_PTR(DWGNoteProcessor, right))throw IllegalArgumentException("right != DWGNoteProcessor");
			right->init(cfg);
		}
	}
	u_sample DWGNoteProcessor::getAmp(const Note & note){
		if(left)left->getAmp(note);
		if(right)right->getAmp(note);
		u_sample loadVal=load != nullptr?load->getAmp(note):0;
		auto & dwg=*getData(note);
		if(loadAt == 0)dwg.leftNode.load=loadVal;
		else dwg.rightNode.load=loadVal;
		dwg.processDelay();
		dwg.calculateLoad();
		dwg.updateSignals();
		if(signalAt == 0)return dwg.leftNode.signals[1];
		else return dwg.rightNode.signals[0];
	}
	DigitalWaveGuide * DWGNoteProcessor::init(DigitalWaveGuide * data, const Note & note){
		if(data == nullptr)data=new DigitalWaveGuide();
		auto & dwg=*data;
		u_sample_rate sampleRate=note.cfg->sampleRate;
		dwg.resetMemory();
		dwg.resetConnection();
		u_freq f1=delayFreq1->getFreq(note);
		u_freq f2=delayFreq2->getFreq(note);
		dwg.init(Z, f1 == 0?0:sampleRate / f1, f2 == 0?0:sampleRate / f2);
		if(left != nullptr){
			u_sp< DWGNoteProcessor> another=spsc< DWGNoteProcessor>(left);
			auto & anotherDwg=*another->getData(note);
			if(another->signalAt == 0){
				DigitalWaveGuide::connectLeftLeft(dwg, anotherDwg);
			} else{
				DigitalWaveGuide::connectLeftRight(anotherDwg, dwg);
			}
			anotherDwg.initAlphaCoefficients();
		}
		if(right != nullptr){
			u_sp< DWGNoteProcessor> another=spsc< DWGNoteProcessor>(right);
			auto & anotherDwg=*another->getData(note);
			if(another->signalAt == 0){
				DigitalWaveGuide::connectLeftRight(dwg, anotherDwg);
			} else{
				DigitalWaveGuide::connectRightRight(anotherDwg, dwg);
			}
			anotherDwg.initAlphaCoefficients();
		}
		if(lowpass){
			ArrayList<DSPPtr> disp;
			for(auto d : dispersion){
				disp.add(d->clone());
			}
			dwg.setDispersion(disp, lowpass->clone(), fracDelay->getFreq(note));
		}
		dwg.initAlphaCoefficients();
		return data;
	}

	String  DWGNoteProcessor::toString()const{
		return "DWGNoteProcessor";
	}
}