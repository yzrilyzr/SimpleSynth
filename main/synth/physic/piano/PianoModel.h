#pragma once
#include "SimpleSynth.h"
#include "PianoKey.h"
#include "PianoKeyParameters.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(PianoModel){
	public:
	static void PianoKeyInitialize(PianoKey & key, PianoKeyParameters & param);
	static void Piano_initString(PianoDwgs & pianoDWGs, u_sample freq, u_sample sampleRate, u_sample hammerPos, u_sample c1, u_sample c3, u_sample dispersionFactor, u_sample Z, u_sample Zb, u_sample Zh);
	static u_sample PianoKeyGo(PianoKey & key);
	static void PianoKeyTrigger(PianoKey & key, s_note_vel v);
	static void PianoKeyDamper(PianoKey & key);
	};
}