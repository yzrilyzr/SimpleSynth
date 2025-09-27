#include "Channel.h"
#include "Mixer.h"
#include "Mixer2.h"
#include "SynthUtil.h"
#include "instrument/SimpleWaveTable.h"
#include "instrument/XMInstrument.h"
#include "synth/envelopers/EnvUtil.h"
#include "synth/generators/physic/KarplusStrongSrc.h"
#include "synth/source/AmpBuilder.h"
#include "interface/NoteProcessor.h"
#include "array/SampleArray.h"
#include "dsp/IIR.h"
#include "dsp/IIRUtil.h"
#include "util/Random.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_array;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_io;
using namespace yzrilyzr_collection;
using namespace yzrilyzr_lang;

namespace yzrilyzr_simplesynth{
	std::string TickChange::toString() const{
		return StringFormat::format("[TickChange:%.2fms Start:%d]", tick * 1000.0f, startAtTick);
	}
	std::shared_ptr<MixerSequence> SynthUtil::parseMIDI(InputStream & is){
		try{
			std::shared_ptr<MixerSequence> mixerSequence=std::make_shared<MixerSequence>();
			MIDIFile::MIDISequence * midiSequence=MIDIFile::parse(is);
			if(midiSequence == nullptr)return nullptr;
			//Set<String> channelAndTrackName = new HashSet<>();
			int noteShift=0;
			ArrayList<ChannelEvent *> ticks;
			for(int ii=0;ii < midiSequence->tracks.size();ii++){
				MIDIFile::Track * t=midiSequence->tracks.get(ii);
				double delay=0;
				int delayTicks=0;
				float ticksSecond=0.001f;
				for(int jj=0;jj < t->events.size();jj++){
					MIDIFile::MIDIEvent * e=t->events.get(jj);
					if(e == nullptr)continue;
					delayTicks+=e->deltaTimeTicks;
					delay+=e->deltaTimeTicks * ticksSecond;
					for(int32_t i=(int32_t)ticks.size() - 1;i >= 0;i--){
						ChannelEvent * m=ticks.get(i);
						TickChange * change=(TickChange *)m;
						if(delayTicks >= change->startAtTick){
							//if(change->tick!=ticksSecond){
							ticksSecond=change->tick;
							delay=m->startAtTime + (delayTicks - change->startAtTick) * ticksSecond;
							//}
							break;
						}
					}
					MIDIFile::EventType et=e->getType();
					if(et == MIDIFile::EventType::Note){
						MIDIFile::MIDINote * mnote=(MIDIFile::MIDINote *)e;
						int channel=mnote->channel;
						if(mnote->action == 0){
							ChannelEvent * n1=new NoteOff(mnote->id + noteShift, mnote->velocity / 127.0f);
							mixerSequence->postToSequence(channel, n1, delay);
						} else if(mnote->action == 1){
							ChannelEvent * n1=new NoteOn(mnote->id + noteShift, mnote->velocity / 127.0f);
							mixerSequence->postToSequence(channel, n1, delay);
						} else if(mnote->action == 2){
							ChannelEvent * n1=new NotePressure(mnote->id + noteShift, mnote->velocity / 127.0f);
							mixerSequence->postToSequence(channel, n1, delay);
						}
						//channelAndTrackName->add(t->name + " Channel:" + channel);
					} else if(et == MIDIFile::EventType::ChannelPitchBend){
						MIDIFile::MIDIPitchBend * pitchBend=(MIDIFile::MIDIPitchBend *)e;
						int channel=pitchBend->channel;
						ChannelEvent * n1=new ChannelPitchBend((pitchBend->value - 8192.0f) / 8192.0f);
						mixerSequence->postToSequence(channel, n1, delay);
					} else if(et == MIDIFile::EventType::ChannelPressure){
						MIDIFile::MIDIChannelPressure * pressure=(MIDIFile::MIDIChannelPressure *)e;
						int channel=pressure->channel;
						ChannelEvent * n1=new ChannelPressure(pressure->value / 127.0f);
						mixerSequence->postToSequence(channel, n1, delay);
					} else if(et == MIDIFile::EventType::ChannelControl){
						MIDIFile::MIDIChannelControl * control=(MIDIFile::MIDIChannelControl *)e;
						int channel=control->channel;
						ChannelEvent * n1=new ChannelControl(control->control, control->value);
						mixerSequence->postToSequence(channel, n1, delay);
					} else if(et == MIDIFile::EventType::FF){
						MIDIFile::FFMessage * ffMessage=(MIDIFile::FFMessage *)e;
						if(ffMessage->type == 0x51){
							float newTick=ffMessage->quarterNoteDurationMicroSeconds / (float)midiSequence->ticksForQuarterNote / 1000000.0f;
							ChannelEvent * ce=new TickChange(delayTicks, newTick);
							ce->startAtTime=delay;
							ticks.add(ce);
						}
					} else if(et == MIDIFile::EventType::ProgramChange){
						MIDIFile::MIDIProgramChange * setInstrument=(MIDIFile::MIDIProgramChange *)e;
						int channel=setInstrument->channel;
						ChannelEvent * n1=new ProgramChange(setInstrument->num);
						mixerSequence->postToSequence(channel, n1, delay);
					}
				}
			}
			delete midiSequence;
			mixerSequence->sortPosted();
			//for (String s : channelAndTrackName) System->out->println(s);
			return mixerSequence;
		} catch(...){
			return nullptr;
		}
	}
	NoteProcPtr SynthUtil::getDefault(){
		return AmpBuilder().src(KarplusStrongBuilder().burst(SimpleWaveTable::Piano_Wave).alpha(0.7).build())
			.biquadEnvVel(70, 127, 1, LOWPASS)
			.ADSR(5, 5000, 0, false, 100, Pow(-5), Pow(8), Pow(5))
			.build();
	}
	//std::shared_ptr<IChannel> SynthUtil::getMIDIChannelOrNew(IMixer * mixer, s_midichannel_id channelID){
	//	return getMIDIChannelOrNew(mixer, IMixer::DEFAULT_MIDI_CHANNEL_GROUP_NAME, channelID);
	//}
	//std::shared_ptr<IChannel> SynthUtil::getMIDIChannelOrNew(IMixer * mixer, const std::string & groupName, s_midichannel_id channelID){
	//	if(auto m1=dynamic_cast<Mixer *>(mixer)){
	//		std::shared_ptr<Channel> channel=std::dynamic_pointer_cast<Channel>(m1->getMIDIChannel(groupName, channelID));
	//		if(channel == nullptr){
	//			channel=std::make_shared<Channel>();
	//			channel->setName(groupName + " #" + std::to_string(channelID));
	//			m1->setMIDIChannel(groupName, channelID, channel);
	//		}
	//		if(channelID == 9) channel->setSustain(true);
	//		return std::dynamic_pointer_cast<IChannel>(channel);
	//	}
	//	return nullptr;
	//}
	ChannelEvent * SynthUtil::MIDIBytes2Event(uint8_t ty, uint8_t data1, uint8_t data2){
		int type=ty & 0b11110000;
		int channelID=ty & 0b1111;
		switch(type){
			case 0x80:
				return new NoteOff(channelID, data1 & 0xff, (data2 & 0xff) / 127.0f);
			case 0x90:
				return  new NoteOn(channelID, data1 & 0xff, (data2 & 0xff) / 127.0f);
			case 0xa0:
				return new NotePressure(channelID, (data1 & 0xff), (data2 & 0xff) / 127.0f);
			case 0xb0:
				return new ChannelControl(channelID, data1 & 0xff, data2 & 0xff);
			case 0xc0:
				return new ProgramChange(channelID, data1 & 0xff);
			case 0xd0:
				return new ChannelPressure(channelID, (data2 & 0xff) / 127.0f);
			case 0xe0:
				return new ChannelPitchBend(channelID, (((data1 & 0x7f) | ((data2 & 0x7f) << 7)) - 8192.0f) / 8191.0f);
			case 0xf0:
				break;
		}
		return nullptr;
	}
	void SynthUtil::sendMIDIBytes(IMixer * mixer, uint8_t ty, uint8_t data1, uint8_t data2){
		sendMIDIBytes(mixer, ty, data1, data2, Mixer::DEFAULT_MIDI_CHANNEL_GROUP_NAME);
	}
	void SynthUtil::sendMIDIBytes(IMixer * mixer, uint8_t ty, uint8_t data1, uint8_t data2, const std::string & groupName){
		auto * event=MIDIBytes2Event(ty, data1, data2);
		event->groupName=groupName;
		mixer->sendInstantEvent(event);
	}
	void SynthUtil::sendMIDIEvent(ChannelEvent * event, const std::string & deviceName){
		if(callback != nullptr)(*callback)(deviceName, Event2MIDIBytes(event));
	}
	uint64_t SynthUtil::MergeMIDIBytes(uint8_t ty, uint8_t data1, uint8_t data2){
		return (uint64_t)(ty & 0xFF) |
			((uint64_t)(data1 & 0x7F) << 8) |
			((uint64_t)(data2 & 0x7F) << 16);
	}
	uint64_t SynthUtil::Event2MIDIBytes(ChannelEvent * event){
		uint8_t ch=event->channelID & 0b1111;
		switch(event->getType()){
			case EventType::NOTE_ON:
			{
				NoteOn & ev=static_cast<NoteOn &>(*event);
				return MergeMIDIBytes(0x90 | ch, ev.id, static_cast<uint8_t>(Util::clamp(ev.velocity * 127.0, 0.0, 127.0)));
			}
			case EventType::NOTE_OFF:
			{
				NoteOff & ev=static_cast<NoteOff &>(*event);
				return MergeMIDIBytes(0x80 | ch, ev.id, static_cast<uint8_t>(Util::clamp(ev.velocity * 127.0, 0.0, 127.0)));
			}
			case EventType::CHANNEL_PITCH_BEND:
			{
				ChannelPitchBend & ev=static_cast<ChannelPitchBend &>(*event);
				uint32_t v=Util::clamp(ev.value * 8191.0 + 8192.0, 0.0, 16383.0);
				return MergeMIDIBytes(0xe0 | ch, v & 0x7f, (v >> 7) & 0x7f);
			}
			case EventType::NOTE_PRESSURE:
			{
				NotePressure & ev=static_cast<NotePressure &>(*event);
				return MergeMIDIBytes(0xa0 | ch, ev.id, static_cast<uint8_t>(Util::clamp(ev.velocity * 127.0, 0.0, 127.0)));
			}
			case EventType::CHANNEL_CONTROL:
			{
				ChannelControl & ev=static_cast<ChannelControl &>(*event);
				return MergeMIDIBytes(0xb0 | ch, ev.control, ev.value);
			}

			case EventType::CHANNEL_PRESSURE:
			{
				ChannelPressure & ev=static_cast<ChannelPressure &>(*event);
				return MergeMIDIBytes(0xd0 | ch, static_cast<uint8_t>(Util::clamp(ev.value * 127.0, 0.0, 127.0)), 0);
			}
			case EventType::CHANNEL_PROGRAM_CHANGE:
			{
				ProgramChange & ev=static_cast<ProgramChange &>(*event);
				return MergeMIDIBytes(0xc0 | ch, ev.id, 0);
			}
			default:return 0;
		}
	}
	void SynthUtil::setMIDICallback(MIDICallback c){
		callback=c;
	}
	FixedRandom::FixedRandom(SampleArray * data){
		this->data=data;
		//this->index=0;
	}
	FixedRandom::FixedRandom(){
		this->data=SynthUtil::NOISE;
		//this->index=0;
	}
	double FixedRandom::next(size_t * index){
		double d=(*data)[*index];
		*index=(*index + 1) % data->length;
		return d;
	}
	SampleArray * SynthUtil::noise(int32_t length, int32_t sampleRate, u_freq f1, u_freq f2){
		SampleArray * randomData=new SampleArray(length);
		std::shared_ptr<IIR> iir=IIRUtil::newButterworthIIRFilter(sampleRate, FilterPassType::BANDPASS, 2, f1, f2);
		iir->init(sampleRate);
		Random random(5319539547595419742L);
		for(int i=0;i < length;i++){
			(*randomData)[i]=iir->procDsp(random.nextGaussian());
		}
		return randomData;
	}
	void SynthUtil::deleteStatic(){
		delete NOISE;
	}
	std::shared_ptr<MixerSequence> SynthUtil::parseXM(InputStream & inputStream){
		std::shared_ptr<MixerSequence> mixerSequence=std::make_shared<MixerSequence>();
		std::shared_ptr<XMFile::Module> modulep=XMFile::parse(inputStream);
		if(modulep == nullptr)return nullptr;
		XMFile::Module & module1=*modulep;
		double time=0;
		double ticksPerSecond=module1.bpm * 0.4;
		double rowsPerSecond=ticksPerSecond / module1.tempo;
		double rowDeltaTime=1.0 / rowsPerSecond;
		int channelMap=32;
		std::cout << rowDeltaTime * 64 << std::endl;
		for(int i=0;i < module1.num_channels;i++){
			ChannelEvent * event=new ChannelControl(MIDIFile::CC::MONO_MODE, 127);
			mixerSequence->postToSequence(i + channelMap, event, 0);
		}
		IntArray notePrevInstrument(module1.num_channels);
		for(int patternTableIndex=0;patternTableIndex < 19;patternTableIndex++){
			int patternIndex=module1.pattern_table[patternTableIndex];
			if(patternIndex >= module1.patterns.size())throw IndexOutOfBoundsException();
			XMFile::Pattern & pattern=module1.patterns[patternIndex];
			bool portamentoState=false;
			for(int row=0;row < pattern.num_rows;row++){
				time+=rowDeltaTime;
				for(int channelI=0;channelI < module1.num_channels;channelI++){
					int slotIndex=row * module1.num_channels + channelI;
					int channel=channelI + channelMap;
					XMFile::PatternSlot & s=pattern.slots[slotIndex];
					if(s.instrument > 0 && notePrevInstrument[channelI] == s.instrument){
						ChannelEvent * channelEvent=new ChannelControl(MIDIFile::CC::RESET_ALL_CONTROLLERS, 0);
						mixerSequence->postToSequence(channel, channelEvent, time);
						channelEvent=new ChannelControl(MIDIFile::CC::MONO_MODE, 127);
						mixerSequence->postToSequence(channel, channelEvent, time);
					} else if(s.instrument > 0 && notePrevInstrument[channelI] != s.instrument){
						ChannelEvent * channelEvent=new ProgramChange(s.instrument - 1);
						mixerSequence->postToSequence(channel, channelEvent, time);
						channelEvent=new ChannelControl(MIDIFile::CC::RESET_MUTE_ALL_NOTES, 127);
						mixerSequence->postToSequence(channel, channelEvent, time);
						notePrevInstrument[channelI]=s.instrument;
					}
					switch(s.volume_column >> 4){
						case 0x5:
							if(s.volume_column > 0x50) break;
							[[fallthrough]];
						case 0x1: [[fallthrough]];
						case 0x2: [[fallthrough]];
						case 0x3: [[fallthrough]];
						case 0x4:
						{/* Set volume */
							float volume=(float)(s.volume_column - 0x10) / (float)0x40;
							ChannelEvent * channelEvent=new ChannelControl(MIDIFile::CC::VOLUME, (int)(volume * 127));
							mixerSequence->postToSequence(channel, channelEvent, time);
						}
						break;
						case 0xC: /* Set panning */
						{
							float panning=(float)(((s.volume_column & 0x0F) << 4) | (s.volume_column & 0x0F)) / (float)0xFF;
							ChannelEvent * channelEvent=new ChannelControl(MIDIFile::CC::PAN, (int)(panning * 127));
							mixerSequence->postToSequence(channel, channelEvent, time);
						}
						break;
						default:
							break;
					}
					if(s.effect_param == 0 && s.effect_type == 0){
					}
					switch(s.effect_type){
						case XMFile::EffectType::PORTAMENTO_UP:
						{
							portamentoState=true;
							ChannelEvent * channelEvent=new ChannelControl(XMFile::CC::PORTAMENTO_UP, s.effect_param * 127 / 0xff);
							mixerSequence->postToSequence(channel, channelEvent, time);
						}
						break;
						case XMFile::EffectType::PORTAMENTO_DOWN:
						{
							portamentoState=true;
							ChannelEvent * channelEvent=new ChannelControl(XMFile::CC::PORTAMENTO_DOWN, s.effect_param * 127 / 0xf);
							mixerSequence->postToSequence(channel, channelEvent, time);
						}
						break;
						case XMFile::EffectType::TONE_PORTAMENTO:
						{
							portamentoState=true;
							ChannelEvent * channelEvent=new ChannelControl(MIDIFile::CC::PORTAMENTO_SWITCH, 127);
							mixerSequence->postToSequence(channel, channelEvent, time);
							channelEvent=new ChannelControl(MIDIFile::CC::PORTAMENTO_TIME, (0xff - s.effect_param) * 32 / 0xff);
							mixerSequence->postToSequence(channel, channelEvent, time);
						}
						break;
						case XMFile::EffectType::SET_PANNING: /* 8xx: Set panning */
						{
							float panning=(float)s.effect_param / (float)0xFF;
							ChannelEvent * channelEvent=new ChannelControl(MIDIFile::CC::VOLUME, (int)(panning * 127));
							mixerSequence->postToSequence(channel, channelEvent, time);
						}
						break;
						case XMFile::EffectType::SET_VOLUME: /* Cxx: Set volume */
						{
							float volume=(float)(std::min(s.effect_param, 0x40)) / (float)0x40;
							ChannelEvent * channelEvent=new ChannelControl(MIDIFile::CC::VOLUME, (int)(volume * 127));
							//mixerSequence->postToSequence(channel, channelEvent, time);
						}
						break;
					}
					if(s.note > 0 && s.note < 97){
						if(s.effect_type != XMFile::EffectType::TONE_PORTAMENTO &&
						   s.effect_type != XMFile::EffectType::PORTAMENTO_DOWN &&
						   s.effect_type != XMFile::EffectType::PORTAMENTO_UP &&
						   portamentoState){
							ChannelEvent * channelEvent=new ChannelControl(MIDIFile::CC::PORTAMENTO_SWITCH, 0);
							mixerSequence->postToSequence(channel, channelEvent, time);
							portamentoState=false;
						}
						if(s.instrument > 0){
							ChannelEvent * channelEvent=new NoteOn(s.note - 1, 1);
							mixerSequence->postToSequence(channel, channelEvent, time);
						}
					} else if(s.note == 97){
						ChannelEvent * channelEvent=new ChannelControl(MIDIFile::CC::ALL_NOTES_OFF, 127);
						mixerSequence->postToSequence(channel, channelEvent, time);
					}
				}
			}
		}
		time+=rowDeltaTime;
		for(int channelI=0;channelI < module1.num_channels;channelI++){
			int channel=channelI + channelMap;
			ChannelEvent * channelEvent=new ChannelControl(MIDIFile::CC::ALL_NOTES_OFF, 127);
			mixerSequence->postToSequence(channel, channelEvent, time);
		}
		mixerSequence->setInstrument(std::make_shared<XMInstrument>(modulep));
		mixerSequence->sortPosted();
		return mixerSequence;
	}
	bool SynthUtil::isInstrumentProviderSustainable(s_program_id program){
		return
			(program >= MIDIFile::Instruments::ORGAN_HAMMOND_ORGAN && program <= MIDIFile::Instruments::ORGAN_TANGO_ACCORDIAN)
			|| (program >= MIDIFile::Instruments::GUITAR_OVERDRIVEN_GUITAR && program <= MIDIFile::Instruments::GUITAR_GUITAR_HARMONICS)
			|| (program >= MIDIFile::Instruments::SOLO_STRING_VIOLIN && program <= MIDIFile::Instruments::SOLO_STRING_TREMOLO_STRINGS)
			|| (program >= MIDIFile::Instruments::ENSEMBLE_STRING_ENSEMBLE_1 && program <= MIDIFile::Instruments::ENSEMBLE_SYNTH_VOICE)
			|| (program >= MIDIFile::Instruments::BRASS_TRUMPET && program <= MIDIFile::Instruments::BRASS_SYNTH_BRASS_2)
			|| (program >= MIDIFile::Instruments::REED_SOPRANO_SAX && program <= MIDIFile::Instruments::REED_CLARINET)
			|| (program >= MIDIFile::Instruments::PIPE_PICCOLO && program <= MIDIFile::Instruments::PIPE_OCARINA)
			|| (program >= MIDIFile::Instruments::LEAD_SQUARE && program <= MIDIFile::Instruments::LEAD_BASS_LEAD)
			|| (program >= MIDIFile::Instruments::PAD_NEW_AGE && program <= MIDIFile::Instruments::PAD_SWEEP)
			|| (program >= MIDIFile::Instruments::ETHNIC_BAGPIPE && program <= MIDIFile::Instruments::ETHNIC_FIDDLE)
			;
	}
	SampleArray * SynthUtil::NOISE=SynthUtil::noise(96000, 48000, 20, 20000);
	SynthUtil::MIDICallback SynthUtil::callback=nullptr;
}