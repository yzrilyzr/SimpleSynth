#pragma once
#include "SimpleSynth.h"
#include "interface/NoteTuning.h"

namespace yzrilyzr_simplesynth{
	// 纯律 - 基于自然泛音列的简单整数比
	ECLASS(JustIntonation, public NoteTuning){
	private:
	// C大调的12个音级与主音的频率比
	// 包含了F#的比值(7/5)
	static constexpr double ratios[12]={
		1.0,        // C
		16.0 / 15.0,  // C#/Db
		9.0 / 8.0,    // D
		6.0 / 5.0,    // D#/Eb
		5.0 / 4.0,    // E
		4.0 / 3.0,    // F
		7.0 / 5.0,    // F#/Gb
		3.0 / 2.0,    // G
		8.0 / 5.0,    // G#/Ab
		5.0 / 3.0,    // A
		16.0 / 9.0,   // A#/Bb
		15.0 / 8.0    // B
	};
	public:
	u_freq getFrequency(s_note_id id) override{
		// 计算八度和音级
		int octave=static_cast<int>(id) / 12;
		int degree=static_cast<int>(id) % 12;
		if(degree < 0) degree+=12; // 处理负向音符

		// 频率 = 基频 * 2^八度 * 音级比率
		return baseFreq * pow(2.0, octave) * ratios[degree];
	}

	s_note_id getIDByFrequency(u_freq frequency) override{
		// 找到最接近的八度
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

	JustIntonation(){
		setBaseFreqByA(440.0); // 默认A4=440Hz
	}

	// 根据A4频率设置基频(C0)
	void setBaseFreqByA(u_freq frequency) override{
		// A4对应音符ID 69
		setBaseFreq(frequency / (pow(2.0, 5) * ratios[9]));
	}
	};
}
