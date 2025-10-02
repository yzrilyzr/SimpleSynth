#pragma once
#include "interface/NoteTuning.h"
#include "SimpleSynth.h"
#include <cmath>

namespace yzrilyzr_simplesynth{
	// 中庸全音律 - 调整五度使其更接近纯律三度
	ECLASS(Meantone, public NoteTuning){
	private:
		// 四分之一逗点中庸全律的五度缩减量
	static constexpr double fifthRatio=1.4953487812212205; // 3^(1/4)/2^(7/12)
	// 计算12个音级的比率
	double calculateRatio(int degree) const{
		// 每个音级基于C通过五度相生计算
		static const int fifths[12]={0, -5, 2, -3, 4, -1, 6, 1, -4, 3, -2, 5};
		double ratio=pow(fifthRatio, fifths[degree]);

		// 调整到一个八度内
		while(ratio >= 2.0) ratio/=2.0;
		while(ratio < 1.0) ratio*=2.0;
		return ratio;
	}
	public:
	u_freq getFrequency(s_note_id id) override{
		int octave=static_cast<int>(id) / 12;
		int degree=static_cast<int>(id) % 12;
		if(degree < 0) degree+=12;

		return baseFreq * pow(2.0, octave) * calculateRatio(degree);
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
			double ratio=calculateRatio(i);
			double diff=fabs(withinOctave - ratio);
			if(diff < minDiff){
				minDiff=diff;
				bestDegree=i;
			}
		}

		return octave * 12 + bestDegree;
	}

	Meantone(){
		setBaseFreqByA(440.0); // 默认A4=440Hz
	}

	void setBaseFreqByA(u_freq frequency) override{
		// A4对应音符ID 69 (9度)
		setBaseFreq(frequency / (pow(2.0, 5) * calculateRatio(9)));
	}
	};
}
