#include "PianoSoundBoard.h"
#include "dsp/IIRUtil.h"
#include "dsp/FilterPassType.h"
#include "array/Array.hpp"

using namespace yzrilyzr_dsp;
using namespace yzrilyzr_array;
namespace yzrilyzr_simplesynth{
	PianoSoundBoard::~PianoSoundBoard(){}
	PianoSoundBoard::PianoSoundBoard(){}
	PianoSoundBoard::PianoSoundBoard(PianoSoundBoardParameters & param){
		setParam(param);
	}
	void PianoSoundBoard::setParam(PianoSoundBoardParameters & param){
		this->param=&param;
	}
	void PianoSoundBoard::procBlock(u_sample * input, u_sample * output, u_index length){
		static thread_local SampleArray temp1;
		if(temp1 == nullptr || temp1.length < length * 2)temp1=SampleArray(length * 2);
		u_sample * data=temp1.data();
		soundboard.procBlock(input, data, length);
		shaping1.procBlock(input, data + length, length);
		for(size_t i=0; i < length; i++){
			data[i]+=data[i + length];
		}
		shaping2.procBlock(input, data + length, length);
		for(size_t i=0; i < length; i++){
			data[i]+=data[i + length];
		}
		shaping3.procBlock(input, data + length, length);
		for(size_t i=0; i < length; i++){
			data[i]+=data[i + length];
			data[i]/=4.0;
		}
		shaping4.procBlock(data, data, length);
		shaping5.procBlock(data, data, length);
		for(size_t i=0; i < length; i++){
			input[i]=data[i] * 0.9 + input[i] * 0.1;
		}
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
		IIRUtil::RBJ_biquad(shaping1, RBJParams{FilterPassType::LOWSHELF, param->eq1, sampleRate, 1.5, 4.0});
		IIRUtil::RBJ_biquad(shaping2, RBJParams{FilterPassType::BELL, param->eq2, sampleRate, 2.0, 5.0});
		IIRUtil::RBJ_biquad(shaping3, RBJParams{FilterPassType::BELL, param->eq3, sampleRate, 1.8, 3.0});
		IIRUtil::RBJ_biquad(shaping4, RBJParams{FilterPassType::HIGHSHELF, param->eq4, sampleRate, 0.707, -3.0});
		IIRUtil::RBJ_biquad(shaping5, RBJParams{FilterPassType::LOWPASS, param->eq5, sampleRate, 0.5, 0.0});
		shaping1.init(sampleRate);
		shaping2.init(sampleRate);
		shaping3.init(sampleRate);
		shaping4.init(sampleRate);
		shaping5.init(sampleRate);
	}
	void PianoSoundBoard::resetMemory(){
		shaping1.resetMemory();
		shaping2.resetMemory();
		shaping3.resetMemory();
		shaping4.resetMemory();
		shaping5.resetMemory();
		soundboard.resetMemory();
	}
	DSPPtr PianoSoundBoard::newInstance(){
		return mksp< PianoSoundBoard>(*param);
	}
	void PianoSoundBoard::cloneParam(DSP * obj1){}
}