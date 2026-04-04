#include "XMInstrument.h"
#include "array/Array.hpp"
#include "events/ChannelConfig.h"
#include "events/ChannelEvent.h"
#include "events/NoteUpdater.h"
#include "synth/enveloper/AHDSREnvelop.h"
#include "synth/enveloper/FadeOutEnvelop.h"
#include "synth/enveloper/GraphEnvelop.h"
#include "synth/util/AmplitudeSources.h"
#include "synth/util/EnvUtil.h"
#include "util/MIDIFile.h"
using namespace yzrilyzr_array;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	XMInstrument::XMInstrument(u_sp<XMFile::Module> mod) :mod(mod){
		for(u_index i=0;i < mod->instruments.size();i++){
			insts.add(mksp<XMNoteProcessor>(mod, &mod->instruments[i], i));
		}
	}

	
		XMNoteProcessor::XMNoteProcessor(u_sp<XMFile::Module> mod, XMFile::Instrument * instrument, int ii) :mod(mod), xmInstrument(instrument), instrumentInModIndex(ii){
			samples.reserve(instrument->num_samples);
			XMFile::Envelop & volEnv=xmInstrument->volume_envelope;
			if(volEnv.enabled){
				DoubleArray pts(volEnv.num_points * 2);
				for(u_index i=0;i < volEnv.num_points;i++){
					XMFile::EnvelopPoint & envelopPoint=volEnv.points[i];
					pts[i * 2]=(u_time)envelopPoint.frame * 1000.0 / (mod->bpm * 0.4);
					pts[i * 2 + 1]=envelopPoint.value / 64.0;
				}
				volEnvelop=mksp<GraphEnvelop>(volEnv.sustain_enabled?volEnv.sustain_point:-1,
											  volEnv.loop_enabled?volEnv.loop_start_point:-1,
											  volEnv.loop_enabled?volEnv.loop_end_point:-1,
											  pts);
				fadeoutEnvelop=mksp<FadeOutEnvelop>((float)xmInstrument->volume_fadeout * 2.0f / 0xfff, FadeOutEnvelop::OnNoteOff);
			} else{
				volEnvelop=mksp<AHDSREnvelop>(0, 1, 1, 1, 1, true, 100, 50, Pow(-5), Pow(5), Pow(5));
			}
			XMFile::Envelop & panEnv=xmInstrument->panning_envelope;
			if(panEnv.enabled){
				DoubleArray pts(panEnv.num_points * 2);
				for(u_index i=0;i < panEnv.num_points;i++){
					XMFile::EnvelopPoint & envelopPoint=panEnv.points[i];
					pts[i * 2]=(u_time)envelopPoint.frame * 1000.0 / (mod->bpm * 0.4);
					pts[i * 2 + 1]=(envelopPoint.value - 32) / 32.0;
				}
				panEnvelop=mksp< GraphEnvelop>(panEnv.sustain_enabled?panEnv.sustain_point:-1,
											   panEnv.loop_enabled?panEnv.loop_start_point:-1,
											   panEnv.loop_enabled?panEnv.loop_end_point:-1,
											   pts);
			}
			for(u_index i=0;i < instrument->num_samples;i++){
				XMFile::SampleData & xsample=xmInstrument->samples[i];
				double noteOffset=xsample.relative_note + xsample.finetune / 128.0 + 12;
				if(xsample.loop_type == XMFile::Loop::XM_NO_LOOP){
					double loopFreq;
					loopFreq=1 / 32.0;
					loopFreq*=(pow(2, -noteOffset / 12));
					auto & paramRegPtr=WaveSamplerBuilder()
						.sampleFreq(loopFreq, 1)
						.loop(xsample.loop_start, xsample.loop_end, getLoopType(xsample.loop_type));
					if(xsample.bits == 8)paramRegPtr.sample(xsample.data8);
					else if(xsample.bits == 16)paramRegPtr.sample(xsample.data16);
					samples.emplace_back(paramRegPtr.build() * xsample.volume);
				} else{
					double loopFreq;
					loopFreq=1 / 32.0;
					loopFreq*=(pow(2, -noteOffset / 12));
					auto & paramRegPtr=WaveSamplerBuilder()
						.sampleFreq(loopFreq, 1)
						.loop(xsample.loop_start, xsample.loop_end, getLoopType(xsample.loop_type));
					if(xsample.bits == 8)paramRegPtr.sample(xsample.data8);
					else if(xsample.bits == 16)paramRegPtr.sample(xsample.data16);
					samples.emplace_back(paramRegPtr.build() * xsample.volume);
				}
			}

		}
		void XMNoteProcessor::init(ChannelConfig & cfg){
			volEnvelop->init(cfg);
			if(fadeoutEnvelop != nullptr)fadeoutEnvelop->init(cfg);
			if(panEnvelop != nullptr)panEnvelop->init(cfg);
		}
		u_sample XMNoteProcessor::getAmp(const Note & note){
			if(samples.size() == 0) return 0;
			u_time deltaTime=note.cfg->deltaTime;

			//根据音符id获取采样索引
			int sampleIndex=(xmInstrument->sample_of_notes)[note.id];
			//计算效果
			Note & n=const_cast<Note &>(note);
			if(currentVolCol != 0){
				if(currentVolCol >= 0x10 && currentVolCol <= 0x50){
					n.velocity=Math::sqrt((float)(currentVolCol - 0x10) / 0x40);
				} else{
					switch(currentVolCol >> 4){
						case XMFile::VolumeColumn::VOLUME_SLIDE_UP:
						{
							n.velocity+=(float)(currentVolCol & 0xf) * 0.5 / 0xf;
							n.velocity=Util::clamp01(n.velocity);
							currentVolCol=0;
						}
						break;
						case XMFile::VolumeColumn::VOLUME_SLIDE_DOWN:
						{
							n.velocity-=(float)(currentVolCol & 0xf) * 0.5 / 0xf;
							n.velocity=Util::clamp01(n.velocity);
							currentVolCol=0;
						}
						break;
					}
				}
				//重置
			}
			if(currentEffect != 0 || effectArg != 0){
				switch(currentEffect){
					case XMFile::EffectType::SET_VOLUME:
					{
						n.velocity=Math::sqrt((float)effectArg / 127);
					}
					break;
					case XMFile::EffectType::ARPEGGIO://琶音
					{
						int whichNote=((int)(note.passedTime * mod->bpm * 0.4)) % 3;
						if(whichNote == 0)arpeggioIDDelta=0;
						else if(whichNote == 1)arpeggioIDDelta=(effectArg >> 4) & 0xf;
						else if(whichNote == 2)arpeggioIDDelta=effectArg & 0xf;
					}
					break;
					case XMFile::EffectType::VOLUME_SLIDE://指定速率音量滑动
					{
						int x=(effectArg >> 4) & 0xf;
						int y=effectArg & 0xf;
						if(y != 0){
							n.velocity-=(u_sample)y * deltaTime * mod->bpm * (0.4 / 64);
						} else if(x != 0){
							n.velocity+=(u_sample)x * deltaTime * mod->bpm * (0.4 / 64);
						}
						n.velocity=Util::clamp01(n.velocity);
					}
					break;
					case XMFile::EffectType::PORTAMENTO_UP://指定速率上升音高
					{
						portamentoIDDelta+=(s_note_vel)effectArg / 16.0 * (mod->bpm * 0.4) * deltaTime;
					}
					break;
					case XMFile::EffectType::PORTAMENTO_DOWN://指定速率下滑音高
					{
						portamentoIDDelta-=(s_note_vel)effectArg / 16.0 * (mod->bpm * 0.4) * deltaTime;
					}
					break;
					case XMFile::EffectType::TONE_PORTAMENTO://指定速率从前一个音高滑至当前指定音高
					{
						if(n.dataInvalidated){
							portamentoIDDelta=n.lastPortamentoID - n.id;
						}
						double slidePerTick=effectArg / 16.0;
						double slidePerSec=slidePerTick * (mod->bpm * 0.4);
						double step=slidePerSec * deltaTime;

						if(portamentoIDDelta > 0){
							portamentoIDDelta-=step;
							if(portamentoIDDelta < 0) portamentoIDDelta=0;
						} else if(portamentoIDDelta < 0){
							portamentoIDDelta+=step;
							if(portamentoIDDelta > 0) portamentoIDDelta=0;
						}
					}
					break;
					case XMFile::EffectType::EXTENDED_TYPE:
					{
						switch(effectArg >> 4){
							case 9://重触发
							{
								float val=(float)(effectArg & 0xf) / (mod->bpm * 0.4);
								if(n.passedTime > val){
									NoteUpdater::noteOn(n, *n.cfg, n.id, 1);
								}
							}
							break;
						}
					}
					break;
				}
			}
			n.pitchBend=portamentoIDDelta + arpeggioIDDelta;
			//生成采样数据
			double sampleData=samples[sampleIndex]->getAmp(note);
			//生成音量包络
			double volEnv=volEnvelop->getAmp(note);
			if(fadeoutEnvelop != nullptr)volEnv*=fadeoutEnvelop->getAmp(note);
			//执行Pan设置
			if(panEnvelop != nullptr){
				double panEnv=panEnvelop->getAmp(note);
				note.cfg->Pan=panEnv;
			}
			return volEnv * sampleData;
		}
		void XMNoteProcessor::cc(ChannelConfig & cfg, ChannelControl & cc){
			if(cc.control == 256){
				uint32_t v=cc.value;
				currentVolCol=(v >> 16) & 0xff;
				currentEffect=(v >> 8) & 0xff;
				effectArg=v & 0xff;
				cfg.Pan=0;
				if(currentVolCol >> 4 == 0xc){
					int p=(currentVolCol & 0xf) - 8;
					if(p == 0)cfg.Pan=0;
					else if(p < 0)cfg.Pan=(float)p / 8;
					else cfg.Pan=(float)p / 7;
				}
				if(currentEffect == XMFile::EffectType::SET_PANNING){
					int p=effectArg - 0x7f;
					if(p == 0)cfg.Pan=0;
					else if(p < 0)cfg.Pan=(float)p / 0x7f;
					else cfg.Pan=(float)p / 0x7f;
				}
			}
		}
		void XMNoteProcessor::noteOn(ChannelConfig & cfg, s_note_id_i id, s_note_vel vel){
			portamentoIDDelta=0;
			arpeggioIDDelta=0;
		}

		int XMNoteProcessor::getLoopType(XMFile::Loop loop_type){
			switch(loop_type){
				case XMFile::Loop::XM_NO_LOOP:
					return WaveSampler::LOOP_DISABLE;
				case XMFile::Loop::XM_FORWARD_LOOP:
					return WaveSampler::LOOP_LOOP;
				case XMFile::Loop::XM_PING_PONG_LOOP:
					return WaveSampler::LOOP_PING_PONG;
				default:
					throw Exception();
			}
		}
		bool XMNoteProcessor::noMoreData(const Note & note)const{
			if(note.fclosed(*note.cfg))return true;
			if(fadeoutEnvelop != nullptr)return fadeoutEnvelop->noMoreData(note);
			return volEnvelop->noMoreData(note);
		}
		NoteProcPtr XMNoteProcessor::clone(){
			auto p=mksp<XMNoteProcessor>();
			p->xmInstrument=xmInstrument;
			p->samples=samples;
			p->volEnvelop=volEnvelop?volEnvelop->clone():nullptr;
			p->panEnvelop=panEnvelop?panEnvelop->clone():nullptr;
			p->fadeoutEnvelop=fadeoutEnvelop?fadeoutEnvelop->clone():nullptr;
			p->instrumentInModIndex=instrumentInModIndex;
			p->mod=mod;
			p->currentEffect=currentEffect;
			p->effectArg=effectArg;
			p->currentVolCol=currentVolCol;
			p->portamentoIDDelta=portamentoIDDelta;
			p->arpeggioIDDelta=arpeggioIDDelta;
			return p;
		}
	
}