#include "DLSFormatInstrument.h"
#include "SimpleSynth.h"
#include "SynthUtil.h"
#include "array/Array.hpp"
#include "array/Array.hpp"
#include "array/Array.hpp"
#include "array/SampleProvider.h"
#include "collection/ArrayList.hpp"
#include "interface/NoteProcessor.h"
#include "io/InputStream.h"
#include "lang/Math.h"
#include "soundbank/dls/DLSInstrument.h"
#include "soundbank/dls/DLSModulator.h"
#include "soundbank/dls/DLSRegion.h"
#include "soundbank/dls/DLSSample.h"
#include "soundbank/dls/DLSSampleLoop.h"
#include "soundbank/dls/DLSSampleOptions.h"
#include "soundbank/dls/DLSSoundbank.h"
#include "synth/composed/NonInterpolateAmpSet.h"
#include "synth/composed/RegionAmp.h"
#include "synth/envelopers/AHDSREnvelop.h"
#include "synth/envelopers/EnvUtil.h"
#include "synth/generators/sampler/WaveSampler.h"
#include "synth/source/AmpBuilder.h"
#include "synth/source/AmplitudeSources.h"
#include "util/Util.h"
#include "yzrutil.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
using namespace yzrilyzr_util;
using namespace yzrilyzr_collection;
using namespace yzrilyzr_io;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_array;
using namespace yzrilyzr_soundbank;

