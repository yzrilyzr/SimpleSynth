#pragma once
#include "SimpleSynth.h"
#include "interface/NoteTuning.h"

namespace yzrilyzr_simplesynth{
	// Werckmeister律 - 一种流行的Well-Tempered律，这里实现Werckmeister III
	ECLASS(Werckmeister, public NoteTuning){
	private:
		// Werckmeister III的12个音级比率
	static constexpr double ratios[12]={
		1.0,           // C
		1.059463,      // C#/Db
		1.122462,      // D
		1.189207,      // D#/Eb
		1.259921,      // E
		1.334840,      // F
		1.414213,      // F#/Gb
		1.498307,      // G
		1.587401,      // G#/Ab
		1.681793,      // A
		1.781797,      // A#/Bb
		1.887749       // B
	};
	public:
	u_freq getFrequency(s_note_id id) override{
		int octave=static_cast<int>(id) / 12;
		int degree=static_cast<int>(id) % 12;
		if(degree < 0) degree+=12;

		return baseFreq * pow(2.0, octave) * ratios[degree];
	}

	s_note_id getIDByFrequency(u_freq frequency) override{
		double relative=frequency / baseFreq;
		if(relative <= 0) return 0;

		int octave=static_cast<int>(log2(relative));
		double withinOctave=relative / pow(2.0, octave);

		// 找到最接近的音级
		double minDiff=1e9;
		int bestDegree=0;
		for(int i=0; i < 12; i++){
			double diff=fabs(withinOctave - ratios[i]);
			if(diff < minDiff){
				minDiff=diff;
				bestDegree=i;
			}
		}

		return octave * 12 + bestDegree;
	}

	Werckmeister(){
		setBaseFreqByA(440.0); // 默认A4=440Hz
	}

	void setBaseFreqByA(u_freq frequency) override{
		// A4对应音符ID 69
		setBaseFreq(frequency / (pow(2.0, 5) * ratios[9]));
	}
	};
}
