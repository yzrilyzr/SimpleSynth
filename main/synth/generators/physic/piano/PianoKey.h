#pragma once
#include "SimpleSynth.h"
#include "Hammer.h"
#include "synth/generators/physic/dwg/DigitalWaveGuide.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(PianoDwgs){
	public:
	DigitalWaveGuide dwgs[4];
	};
	EBCLASS(PianoKey){
	public:
	double weight=0;
	int loadState=0;
	std::vector<std::shared_ptr<PianoDwgs>> string;
	double Z=0;
	double ZBridge=0;
	double ZHammer=0;
	std::unique_ptr<Hammer> hammer=nullptr;
	double onTimePassed=0;
	double offTimePassed=0;
	};
}