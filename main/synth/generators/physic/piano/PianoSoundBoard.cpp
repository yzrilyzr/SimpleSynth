#include "PianoSoundBoard.h"
#include "dsp/IIRUtil.h"
#include "dsp/FilterPassType.h"

using namespace yzrilyzr_dsp;
namespace yzrilyzr_simplesynth{
	PianoSoundBoard::~PianoSoundBoard(){}
	PianoSoundBoard::PianoSoundBoard(){}
	PianoSoundBoard::PianoSoundBoard(PianoSoundBoardParameters & param){
		setParam(param);
	}
	void PianoSoundBoard::setParam(PianoSoundBoardParameters & param){
		this->param=&param;
	}
	u_sample PianoSoundBoard::procDsp(u_sample in){
		u_sample signal=soundboard.procDsp(in);
		signal+=shaping1.procDsp(in);
		signal+=shaping2.procDsp(in);
		signal+=shaping3.procDsp(in);
		signal/=4.0;
		signal=shaping4.procDsp(signal);
		signal=shaping5.procDsp(signal);
		return signal * 0.9 + in * 0.1;
	}
	void PianoSoundBoard::init(u_sample_rate sampleRate){
		soundboard.lowpassCoeff=param->c1;
		soundboard.highpassCoeff=param->c3;
		soundboard.feedbackGain=-0.25;
		soundboard.wetRatio=1;
		soundboard.init(sampleRate);
		IIRUtil::biquad(shaping1, param->eq1, sampleRate, 1.5, FilterPassType::LOWSHELF, 4.0);
		IIRUtil::biquad(shaping2, param->eq2, sampleRate, 2.0, FilterPassType::BELL, 5.0);
		IIRUtil::biquad(shaping3, param->eq3, sampleRate, 1.8, FilterPassType::BELL, 3.0);
		IIRUtil::biquad(shaping4, param->eq4, sampleRate, 0.707, FilterPassType::HIGHSHELF, -3.0);
		IIRUtil::biquad(shaping5, param->eq5, sampleRate, 0.5, FilterPassType::LOWPASS);
	}
	void PianoSoundBoard::resetMemory(){
		shaping1.resetMemory();
		shaping2.resetMemory();
		shaping3.resetMemory();
		shaping4.resetMemory();
		shaping5.resetMemory();
		soundboard.resetMemory();
	}
	DSPPtr PianoSoundBoard::cloneDSP(){
		return mksp< PianoSoundBoard>(*param);
	}
	void PianoSoundBoard::cloneParam(DSP * obj1){}
}