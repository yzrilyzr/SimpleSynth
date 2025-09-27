#include "KarplusStrongSrc.h"
#include "events/NoteUpdater.h"
#include "dsp/BufferDelayer.h"
#include "dsp/InterpolateFunction.h"

using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	KarplusStrongSrc::KarplusStrongSrc() :Osc(nullptr){
		registerParamNormal01("Alpha", &alpha);
		registerParamSrc("Burst", &burst);
		registerParamSample("BurstSample", &burstSampleData);
		registerParamInterpolator("AlphaInterpolator", &alphaInterpolator);
	}
	KarplusStrongSrcKeyData * KarplusStrongSrc::init(KarplusStrongSrcKeyData * data, Note & note){
		if(data == nullptr){
			data=new KarplusStrongSrcKeyData();
		}
		if(alphaInterpolator != nullptr){
			data->alpha=alphaInterpolator->y(note.id);
		} else{
			data->alpha=alpha;
		}
		RingBufferSample & buffer=data->buffer;
		buffer.reset();
		buffer.fill(0);
		u_freq freq=getInitSetFreq(note);
		double bufferLength=note.cfg->sampleRate / freq;
		size_t lengthInt=(size_t)ceil(bufferLength) + 3;
		buffer.ensureCapacity(bufferLength);
		if(burstSampleData != nullptr){
			auto & data=*burstSampleData;
			double dataLen=data.length;
			for(size_t i=0;i < lengthInt;i++){
				double dIndex=(double)i * dataLen / bufferLength;
				int32_t iIndex=(int32_t)dIndex;
				s_phase y0=data[RingBufferUtil::mod(iIndex - 1, dataLen)];
				s_phase y1=data[RingBufferUtil::mod(iIndex, dataLen)];
				s_phase y2=data[RingBufferUtil::mod(iIndex + 1, dataLen)];
				s_phase y3=data[RingBufferUtil::mod(iIndex + 2, dataLen)];
				u_sample val=InterpolateFunction::cubicSpline(dIndex, iIndex, y0, y1, y2, y3);
				buffer.write(val * note.velocitySynth);
			}
		} else if(burst != nullptr){
			Note initNote(note.uniqueID);
			initNote.set(note);
			ChannelConfig & cfg=*note.cfg;
			u_time cur=cfg.currentTime;
			u_time dt=cfg.deltaTime;
			NoteProcessor & pro=*burst;
			for(size_t i=0;i < lengthInt;i++){
				NoteUpdater::preUpdateNote(initNote, cfg);
				buffer.write(pro.getAmp(initNote));
				NoteUpdater::postUpdateNote(initNote, cfg);
				cfg.currentTime+=dt;
			}
			cfg.currentTime=cur;
		} else{
			static thread_local size_t randomIndex=0;
			for(size_t i=0;i < lengthInt;i++){
				buffer.write(random.next(&randomIndex) * note.velocitySynth);
			}
		}
		return data;
	}
	u_freq KarplusStrongSrc::getInitSetFreq(Note & note){
		u_freq freq2=constFreq;
		if(freq2 == 0) freq2=note.freqSynth;
		return freq2;
	}
	void KarplusStrongSrc::init(ChannelConfig & cfg){
		Osc::init(cfg);
		if(burst != nullptr) burst->init(cfg);
	}
	NoteProcPtr KarplusStrongSrc::clone(){
		return KarplusStrongBuilder()
			.alpha(alpha)
			.alpha(alphaInterpolator)
			.burst(burstSampleData)
			.burst(burst)
			.constFreq(constFreq)
			.freqSrc(getPhaseSource())
			.build();
	}
	u_sample KarplusStrongSrc::getAmp(Note & note){
		KarplusStrongSrcKeyData & data=*getData(note);
		RingBufferSample & buffer=data.buffer;
		u_freq freq2=getSetFreq(note);
		double len=RingBufferUtil::freq2delayIndex(freq2, note.cfg->sampleRate);
		buffer.ensureCapacity(len);
		u_sample delayed=BufferDelayer::cubicSplineDelay(buffer, len);
		u_sample newest=buffer.newest();
		double alpha2=pow(data.alpha, Util::clamp(len, 0.0, 800.0) / 367.0);
		u_sample sum=newest * (1 - alpha2) + delayed * alpha2;
		buffer.write(sum);
		return sum;
	}
	u_freq KarplusStrongSrc::getSetFreq(Note & note){
		return constFreq == 0?note.freqSynth:constFreq;
	}
	std::string KarplusStrongSrc::toString()const{
		return StringFormat::object2string("KarplusStrong",
										   getPhaseSource(), alpha,
										   alphaInterpolator,
										   burstSampleData,
										   burst,
										   constFreq);
	}
}