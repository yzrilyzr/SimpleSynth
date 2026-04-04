#pragma once
#include "PianoSoundBoardParameters.h"
#include "SimpleSynth.h"
#include "dsp/DSP.h"
#include "dsp/IIR.h"
#include "dsp/FDNReverb2.h"

namespace yzrilyzr_simplesynth{
	ECLASS(PianoSoundBoard, public yzrilyzr_dsp::DSP){
	private:
	PianoSoundBoardParameters * param=nullptr;
	yzrilyzr_dsp::FDNReverb2 soundboard;
	yzrilyzr_dsp::IIR shaping1;
	yzrilyzr_dsp::IIR shaping2;
	yzrilyzr_dsp::IIR shaping3;
	yzrilyzr_dsp::IIR shaping4;
	yzrilyzr_dsp::IIR shaping5;
	public:
	~PianoSoundBoard();
	PianoSoundBoard();
	PianoSoundBoard(PianoSoundBoardParameters & param);
	u_sample procDsp(u_sample in) override;
	void procBlock(u_sample * input, u_sample * output, u_index length) override;
	void init(u_sample_rate sampleRate) override;
	void resetMemory() override;
	void cloneParam(yzrilyzr_dsp::DSP * obj1)override;
	yzrilyzr_dsp::DSPPtr newInstance() override;
	void setParam(PianoSoundBoardParameters & param);
	};
}
