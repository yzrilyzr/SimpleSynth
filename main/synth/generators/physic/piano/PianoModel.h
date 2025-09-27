#pragma once
#include "SimpleSynth.h"
#include "PianoKey.h"
#include "PianoKeyParameters.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(PianoModel){
	public:
	static void PianoKeyInitialize(PianoKey & key, PianoKeyParameters & param);
	static void Piano_initString(PianoDwgs & pianoDWGs, double freq, double sampleRate, double hammerPos, double c1, double c3, double dispersionFactor, double Z, double Zb, double Zh);
	static double PianoKeyGo(PianoKey & key);
	static void PianoKeyTrigger(PianoKey & key, s_note_vel v);
	static void PianoKeyDamper(PianoKey & key);
	};
}