namespace yzrilyzr_simplesynth{
	u_time_ms DLSFormatInstrument::timeCent2Ms(double tc){
		if(tc <= -32768.0 * 65536.0) return 0.0;
		return pow(2.0, tc / (65536.0 * 1200.0)) * 1000.0;
	}
	u_normal_01 DLSFormatInstrument::scale2Normal01(double scale){
		double level=scale / 65536.0 / 1000.0;  // 0.1% → % → 0~1
		return static_cast<u_normal_01>(Util::clamp01(level));
	}
	u_sp<SampleProvider> DLSFormatInstrument::convertToSample(DLSSample & sample){
		ByteArray * byteArr=sample.getDataBuffer()->array();
		switch(sample.format){
			case DLSSampleFormat::SIGNED:
				if(sample.bits == 16){
					ShortArray arr(byteArr->length / 2);
					for(u_index si=0, bi=0;bi < byteArr->length;){
						uint16_t val=0;
						val=(*byteArr)[bi++] & 0xff;
						val|=((*byteArr)[bi++] & 0xff) << 8;
						arr[si++]=val;
					}
					return mksp<ShortArrayProvider>(arr);
				} else if(sample.bits == 8){
					ByteArray arr(byteArr->length);
					memcpy(arr._array, byteArr->_array, byteArr->length);
					return mksp<ByteArrayProvider>(arr);
				}
				break;

			case DLSSampleFormat::UNSIGNED:
				if(sample.bits == 8){
					ByteArray arr(byteArr->length);
					memcpy(arr._array, byteArr->_array, byteArr->length);
					return mksp<ByteArrayProvider>(arr);
				}
				break;

			case DLSSampleFormat::FLOAT:
				if(sample.bits == 32){
					FloatArray arr(byteArr->length / 4);
					for(u_index si=0, bi=0;bi < byteArr->length;){
						uint32_t val=0;
						val=(*byteArr)[bi++] & 0xff;
						val|=((*byteArr)[bi++] & 0xff) << 8;
						val|=((*byteArr)[bi++] & 0xff) << 16;
						val|=((*byteArr)[bi++] & 0xff) << 24;
						uint32_t * valp=&val;
						arr[si++]=*((float *)valp);
					}
					return mksp<FloatArrayProvider>(arr);
				}
				break;
		}

		return nullptr;
	}
	void DLSFormatInstrument::procModulators(DLSRegion & region, DLSInstrument & instrument, AmpBuilder & builder){
		struct Env{
			u_time_ms delayTime=0.0;
			u_time_ms attackTime=10.0;
			u_time_ms holdTime=0.0;
			u_time_ms decayTime=1000.0;
			u_time_ms releaseTime=100.0;
			double sustainLevel=0.0;
			bool canSustain=false;
		};
		Env eg1, eg2;
		HashMap<int32_t, u_sp<DLSModulator>> currentDstModulators;
		for(auto & modulatorPtr : instrument.getModulators()){
			int32_t destination=modulatorPtr->getDestination();
			currentDstModulators.put(destination, modulatorPtr);
		}
		for(auto & modulatorPtr : region.getModulators()){
			int32_t destination=modulatorPtr->getDestination();
			currentDstModulators.put(destination, modulatorPtr);
		}
		for(auto & ent : currentDstModulators.entries()){
			auto & modulator=*ent.second;
			int32_t source=modulator.getSource();
			int32_t destination=modulator.getDestination();
			int32_t scale=modulator.getScale();
			switch(destination){
				case DLSModulator::CONN_DST_GAIN:
					break;
				case DLSModulator::CONN_DST_PITCH:
					break;
				case DLSModulator::CONN_DST_PAN:
					break;
				case DLSModulator::CONN_DST_LFO_FREQUENCY:
					break;
				case DLSModulator::CONN_DST_LFO_STARTDELAY:
					break;
				case DLSModulator::CONN_DST_EG1_DELAYTIME:
					eg1.delayTime=timeCent2Ms(scale);
					break;
				case DLSModulator::CONN_DST_EG1_ATTACKTIME:
					eg1.attackTime=timeCent2Ms(scale);
					break;
				case DLSModulator::CONN_DST_EG1_DECAYTIME:
					if(source == DLSModulator::CONN_SRC_NONE){
						eg1.decayTime=timeCent2Ms(scale);
					} else{
						eg1.decayTime=timeCent2Ms(scale) * 32;
					}
					break;
				case DLSModulator::CONN_DST_EG1_SUSTAINLEVEL:
					eg1.sustainLevel=scale2Normal01(scale);
					break;
				case DLSModulator::CONN_DST_EG1_RELEASETIME:
					eg1.releaseTime=timeCent2Ms(scale);
					break;
				case DLSModulator::CONN_DST_EG2_DELAYTIME:
					eg2.delayTime=timeCent2Ms(scale);
					break;
				case DLSModulator::CONN_DST_EG2_ATTACKTIME:
					eg2.attackTime=timeCent2Ms(scale);
					break;
				case DLSModulator::CONN_DST_EG2_DECAYTIME:
					eg2.decayTime=timeCent2Ms(scale);
					break;
				case DLSModulator::CONN_DST_EG2_SUSTAINLEVEL:
					eg2.sustainLevel=scale2Normal01(scale);
					break;
				case DLSModulator::CONN_DST_EG2_RELEASETIME:
					eg2.releaseTime=timeCent2Ms(scale);
					break;
				default:
					//std::cout << "DLSFormatInstrument::procModulators: unsupported modulator destination " << destination << std::endl;
					break;
			}
		}
		eg1.canSustain=SynthUtil::isInstrumentProviderSustainable(instrument.getPreset());
		if(eg1.canSustain)eg1.sustainLevel=0.7;
		else eg1.sustainLevel=0;
		//builder.biquadEnv(
		//	AmpBuilder().src(ConstAmp(127)).DAHDSR(
		//		Math::max(0.0, eg2.delayTime),
		//		Math::max(0.0, eg2.attackTime),
		//		Math::max(0.0, eg2.holdTime),
		//		Math::max(0.0, eg2.decayTime),
		//		Util::clamp01(eg2.sustainLevel),
		//		eg2.canSustain,
		//		Math::max(0.0, eg2.releaseTime),
		//		100,//forceRelease
		//		EnvUtil::Line(),
		//		EnvUtil::Line(),
		//		EnvUtil::Line()
		//	).build(), ConstAmp(1), FilterPassType::LOWPASS);
		builder.DAHDSR(
			Math::max(0.0, eg1.delayTime),
			Math::max(0.0, eg1.attackTime),
			Math::max(0.0, eg1.holdTime),
			Math::max(0.0, eg1.decayTime),
			Util::clamp01(eg1.sustainLevel),
			eg1.canSustain,
			Math::max(0.0, eg1.releaseTime),
			EnvUtil::Pow(5),  // 攻击曲线
			EnvUtil::Pow(5),  // 衰减曲线
			EnvUtil::Pow(5)   // 释放曲线
		);
	}
	float DLSFormatInstrument::scaleToFloat(int32_t scale){
   // 1. 解析整数部分：高16位，直接作为有符号整数（保留符号位）
		int16_t intPart=static_cast<int16_t>(scale >> 16);  // 关键：用int16_t接收，自动处理符号
		// 2. 解析小数部分：低16位，无符号（0~65535 → 0.0~1.0）
		uint16_t fracPart=static_cast<uint16_t>(scale & 0xFFFF);
		// 3. 合并：整数部分 + 小数部分/65536.0
		return static_cast<float>(intPart) + (fracPart / 65536.0f);
	}
	DLSFormatInstrument::DLSFormatInstrument(InputStream & inputStream){
		soundbank=mksp<DLSSoundbank>(inputStream);
		buildInstruments();
		soundbank=nullptr;
	}

	DLSFormatInstrument::~DLSFormatInstrument(){}

	NoteProcPtr DLSFormatInstrument::get(s_bank_id bank, s_program_id program, u_sample_rate sampleRate){
		auto it=programMap.find({bank, program});
		if(it != programMap.end()){
			NoteProcPtr p=it->second;
			if(p != nullptr) return p;
		}
		it=programMap.find({0, program});
		if(it != programMap.end()){
			NoteProcPtr p=it->second;
			if(p != nullptr) return p;
		}
		return nullptr;
	}

