#pragma once
#include "SimpleSynth.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(NRPN){
	public:
	static constexpr uint16_t MIXER_LIMITER=1000;
	static constexpr uint16_t MIXER_ENABLE_MIDI_CC_EFFECT=1001;
	static constexpr uint16_t MIXER_ENABLE_MIDI_CC_ADSR=1002;
	static constexpr uint16_t BUILDER_START=2000;
	static constexpr uint16_t CHANNEL_3D_YAW=1500;
	static constexpr uint16_t CHANNEL_3D_PITCH=1501;
	static constexpr uint16_t CHANNEL_3D_DISTANCE=1502;
	static constexpr uint16_t CHANNEL_3D_X=1510;
	static constexpr uint16_t CHANNEL_3D_Y=1511;
	static constexpr uint16_t CHANNEL_3D_Z=1512;
	};
}