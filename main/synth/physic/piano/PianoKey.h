#pragma once
#include "SimpleSynth.h"
#include "Hammer.h"
#include "synth/physic/dwg/DigitalWaveGuide.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(PianoDwgs){
	public:
	DigitalWaveGuide dwgs[4];
	};
	EBCLASS(PianoKey){
	public:
	u_sample weight=0;
	int loadState=0;
	std::vector<u_sp<PianoDwgs>> string;
	u_sample Z=0;
	u_sample ZBridge=0;
	u_sample ZHammer=0;
	u_sp<Hammer> hammer=nullptr;
	u_sample onTimePassed=0;
	u_sample offTimePassed=0;
	};
}