	NoteProcPtr DLSFormatInstrument::getDrumSet(s_bank_id bank, u_sample_rate sampleRate){
		auto it=drumSetMap.find(bank);
		if(it != drumSetMap.end()){
			NoteProcPtr p=it->second;
			if(p != nullptr) return p;
		}
		it=drumSetMap.find(0);
		if(it != drumSetMap.end()){
			NoteProcPtr p=it->second;
			if(p != nullptr) return p;
		}
		return nullptr;
	}
	void DLSFormatInstrument::buildInstruments(){
		auto instruments=soundbank->getInstruments();
		for(auto & instrument : instruments){
			auto dlsInstrument=spdc<DLSInstrument>(instrument);
			if(!dlsInstrument)continue;
			if(dlsInstrument->druminstrument){
				buildDrumInstrument(*dlsInstrument);
			} else{
				buildMelodicInstrument(*dlsInstrument);
			}
		}
	}

	void DLSFormatInstrument::buildMelodicInstrument(DLSInstrument & instrument){
		auto noteSrc=mksp<RegionAmp>();

		for(auto & region : instrument.getRegions()){
			AmpBuilder builder;
			createSampler(*region, builder);
			procModulators(*region, instrument, builder);
			auto envelopedSampler=builder.build();
			// 设置键位范围
			int keyFrom=region->getKeyfrom();
			int keyTo=region->getKeyto();
			int velFrom=region->getVelfrom();
			int velTo=region->getVelto();
			noteSrc->put(keyFrom, keyTo, velFrom, velTo, envelopedSampler);
		}
		noteSrc->build();
		programMap[{instrument.getBank(), instrument.getPreset()}]=noteSrc;
	}

	void DLSFormatInstrument::buildDrumInstrument(DLSInstrument & instrument){
		s_bank_id bank=instrument.getBank();
		u_sp<NonInterpolateAmpSet> drumSet=spdc<NonInterpolateAmpSet>(getDrumSet(bank, 0));
		if(drumSet == nullptr){
			drumSet=mksp<NonInterpolateAmpSet>();
			drumSetMap[bank]=drumSet;
		}
		for(auto & region : instrument.getRegions()){
			AmpBuilder builder;
			createSampler(*region, builder);
			procModulators(*region, instrument, builder);
			auto envelopedSampler=builder.build();
			// 鼓组使用特定的键位
			int key=region->getKeyfrom(); // 通常鼓组每个区域对应一个键位
			int keyTo=region->getKeyto();
			if(!drumSet->has(key))drumSet->add(key, envelopedSampler);
		}
	}

	void DLSFormatInstrument::createSampler(DLSRegion & region, AmpBuilder & builder){
		auto samplePtr=region.getSample();
		if(!samplePtr) return;
		DLSSample & sample=*samplePtr;
		auto optionsPtr=region.getSampleoptions();
		if(!optionsPtr)optionsPtr=sample.getSampleoptions();
		if(!optionsPtr) return;
		DLSSampleOptions & options=*optionsPtr;
		auto data=sample.getDataBuffer();
		if(!data) return;
		auto sampleData=convertToSample(sample);
		if(!sampleData) return;
		// 设置循环信息
		int loopStart=0;
		int loopEnd=0;
		int loopType=WaveSampler::LOOP_DISABLE;
		if(!options.getLoops().isEmpty()){
			auto & loop=options.getLoops()[0];
			loopStart=loop->getStart();
			loopEnd=loop->getStart() + loop->getLength();
			loopType=(loop->getType() == DLSSampleLoop::LOOP_TYPE_FORWARD)?
				WaveSampler::LOOP_LOOP:WaveSampler::LOOP_DISABLE;
		}
		WaveSamplerBuilder wb;
		wb.sample(sampleData);
		if(loopType != WaveSampler::LOOP_DISABLE){
			wb.loop(loopStart, loopEnd, loopType);
		}
		// 计算音高校正
		double pitchCorrection=0.0;
		if(options.unitynote > 0){
			s_note_id nid=(s_note_id)options.unitynote - options.finetune / 100.0;
			u_freq originalFreq=tuning.getFrequencyByID(nid);
			wb.sampleFreq(originalFreq, sample.samplerate);
		} else{
			u_freq originalFreq=tuning.getFrequencyByID(60);
			wb.sampleFreq(originalFreq, sample.samplerate);
		}
		builder.src(wb.build());
		if(options.attenuation != 0){
			double volumeAttenuationDB=options.attenuation / 65536.0 / 100.0;
			double volumeMultiplier=pow(10.0, -volumeAttenuationDB / 20.0);
			builder.mul(volumeMultiplier);
		}
	}
}