#include "TwoStringResonatorExciter.h"
#include "dsp/BiquadIIR.h"
#include "dsp/BufferDelayer.h"
#include "dsp/IIRUtil.h"
#include "util/Util.h"
#include "util/ClassRegister.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
namespace yzrilyzr_simplesynth{
	void TwoStringResonatorExciter::onRegisterParam(){
		Osc::onRegisterParam();
		static float min=0, max=1;
		registerParam("NoiseMix", ParamType::Float, &noiseMixRatio, &min, &max);
		registerParam("NoiseRate", ParamType::Float, &noiseRate, &min, &max);
	}
	TwoStringResonatorExciter::TwoStringResonatorExciter() :Osc(nullptr){}
	u_sample TwoStringResonatorExciter::exciteClickFunc(s_phase mod){
		if(mod > 1)return 0;
		mod=RingBufferUtil::mod1(mod);
		if(mod < 0.18946)return 1.0 - 3.0 * mod;
		return pow(mod - 1.0, 4.0);
	}

	void TwoStringResonatorExciter::init(ChannelConfig & cfg){
		
	}
	NoteProcPtr TwoStringResonatorExciter::clone(){
		return mksp<TwoStringResonatorExciter>();
	}
	u_sample TwoStringResonatorExciter::getAmp(const Note & note){
		TwoStringResonatorExciterKeyData * data=getData(note);
		u_sample sumExcite=0;
		sumExcite=exciteClickFunc(note.phaseSynth);
		u_sample_rate sampleRate=note.cfg->sampleRate;
		data->noiseRateCounter+=noiseRate * sampleRate;
		if(data->noiseRateCounter > sampleRate){
			data->noiseRateCounter=0;
			data->lastNoiseValue=random.next();
		}
		sumExcite+=(u_sample)noiseMixRatio * data->lastNoiseValue;
		return sumExcite * note.velocitySynth;
	}

	TwoStringResonatorExciterKeyData * TwoStringResonatorExciter::init(TwoStringResonatorExciterKeyData * data, const Note & note){
		if(data == nullptr){
			data=new TwoStringResonatorExciterKeyData();
		}
		data->noiseRateCounter=0;
		data->lastNoiseValue=0;

		return data;
	}
}