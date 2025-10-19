#include "XMNoteProcessor.h"
#include "array/Array.hpp"
#include "events/ChannelConfig.h"
#include "events/ChannelEvent.h"
#include "synth/envelopers/AHDSREnvelop.h"
#include "synth/envelopers/EnvUtil.h"
#include "synth/envelopers/GraphEnvelop.h"
#include "util/MIDIFile.h"
using namespace yzrilyzr_array;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	XMNoteProcessor::XMNoteProcessor(std::shared_ptr<XMFile::Module> mod, XMFile::Instrument * instrument, int ii) :mod(mod), xmInstrument(instrument), index(ii){
		samples.reserve(instrument->num_samples);
		double ticksPerSecond=mod->bpm * 0.4;
		XMFile::Envelop & volEnv=xmInstrument->volume_envelope;
		if(volEnv.enabled){
			DoubleArray pts(volEnv.num_points * 2);
			for(u_index i=0;i < volEnv.num_points;i++){
				XMFile::EnvelopPoint & envelopPoint=volEnv.points[i];
				pts[i * 2]=envelopPoint.frame * 1000.0 / ticksPerSecond;
				pts[i * 2 + 1]=envelopPoint.value / 64.0;
			}
			volEnvelop=std::make_shared<GraphEnvelop>(volEnv.sustain_enabled?volEnv.sustain_point:-1,
													  volEnv.loop_enabled?volEnv.loop_start_point:-1,
													  volEnv.loop_enabled?volEnv.loop_end_point:-1,
													  pts);
		} else{
			volEnvelop=std::make_shared<AHDSREnvelop>(0, 1, 1, 1, 1, true, 100, 50, Pow(-5), Pow(5), Pow(5));
		}
		XMFile::Envelop & panEnv=xmInstrument->panning_envelope;
		if(panEnv.enabled){
			DoubleArray pts(panEnv.num_points * 2);
			for(u_index i=0;i < panEnv.num_points;i++){
				XMFile::EnvelopPoint & envelopPoint=panEnv.points[i];
				pts[i * 2]=envelopPoint.frame * 1000.0 / ticksPerSecond;
				pts[i * 2 + 1]=(envelopPoint.value - 32) / 32.0;
			}
			panEnvelop=std::make_shared< GraphEnvelop>(panEnv.sustain_enabled?panEnv.sustain_point:-1,
													   panEnv.loop_enabled?panEnv.loop_start_point:-1,
													   panEnv.loop_enabled?panEnv.loop_end_point:-1,
													   pts);
		}
		for(u_index i=0;i < instrument->num_samples;i++){
			XMFile::SampleData & xsample=xmInstrument->samples[i];
			/*std::shared_ptr<SampleArray> sData=std::make_shared<SampleArray>(xsample.length);
			if(xsample.bits == 8){
				for(u_index j=0;j < sData->length;j++){
					(*sData)[j]=(*xsample.data8)[j] / 127.0 * xsample.volume;
				}
			} else if(xsample.bits == 16){
				for(u_index j=0;j < sData->length;j++){
					(*sData)[j]=(*xsample.data16)[j] / 32767.0 * xsample.volume;
				}
			}*/
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
				samples.emplace_back(paramRegPtr.build());
			} else{
				double loopFreq;
				loopFreq=1 / 32.0;
				loopFreq*=(pow(2, -noteOffset / 12));
				auto & paramRegPtr=WaveSamplerBuilder()
					.sampleFreq(loopFreq, 1)
					.loop(xsample.loop_start, xsample.loop_end, getLoopType(xsample.loop_type));
				if(xsample.bits == 8)paramRegPtr.sample(xsample.data8);
				else if(xsample.bits == 16)paramRegPtr.sample(xsample.data16);
				samples.emplace_back(paramRegPtr.build());
			}
		}
	}
	u_sample XMNoteProcessor::getAmp(Note & note){
		if(samples.size()== 0) return 0;
		int sampleIndex=(*xmInstrument->sample_of_notes)[note.id];
		WaveSampler & sampler=*samples[sampleIndex];
		double sampleData=sampler.getAmp(note);
		double volEnv=volEnvelop->getAmp(note);
		if(note.closed(*note.cfg)){
			double closedPassed=note.closedPassedTime;
			volEnv*=envelopeValue(xmInstrument->volume_fadeout, closedPassed);
		}
		if(panEnvelop != nullptr){
			double panEnv=panEnvelop->getAmp(note);
			note.cfg->Pan=panEnv;
		}
		/*if(portamentoSpeed != 0){
			note.freqTimeSynth+=portamentoSpeed;
		}*/
		return volEnv * sampleData;
	}
	void XMNoteProcessor::cc(ChannelConfig & cfg, ChannelControl & cc){
		switch(cc.control){
			case XMFile::CC::PORTAMENTO_UP:
				portamentoSpeed=cc.value / 127.0;
				break;
			case XMFile::CC::PORTAMENTO_DOWN:
				portamentoSpeed=-cc.value / 127.0;
				break;
			case MIDIFile::CC::PORTAMENTO_SWITCH:
				if(cc.value < 64)portamentoSpeed=0;
				break;
		}
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
	double XMNoteProcessor::calculateDecayRate(int fadeout){
		if(fadeout <= 0){
			return 0.0; // fadeout 为 0 时，包络不变
		}
		// 固定点 fadeout = 2048 对应的衰减速率
		double referenceFadeout=2048.0;
		double targetDecayAtReference=0.001;
		double timeAtReference=0.3; // 秒
		// 计算参考点的衰减速率 k
		double kReference=-std::log(targetDecayAtReference) / timeAtReference;
		// 比例缩放 kReference 以适应不同的 fadeout 值
		double k=(fadeout / referenceFadeout) * kReference;
		return k;
	}
	double XMNoteProcessor::envelopeValue(int fadeout, double time){
		double k=calculateDecayRate(fadeout);
		return std::exp(-k * time);
	}
	NoteProcPtr XMNoteProcessor::clone(){
		std::shared_ptr<XMNoteProcessor> c=std::make_shared<XMNoteProcessor>();
		c->xmInstrument=xmInstrument;
		c->samples=samples;
		c->volEnvelop=volEnvelop->clone();
		if(panEnvelop != nullptr)c->panEnvelop=panEnvelop->clone();
		c->index=index;
		c->mod=mod;
		c->portamentoSpeed=portamentoSpeed;
		return c;
	}
}