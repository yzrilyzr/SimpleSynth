#pragma once
#include "interface/NoteProcessor.h"
#include "SimpleSynth.h"
#include "SynthUtil.h"
#include "interpolator/GraphInterpolator.h"
#include "array/Array.hpp"
#include "events/NoteData.hpp"
#include "piano/PianoKey.h"
#include "piano/PianoKeyParameters.h"
#include "piano/PianoSoundBoardParameters.h"
#include "piano/PianoSoundBoard.h"

namespace yzrilyzr_simplesynth{
	ECLASS(PianoSrc2, public NoteProcessor){
	public:
	PianoKeyParameters keyParams[CHANNEL_MAX_NOTE_ID];
	PianoSoundBoardParameters soundboardParameters;
	private:
	PianoSoundBoard soundboard;
	PianoKey pianoKeys[CHANNEL_MAX_NOTE_ID];
	bool isInSynth[CHANNEL_MAX_NOTE_ID];
	bool onState[CHANNEL_MAX_NOTE_ID];
	public:
	PianoSrc2();
	u_sample getAmp(const Note & note) override;
	void init(ChannelConfig & cfg) override;
	u_sample postProcess(u_sample output) override;
	bool noMoreData(const Note & note) override;
	void cc(ChannelConfig & cfg, ChannelControl & cc) override;
	NoteProcPtr clone() override;
	void noteOn(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel)override;
	void noteOff(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel)override;
	};
}