#include "ChannelEvent.h"
#include "interface/NoteProcessor.h"
#include "interface/NoteTuning.h"
#include "util/MIDIFile.h"

using namespace yzrilyzr_util;

namespace yzrilyzr_simplesynth{
	// ChannelEvent 实现
	ChannelEvent::ChannelEvent(const ChannelEvent * other){
		this->groupName=other->groupName;
		this->channelID=other->channelID;
		this->startAtTime=other->startAtTime;
	}

	// NoteOff 实现
	NoteOff::NoteOff(s_midichannel_id channelID, s_note_id_i id, s_note_vel vel)
		: id(id), velocity(vel){
		this->channelID=channelID;
	}

	NoteOff::NoteOff(s_note_id_i id, s_note_vel vel)
		: id(id), velocity(vel){}

	NoteOff::NoteOff(s_note_id_i id)
		: id(id), velocity(0){}

	NoteOff::NoteOff(const NoteOff * clone)
		: ChannelEvent(clone){
		this->id=clone->id;
		this->velocity=clone->velocity;
	}

	u_up<ChannelEvent>  NoteOff::clone(){
		return mkup<NoteOff>(this);
	}

	uint8_t NoteOff::getType(){
		return EventType::NOTE_OFF;
	}

	// NoteOn 实现
	NoteOn::NoteOn(s_midichannel_id channelID, s_note_id_i id, s_note_vel velocity)
		: id(id), velocity(velocity){
		this->channelID=channelID;
	}

	NoteOn::NoteOn(s_note_id_i id, s_note_vel velocity)
		: id(id), velocity(velocity){}

	NoteOn::NoteOn(const NoteOn * clone)
		: ChannelEvent(clone){
		this->id=clone->id;
		this->velocity=clone->velocity;
	}

	u_up<ChannelEvent>  NoteOn::clone(){
		return  mkup< NoteOn>(this);
	}

	uint8_t NoteOn::getType(){
		return EventType::NOTE_ON;
	}

	// NotePressure 实现
	NotePressure::NotePressure(s_midichannel_id channelID, s_note_id_i id, s_note_vel velocity)
		: id(id), velocity(velocity){
		this->channelID=channelID;
	}

	NotePressure::NotePressure(s_note_id_i id, s_note_vel velocity)
		: id(id), velocity(velocity){}

	NotePressure::NotePressure(const NotePressure * clone)
		: ChannelEvent(clone){
		this->id=clone->id;
		this->velocity=clone->velocity;
	}

	u_up<ChannelEvent>  NotePressure::clone(){
		return  mkup<NotePressure>(this);
	}

	uint8_t NotePressure::getType(){
		return EventType::NOTE_PRESSURE;
	}

	// NotePitchBend 实现
	NotePitchBend::NotePitchBend(s_midichannel_id channelID, s_note_id_i id, s_note_id value)
		: id(id), value(value){
		this->channelID=channelID;
	}

	NotePitchBend::NotePitchBend(s_note_id_i id, s_note_id value)
		: id(id), value(value){}

	NotePitchBend::NotePitchBend(const NotePitchBend * clone)
		: ChannelEvent(clone){
		this->id=clone->id;
		this->value=clone->value;
	}

	u_up<ChannelEvent>  NotePitchBend::clone(){
		return  mkup<NotePitchBend>(this);
	}

	uint8_t NotePitchBend::getType(){
		return EventType::NOTE_PITCH_BEND;
	}

	// ChannelControl 实现
	ChannelControl::ChannelControl(s_midichannel_id channelID, uint32_t control, uint32_t value)
		: control(control), value(value){
		this->channelID=channelID;
	}

	ChannelControl::ChannelControl(uint32_t control, uint32_t value)
		: control(control), value(value){}

	ChannelControl::ChannelControl(const ChannelControl * clone)
		: ChannelEvent(clone){
		this->control=clone->control;
		this->value=clone->value;
	}

	bool ChannelControl::isMSB(){
		return control == MIDIFile::CC::DATA_ENTRY_MSB;
	}

