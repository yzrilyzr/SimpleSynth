#pragma once
#include "SimpleSynth.h"
#include "interface/NoteTuning.h"

namespace yzrilyzr_simplesynth{
	// 毕达哥拉斯律 - 基于纯五度(3/2)的五度相生律
	ECLASS(Pythagorean, public NoteTuning){
	private:
	// 毕达哥拉斯律的12个音级与主音的频率比
	// 全部基于3/2的幂次并调整到一个八度内
	static constexpr double ratios[12]={
		1.0,           // C
		256.0 / 243.0,   // C#/Db
		9.0 / 8.0,       // D
		32.0 / 27.0,     // D#/Eb
		81.0 / 64.0,     // E
		4.0 / 3.0,       // F
		729.0 / 512.0,   // F#/Gb
		3.0 / 2.0,       // G
		128.0 / 81.0,    // G#/Ab
		27.0 / 16.0,     // A
		16.0 / 9.0,      // A#/Bb
		243.0 / 128.0    // B
	};
	public:
	u_freq getFrequency(s_note_id id) override{
		// 计算八度和音级
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
		for(u_index i=0; i < 12; i++){
			double diff=fabs(withinOctave - ratios[i]);
			if(diff < minDiff){
				minDiff=diff;
				bestDegree=i;
			}
		}

		return octave * 12 + bestDegree;
	}

	Pythagorean(){
		setBaseFreqByA(440.0); // 默认A4=440Hz
	}

	// 根据A4频率设置基频(C0)
	void setBaseFreqByA(u_freq frequency) override{
		// A4对应音符ID 69
		setBaseFreq(frequency / (pow(2.0, 5) * ratios[9]));
	}
	};
}
