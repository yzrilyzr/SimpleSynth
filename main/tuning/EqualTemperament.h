#pragma once
#include "SimpleSynth.h"
#include "interface/NoteTuning.h"

namespace yzrilyzr_simplesynth{
	ECLASS(EqualTemperament, public NoteTuning){
	private:
	float divider;//平均律分母
	public:
	u_freq getFrequency(s_note_id id) override{
		return baseFreq * pow(2.0, static_cast<double>(id) / divider);
	}
	s_note_id getIDByFrequency(u_freq frequency) override{
		return divider * log(frequency / baseFreq) / log(2);
	}
	EqualTemperament() : EqualTemperament(8.175798915643707333, 12){}
	EqualTemperament(u_freq baseFreq, float divider) :  divider(divider){
		setBaseFreq(baseFreq);
	}
	/**
	 * 根据A4的频率设置基频
	 * <br>e.g. A=440Hz, BaseFreq=8.175798915643707333
	 */
	void setBaseFreqByA(u_freq frequency)override{
		setBaseFreq(frequency / pow(2.0, 69.0 / divider));
	}
	float getDivider(){
		return divider;
	}
	void setDivider(float d){
		divider=d;
		generateNoteFrequencyTable();
	}
	};
}