	bool ChannelControl::isLSB(){
		return control == MIDIFile::CC::DATA_ENTRY_LSB;
	}

	u_up<ChannelEvent>  ChannelControl::clone(){
		return  mkup<ChannelControl>(this);
	}

	uint8_t ChannelControl::getType(){
		return EventType::CHANNEL_CONTROL;
	}

	// ChannelPitchBend 实现
	ChannelPitchBend::ChannelPitchBend(s_midichannel_id channelID, u_normal_11_f value)
		: value(value){
		this->channelID=channelID;
	}

	ChannelPitchBend::ChannelPitchBend(u_normal_11_f value)
		: value(value){}

	ChannelPitchBend::ChannelPitchBend(const ChannelPitchBend * clone)
		: ChannelEvent(clone){
		this->value=clone->value;
	}

	u_up<ChannelEvent>  ChannelPitchBend::clone(){
		return  mkup<ChannelPitchBend>(this);
	}

	uint8_t ChannelPitchBend::getType(){
		return EventType::CHANNEL_PITCH_BEND;
	}

	// ProgramChange 实现
	ProgramChange::ProgramChange(){}

	ProgramChange::ProgramChange(s_midichannel_id channelID, s_program_id value)
		: id(value){
		this->channelID=channelID;
	}

	ProgramChange::ProgramChange(s_program_id value)
		: id(value){}
	ProgramChange::ProgramChange(NoteProcPtr np)
		: noteProcessor(np){}
	ProgramChange::ProgramChange(s_midichannel_id channelID, NoteProcPtr np)
		: noteProcessor(np){
		this->channelID=channelID;
	}

	ProgramChange::ProgramChange(const ProgramChange * clone)
		: ChannelEvent(clone){
		this->id=clone->id;
		if(clone->noteProcessor != nullptr){
			this->noteProcessor=clone->noteProcessor->clone();
		}
	}

	u_up<ChannelEvent>  ProgramChange::clone(){
		return  mkup<ProgramChange>(this);
	}

	uint8_t ProgramChange::getType(){
		return EventType::CHANNEL_PROGRAM_CHANGE;
	}

	// ChannelPressure 实现
	ChannelPressure::ChannelPressure(s_midichannel_id channelID, s_note_vel value)
		: value(value){
		this->channelID=channelID;
	}

	ChannelPressure::ChannelPressure(s_note_vel value)
		: value(value){}

	ChannelPressure::ChannelPressure(const ChannelPressure * clone)
		: ChannelEvent(clone){
		this->value=clone->value;
	}

	u_up<ChannelEvent>  ChannelPressure::clone(){
		return  mkup<ChannelPressure>(this);
	}

	uint8_t ChannelPressure::getType(){
		return EventType::CHANNEL_PRESSURE;
	}

	// TuningChange 实现
	TuningChange::TuningChange(s_midichannel_id channelID, u_sp<NoteTuning> value){
		this->channelID=channelID;
		this->value=value;
	}
	TuningChange::TuningChange(u_sp<NoteTuning> value){
		this->value=value;
	}

	TuningChange::TuningChange(const TuningChange * clone)
		: ChannelEvent(clone){
		this->value=clone->value;
	}

	u_up<ChannelEvent>  TuningChange::clone(){
		return  mkup<TuningChange>(this);
	}

	uint8_t TuningChange::getType(){
		return EventType::TUNING_CHANGE;
	}
	// DrumChannel 实现
	DrumChannel::DrumChannel(){}
	DrumChannel::DrumChannel(s_midichannel_id channelID, bool enable){
		this->channelID=channelID;
		this->enable=enable;
	}
	DrumChannel::DrumChannel(bool enable){
		this->enable=enable;
	}

	DrumChannel::DrumChannel(const DrumChannel * clone)
		: ChannelEvent(clone){
		this->enable=clone->enable;
	}

	u_up<ChannelEvent>  DrumChannel::clone(){
		return  mkup<DrumChannel>(this);
	}

	uint8_t DrumChannel::getType(){
		return EventType::DRUM_CHANNEL;
	}
}