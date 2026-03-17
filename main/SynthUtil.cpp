#include "Mixer.h"
#include "Mixer2.h"
#include "SynthUtil.h"
#include "instrument/SimpleWaveTable.h"
#include "instrument/XMInstrument.h"
#include "synth/envelopers/EnvUtil.h"
#include "synth/generators/physic/KarplusStrongSrc.h"
#include "synth/source/AmpBuilder.h"
#include "interface/NoteProcessor.h"
#include "array/Array.hpp"
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
	String TickChange::toString() const{
		return StringFormat::format("[TickChange:%.2fms Start:%d]", tick * 1000.0f, startAtTick);
	}
	void SynthUtil::sequenceToMIDI(u_sp<MixerSequence> mixerSeq, OutputStream & os){
		auto midiSeq=mksp<MIDIFile::MIDISequence>();
		midiSeq->midiFormat=1; // 多音轨格式
		midiSeq->ticksForQuarterNote=480; // 标准PPQ
		const double bpm=120;

		const double ticksPerSecond=midiSeq->ticksForQuarterNote * bpm / 60.0;

		auto tempoTrack=mksp<MIDIFile::Track>();
		u_sp<MIDIFile::FFMessage> tickEvent=mksp<MIDIFile::FFMessage>(0);
		tickEvent->type=0x51;
		tickEvent->quarterNoteDurationMicroSeconds=midiSeq->ticksForQuarterNote / ticksPerSecond * 1000000.0;
		tickEvent->b=ByteArray(3);
		tickEvent->b[0]=(tickEvent->quarterNoteDurationMicroSeconds >> 16) & 0xff;
		tickEvent->b[1]=(tickEvent->quarterNoteDurationMicroSeconds >> 8) & 0xff;
		tickEvent->b[2]=(tickEvent->quarterNoteDurationMicroSeconds >> 0) & 0xff;

		tempoTrack->events.add(tickEvent);
		midiSeq->tracks.add(tempoTrack);

		// 2. 遍历MixerSequence的所有通道和事件
		for(const auto & channelEntry : mixerSeq->channelEvents){
			s_midichannel_id globalChannel=channelEntry.first;
			auto & events=channelEntry.second;

			if(events.empty()) continue;

			// 每个通道创建一个音轨
			auto track=mksp<MIDIFile::Track>();

			// 不添加端口元事件，直接使用通道ID作为轨道
			int midiChannel=globalChannel % 16;  // 只使用通道部分

			// 转换该通道的所有事件
			u_time lastEventTime=0;

			for(auto & eventWrapper : events){
				ChannelEvent & event=*eventWrapper.event;
				u_time eventTime=event.startAtTime; // 绝对时间（秒）
				int deltaTicks=static_cast<int>((eventTime - lastEventTime) * ticksPerSecond + 0.5f);
				lastEventTime=eventTime;

				// 根据事件类型创建MIDI事件
				u_sp<MIDIFile::MIDIEvent> midiEvent=nullptr;
				auto eventType=event.getType();
				if(eventType == EventType::NOTE_ON){
					auto * noteOn=static_cast<NoteOn *>(&event);
					uint8_t status=0x90 | (midiChannel & 0x0F);
					auto midiNote=mksp<MIDIFile::MIDINote>(
						deltaTicks,
						status,
						noteOn->id,
						static_cast<uint8_t>(noteOn->velocity * 127)
					);
					midiEvent=midiNote;
				} else if(eventType == EventType::NOTE_OFF){
					auto * noteOff=static_cast<NoteOff *>(&event);
					uint8_t status=0x80 | (midiChannel & 0x0F);
					auto midiNote=mksp<MIDIFile::MIDINote>(
						deltaTicks,
						status,
						noteOff->id,
						static_cast<uint8_t>(noteOff->velocity * 127)
					);
					midiEvent=midiNote;
				} else if(eventType == EventType::CHANNEL_PROGRAM_CHANGE){
					auto * progChange=static_cast<ProgramChange *>(&event);
					uint8_t status=0xC0 | (midiChannel & 0x0F);
					auto midiPC=mksp<MIDIFile::MIDIProgramChange>(
						deltaTicks,
						status,
						progChange->id
					);
					midiEvent=midiPC;
				} else if(eventType == EventType::CHANNEL_PITCH_BEND){
					auto * pitchBend=static_cast<ChannelPitchBend *>(&event);
					uint8_t status=0xE0 | (midiChannel & 0x0F);
					int midiValue=static_cast<int>((pitchBend->value + 1.0f) * 8192.0f);
					midiValue=Util::clamp(midiValue, 0, 16383);
					uint8_t lsb=midiValue & 0x7F;
					uint8_t msb=(midiValue >> 7) & 0x7F;
					auto midiPB=mksp<MIDIFile::MIDIPitchBend>(deltaTicks, status, lsb, msb);
					midiEvent=midiPB;
				} else if(eventType == EventType::CHANNEL_PRESSURE){
					auto * channelPressure=static_cast<ChannelPressure *>(&event);
					uint8_t status=0xD0 | (midiChannel & 0x0F);
					auto midiPressure=mksp<MIDIFile::MIDIChannelPressure>(
						deltaTicks,
						status,
						static_cast<uint8_t>(channelPressure->value * 127)
					);
					midiEvent=midiPressure;
				} else if(eventType == EventType::CHANNEL_CONTROL){
					auto * channelControl=static_cast<ChannelControl *>(&event);
					uint8_t status=0xB0 | (midiChannel & 0x0F);
					auto midiCC=mksp<MIDIFile::MIDIChannelControl>(
						deltaTicks,
						status,
						channelControl->control,
						channelControl->value
					);
					midiEvent=midiCC;
				} else if(eventType == EventType::NOTE_PRESSURE){
					auto * notePressure=static_cast<NotePressure *>(&event);
					uint8_t status=0xA0 | (midiChannel & 0x0F);
					auto midiNote=mksp<MIDIFile::MIDINote>(
						deltaTicks,
						status,
						notePressure->id,
						static_cast<uint8_t>(notePressure->velocity * 127)
					);
					midiEvent=midiNote;
				}
				if(midiEvent){
					track->events.add(midiEvent);
				}
			}
			// 添加音轨结束事件
			auto endMeta=mksp<MIDIFile::FFMessage>(0);
			endMeta->type=0x2F;
			endMeta->b=ByteArray((u_index)0);
			track->events.add(endMeta);

			// 只添加有事件的音轨
			if(track->events.size() > 1){ // 大于1表示除了结束事件外还有其他事件
				midiSeq->tracks.add(track);
			}
		}

		midiSeq->midiTrackCount=midiSeq->tracks.size();

		// 5. 将MIDISequence写入输出流
		if(midiSeq->midiTrackCount > 0){
			midiSeq->toStream(os);
		}
	}
	u_sp<MixerSequence> SynthUtil::parseMIDI(InputStream & is){
		try{
			u_sp<MixerSequence> mixerSequence=mksp<MixerSequence>();
			auto midiSequence=MIDIFile::parse(is);
			if(midiSequence == nullptr)return nullptr;
			int noteShift=0;
			std::vector<u_up<ChannelEvent>> ticks;
			bool isGS=midiSequence->standard == MIDIFile::Standard::ROLAND_GENERAL_STANDARD;
			bool isXG=midiSequence->standard == MIDIFile::Standard::YAMAHA_EXTENDED_GENERAL;
			if(isGS)System::out.println("MIDI GS");
			if(isXG)System::out.println("MIDI XG");
			int32_t mapToDrumDst=0;
			int32_t mapToDrumSrc=0;
			for(u_index ii=0;ii < midiSequence->tracks.size();ii++){
				int midiPort=0;
				auto t=midiSequence->tracks.get(ii);
				double delay=0;
				int delayTicks=0;
				float ticksSecond=0.001f;
				for(u_index jj=0;jj < t->events.size();jj++){
					auto e=t->events.get(jj);
					if(e == nullptr)continue;
					delayTicks+=e->deltaTimeTicks;
					delay+=e->deltaTimeTicks * ticksSecond;
					for(int32_t i=(int32_t)ticks.size() - 1;i >= 0;i--){
						u_up<ChannelEvent> & m=ticks[i];
						TickChange * change=(TickChange *)m.get();
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
						auto mnote=spsc<MIDIFile::MIDINote>(e);
						int channel=mnote->channel;
						if(mapToDrumSrc == channel && IMixer::isDrumSetChannel(mapToDrumDst)){
							channel=mapToDrumDst;
						}
						channel+=midiPort * 16;
						if(mnote->action == 0){
							u_up<ChannelEvent>  n1=mkup< NoteOff>(mnote->id + noteShift, mnote->velocity / 127.0f);
							mixerSequence->postToSequence(channel, std::move(n1), delay);
						} else if(mnote->action == 1){
							u_up<ChannelEvent>  n1=mkup < NoteOn>(mnote->id + noteShift, mnote->velocity / 127.0f);
							mixerSequence->postToSequence(channel, std::move(n1), delay);
						} else if(mnote->action == 2){
							u_up<ChannelEvent>  n1=mkup < NotePressure>(mnote->id + noteShift, mnote->velocity / 127.0f);
							mixerSequence->postToSequence(channel, std::move(n1), delay);
						}
					} else if(et == MIDIFile::EventType::ChannelPitchBend){
						auto pitchBend=spsc<MIDIFile::MIDIPitchBend>(e);
						int channel=pitchBend->channel;
						if(mapToDrumSrc == channel && IMixer::isDrumSetChannel(mapToDrumDst)){
							channel=mapToDrumDst;
						}
						channel+=midiPort * 16;
						u_up<ChannelEvent>  n1=mkup< ChannelPitchBend>((pitchBend->value - 8192.0f) / 8192.0f);
						mixerSequence->postToSequence(channel, std::move(n1), delay);
					} else if(et == MIDIFile::EventType::ChannelPressure){
						auto pressure=spsc<MIDIFile::MIDIChannelPressure>(e);
						int channel=pressure->channel;
						if(mapToDrumSrc == channel && IMixer::isDrumSetChannel(mapToDrumDst)){
							channel=mapToDrumDst;
						}
						channel+=midiPort * 16;
						u_up<ChannelEvent>  n1=mkup< ChannelPressure>(pressure->value / 127.0f);
						mixerSequence->postToSequence(channel, std::move(n1), delay);
					} else if(et == MIDIFile::EventType::ChannelControl){
						auto control=spsc<MIDIFile::MIDIChannelControl>(e);
						int channel=control->channel;
						if(mapToDrumSrc == channel && IMixer::isDrumSetChannel(mapToDrumDst)){
							channel=mapToDrumDst;
						}
						channel+=midiPort * 16;
						u_up<ChannelEvent>  n1=mkup < ChannelControl>(control->control, control->value);
						mixerSequence->postToSequence(channel, std::move(n1), delay);
					} else if(et == MIDIFile::EventType::ProgramChange){
						auto setInstrument=spsc<MIDIFile::MIDIProgramChange>(e);
						int channel=setInstrument->channel;
						if(mapToDrumSrc == channel && IMixer::isDrumSetChannel(mapToDrumDst)){
							channel=mapToDrumDst;
						}
						channel+=midiPort * 16;
						u_up<ChannelEvent>  n1=mkup < ProgramChange>(setInstrument->num);
						mixerSequence->postToSequence(channel, std::move(n1), delay);
					} else if(et == MIDIFile::EventType::Sysex){
						auto sys=spsc<MIDIFile::SysexMessage>(e);
						auto & b=sys->b;
						if(isGS){
							static ByteArray toDrum=ByteArray({0x41, 0x10, 0x42, 0x12, 0x40, 0x1A, 0x15, 0x02, 0x0f, (int8_t)0xf7});
							if(Arrays::equals(toDrum, b)){
								mapToDrumSrc=10;
								mapToDrumDst=256 + 9;
								System::out.println("GS: Mapped Ch 11 To DrumSet");
							} else if(b[2] == 0x42 && (b[3] & 0x10) == 0x10){
								int32_t channel=b[3] - 0x10;
							}
						}
					} else if(et == MIDIFile::EventType::FF){
						auto ffMessage=spsc<MIDIFile::FFMessage>(e);
						if(ffMessage->type == 0x51){
							float newTick=ffMessage->quarterNoteDurationMicroSeconds / (float)midiSequence->ticksForQuarterNote / 1000000.0f;
							u_up<ChannelEvent>  ce=mkup<TickChange>(delayTicks, newTick);
							ce->startAtTime=delay;
							ticks.emplace_back(std::move(ce));
						} else if(ffMessage->type == 0x21){
							midiPort=ffMessage->b[0];
							System::out.printf("MIDI File Port: %d\n", midiPort);
						}
					}
				}
			}
			mixerSequence->sortPosted();
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
	u_up<ChannelEvent>  SynthUtil::MIDIBytes2Event(uint8_t ty, uint8_t data1, uint8_t data2){
		int type=ty & 0b11110000;
		int channelID=ty & 0b1111;
		switch(type){
			case 0x80:
				return mkup< NoteOff>(channelID, data1 & 0xff, (data2 & 0xff) / 127.0f);
			case 0x90:
				return  mkup < NoteOn>(channelID, data1 & 0xff, (data2 & 0xff) / 127.0f);
			case 0xa0:
				return mkup < NotePressure>(channelID, (data1 & 0xff), (data2 & 0xff) / 127.0f);
			case 0xb0:
				return mkup < ChannelControl>(channelID, data1 & 0xff, data2 & 0xff);
			case 0xc0:
				return mkup < ProgramChange>(channelID, data1 & 0xff);
			case 0xd0:
				return mkup < ChannelPressure>(channelID, (data2 & 0xff) / 127.0f);
			case 0xe0:
				return mkup < ChannelPitchBend>(channelID, (((data1 & 0x7f) | ((data2 & 0x7f) << 7)) - 8192.0f) / 8191.0f);
			case 0xf0:
				break;
		}
		return nullptr;
	}
	void SynthUtil::sendMIDIBytes(IMixer * mixer, uint8_t ty, uint8_t data1, uint8_t data2){
		sendMIDIBytes(mixer, ty, data1, data2, Mixer::DEFAULT_MIDI_CHANNEL_GROUP_NAME);
	}
	void SynthUtil::sendMIDIBytes(IMixer * mixer, uint8_t ty, uint8_t data1, uint8_t data2, const String & groupName){
		auto event=MIDIBytes2Event(ty, data1, data2);
		if(event == nullptr)return;
		event->groupName=groupName;
		mixer->sendInstantEvent(std::move(event));
	}
	void SynthUtil::sendMIDIEvent(u_up<ChannelEvent>  event, const String & deviceName){
		if(callback != nullptr)(*callback)(deviceName, Event2MIDIBytes(std::move(event)));
	}
	uint64_t SynthUtil::MergeMIDIBytes(uint8_t ty, uint8_t data1, uint8_t data2){
		return (uint64_t)(ty & 0xFF) |
			((uint64_t)(data1 & 0x7F) << 8) |
			((uint64_t)(data2 & 0x7F) << 16);
	}
	uint64_t SynthUtil::Event2MIDIBytes(u_up<ChannelEvent>  event){
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
		this->index=0;
	}
	FixedRandom::FixedRandom(){
		this->data=SynthUtil::NOISE;
		this->index=0;
	}
	u_sample FixedRandom::next(){
		u_sample d=(*data)[index];
		index=(index + 1) % data->length;
		return d;
	}
	SampleArray * SynthUtil::noise(u_index length, u_sample_rate sampleRate, u_freq f1, u_freq f2){
		SampleArray * randomData=new SampleArray(length);
		u_sp<IIR> iir=IIRUtil::newButterworthIIRFilter(sampleRate, FilterPassType::BANDPASS, 2, f1, f2);
		iir->init(sampleRate);
		Random random(5319539547595419742L);
		for(u_index i=0;i < length;i++){
			(*randomData)[i]=iir->procDsp(random.nextGaussian());
		}
		return randomData;
	}
	void SynthUtil::deleteStatic(){
		delete NOISE;
	}
	u_sp<MixerSequence> SynthUtil::parseXM(InputStream & inputStream){
		u_sp<MixerSequence> mixerSequence=mksp<MixerSequence>();
		u_sp<XMFile::Module> modulep=XMFile::parse(inputStream);
		if(modulep == nullptr)return nullptr;
		XMFile::Module & module1=*modulep;
		double time=0;
		double ticksPerSecond=module1.bpm * 0.4;
		double rowsPerSecond=ticksPerSecond / module1.tempo;
		for(u_index i=0;i < module1.num_channels;i++){
			auto event=mkup< ChannelControl>(MIDIFile::CC::MONO_MODE, 127);
			mixerSequence->postToSequence(i, std::move(event), 0);
			auto event2=mkup< DrumChannel>(false);
			mixerSequence->postToSequence(i, std::move(event2), 0);
		}
		IntArray notePrevInstrument(module1.num_channels);
		//在模式索引表查找
		int nextPatternBreak=-1;//模式跳出，-1禁用，>=0执行跳转
		for(u_index patternTableIndex=0;patternTableIndex < module1.song_length;patternTableIndex++){
			int patternIndex=module1.pattern_table[patternTableIndex];//模式索引
			if(patternIndex >= module1.patterns.size())throw IndexOutOfBoundsException();
			XMFile::Pattern & pattern=module1.patterns[patternIndex];//根据模式索引获取模式
			u_index row;
			if(nextPatternBreak == -1){
				row=0;
			} else{
				row=nextPatternBreak;
				nextPatternBreak=-1;
			}
			//该模式的行
			for(;row < pattern.num_rows;row++){
				if(nextPatternBreak >= 0){
					break;
				}
				time+=1.0 / rowsPerSecond;
				//模块的通道数
				for(u_index channelI=0;channelI < module1.num_channels;channelI++){
					int slotIndex=row * module1.num_channels + channelI;//槽位索引=模块通道数*行索引+当前通道
					int channel=channelI;
					XMFile::PatternSlot & s=pattern.slots[slotIndex];//从模式获取槽位
					if(s.instrument > 0 && notePrevInstrument[channelI] != s.instrument){
						u_up<ChannelEvent>  channelEvent=mkup< ProgramChange>(s.instrument - 1);
						mixerSequence->postToSequence(channel, std::move(channelEvent), time);
						//channelEvent=new ChannelControl(MIDIFile::CC::RESET_MUTE_ALL_NOTES, 127);
						//mixerSequence->postToSequence(channel, channelEvent, time);
						notePrevInstrument[channelI]=s.instrument;
					}
					uint32_t vv=s.volume_column << 16;
					vv|=s.effect_type << 8;
					vv|=s.effect_param;
					u_up<ChannelEvent>  channelEvent=mkup < ChannelControl>(256, vv);
					mixerSequence->postToSequence(channel, std::move(channelEvent), time);

					if(s.effect_type != 0 || s.effect_param != 0){
						switch(s.effect_type){
							case XMFile::EffectType::VIBRATO:
							{
								u_up<ChannelEvent>  channelEvent=mkup < ChannelControl>(MIDIFile::CC::MODULATION, s.effect_param);
								mixerSequence->postToSequence(channel, std::move(channelEvent), time);
							}
							break;
							case XMFile::EffectType::TREMOLO:
							{
								u_up<ChannelEvent>  channelEvent=mkup < ChannelControl>(MIDIFile::CC::MODULATION, s.effect_param);
								mixerSequence->postToSequence(channel, std::move(channelEvent), time);
							}
							break;
							case XMFile::EffectType::PATTERN_BREAK:
							{
								nextPatternBreak=s.effect_param & 0x3F;
							}
							break;
						}
					}
					if(s.effect_type == XMFile::EffectType::SET_SPEED){
						if(s.effect_param >= 32)ticksPerSecond=s.effect_param * 0.4;
						else rowsPerSecond=ticksPerSecond / s.effect_param;
					}
					if(s.note > 0 && s.note < 97){
						if(s.instrument > 0){
							u_up<ChannelEvent>  channelEvent=mkup < NoteOn>(s.note - 1, 1);
							mixerSequence->postToSequence(channel, std::move(channelEvent), time);
						}
					} else if(s.note == 97){
						u_up<ChannelEvent>  channelEvent=mkup < ChannelControl>(MIDIFile::CC::ALL_NOTES_OFF, 127);
						mixerSequence->postToSequence(channel, std::move(channelEvent), time);
					}
				}
			}
		}
		time+=1.0 / rowsPerSecond;
		for(u_index channelI=0;channelI < module1.num_channels;channelI++){
			int channel=channelI;
			u_up<ChannelEvent>  channelEvent=mkup < ChannelControl>(MIDIFile::CC::ALL_NOTES_OFF, 127);
			mixerSequence->postToSequence(channel, std::move(channelEvent), time);
		}
		mixerSequence->setInstrument(mksp<XMInstrument>(modulep));
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