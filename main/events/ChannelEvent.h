#pragma once
#include "SimpleSynth.h"
#include <memory>

namespace yzrilyzr_simplesynth{
	class NoteProcessor;
	class NoteTuning;

	namespace EventType{
		static constexpr uint8_t const NOTE_ON=1;
		static constexpr uint8_t const NOTE_OFF=2;
		static constexpr uint8_t const NOTE_PRESSURE=3;
		static constexpr uint8_t const NOTE_PITCH_BEND=4;
		static constexpr uint8_t const CHANNEL_CONTROL=5;
		static constexpr uint8_t const CHANNEL_PITCH_BEND=6;
		static constexpr uint8_t const CHANNEL_PROGRAM_CHANGE=7;
		static constexpr uint8_t const CHANNEL_PRESSURE=8;
		static constexpr uint8_t const TUNING_CHANGE=9;
		static constexpr uint8_t const DRUM_CHANNEL=10;
	}

	typedef u_sp<NoteProcessor> NoteProcPtr;

	EBCLASS(ChannelEvent){
		public:
		s_midichannel_id channelID=0;
		yzrilyzr_lang::String groupName;
		u_time startAtTime=0;

		ChannelEvent()=default;
		virtual ~ChannelEvent()=default;
		ChannelEvent(const ChannelEvent * other);

		virtual u_up<ChannelEvent>  clone()=0;
		virtual uint8_t getType()=0;
	};

	ECLASS(NoteOff, public ChannelEvent){
		public:
		s_note_id_i id;
		s_note_vel velocity;

		NoteOff(s_midichannel_id channelID, s_note_id_i id, s_note_vel vel);
		NoteOff(s_note_id_i id, s_note_vel vel);
		NoteOff(s_note_id_i id);

		u_up<ChannelEvent>  clone() override;
		uint8_t getType() override;

		NoteOff(const NoteOff * clone);
	};

	ECLASS(NoteOn, public ChannelEvent){
		public:
		s_note_id_i id;
		s_note_vel velocity;

		NoteOn(s_midichannel_id channelID, s_note_id_i id, s_note_vel velocity);
		NoteOn(s_note_id_i id, s_note_vel velocity);

		u_up<ChannelEvent>  clone() override;
		uint8_t getType() override;

		NoteOn(const NoteOn * clone);
	};

	ECLASS(NotePressure, public ChannelEvent){
		public:
		s_note_id_i id;
		s_note_vel velocity;

		NotePressure(s_midichannel_id channelID, s_note_id_i id, s_note_vel velocity);
		NotePressure(s_note_id_i id, s_note_vel velocity);

		u_up<ChannelEvent>  clone() override;
		uint8_t getType() override;

		NotePressure(const NotePressure * clone);
	};

	ECLASS(NotePitchBend, public ChannelEvent){
		public:
		s_note_id_i id;
		s_note_id value;

		NotePitchBend(s_midichannel_id channelID, s_note_id_i id, s_note_id value);
		NotePitchBend(s_note_id_i id, s_note_id value);

		u_up<ChannelEvent>  clone() override;
		uint8_t getType() override;

		NotePitchBend(const NotePitchBend * clone);
	};

	ECLASS(ChannelControl, public ChannelEvent){
		public:
		uint32_t control;
		uint32_t value;

		ChannelControl(s_midichannel_id channelID, uint32_t control, uint32_t value);
		ChannelControl(uint32_t control, uint32_t value);

		bool isMSB();
		bool isLSB();

		u_up<ChannelEvent>  clone() override;
		uint8_t getType() override;

		ChannelControl(const ChannelControl * clone);
	};

	ECLASS(ChannelPitchBend, public ChannelEvent){
		public:
		u_normal_11_f value;

		ChannelPitchBend(s_midichannel_id channelID, u_normal_11_f value);
		ChannelPitchBend(u_normal_11_f value);

		u_up<ChannelEvent>  clone() override;
		uint8_t getType() override;

		ChannelPitchBend(const ChannelPitchBend * clone);
	};

	ECLASS(ProgramChange, public ChannelEvent){
		public:
		s_program_id id;
		NoteProcPtr noteProcessor=nullptr;

		ProgramChange();
		ProgramChange(s_midichannel_id channelID, s_program_id value);
		ProgramChange(s_program_id value);
		ProgramChange(NoteProcPtr np);
		ProgramChange(s_midichannel_id channelID, NoteProcPtr np);

		u_up<ChannelEvent>  clone() override;
		uint8_t getType() override;

		ProgramChange(const ProgramChange * clone);
	};

	ECLASS(ChannelPressure, public ChannelEvent){
		public:
		s_note_vel value;

		ChannelPressure(s_midichannel_id channelID, s_note_vel value);
		ChannelPressure(s_note_vel value);

		u_up<ChannelEvent>  clone() override;
		uint8_t getType() override;

		ChannelPressure(const ChannelPressure * clone);
	};

	ECLASS(TuningChange, public ChannelEvent){
		public:
		u_sp<NoteTuning> value=nullptr;
		TuningChange(u_sp<NoteTuning> value);
		TuningChange(s_midichannel_id channelID, u_sp<NoteTuning> value);

		u_up<ChannelEvent>  clone() override;
		uint8_t getType() override;

		TuningChange(const TuningChange * clone);
	};

	ECLASS(DrumChannel, public ChannelEvent){
		public:
		bool enable;

		DrumChannel();
		DrumChannel(s_midichannel_id channelID, bool enable);
		DrumChannel(bool enable);

		u_up<ChannelEvent>  clone() override;
		uint8_t getType() override;

		DrumChannel(const DrumChannel * clone);
	};
}