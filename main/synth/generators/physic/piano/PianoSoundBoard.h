#pragma once
#include "PianoSoundBoardParameters.h"
#include "SimpleSynth.h"
#include "dsp/DSP.h"
#include "dsp/BiquadIIR.h"
#include "dsp/FDNReverb2.h"

namespace yzrilyzr_simplesynth{
	ECLASS(PianoSoundBoard, public yzrilyzr_dsp::DSP){
	private:
	PianoSoundBoardParameters * param=nullptr;
	yzrilyzr_dsp::FDNReverb2 soundboard;
	yzrilyzr_dsp::BiquadIIR shaping1;
	yzrilyzr_dsp::BiquadIIR shaping2;
	yzrilyzr_dsp::BiquadIIR shaping3;
	yzrilyzr_dsp::BiquadIIR shaping4;
	yzrilyzr_dsp::BiquadIIR shaping5;
	public:
	~PianoSoundBoard();
	PianoSoundBoard();
	PianoSoundBoard(PianoSoundBoardParameters & param);
	u_sample procDsp(u_sample in) override;
	void init(u_sample_rate sampleRate) override;
	void resetMemory() override;
	void cloneParam(DSP * obj1)override;
	std::shared_ptr<DSP> cloneDSP() override;
	void setParam(PianoSoundBoardParameters & param);
	};
}
