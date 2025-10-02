#pragma once
#include "SimpleSynth.h"

namespace yzrilyzr_simplesynth{
	class Note;
	class ChannelConfig;
	EBCLASS(NoteUpdater){
		public:
		static void preUpdateNote(Note & note, const ChannelConfig & cfg);
		static void postUpdateNote(Note & note, const ChannelConfig & cfg);
		static void noteOn(Note & note, ChannelConfig & cfg, s_note_id_i id, s_note_vel velocity);
		static void noteOff(Note & note, const ChannelConfig & cfg, s_note_vel velocity);
	};
}