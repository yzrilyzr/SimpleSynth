#include "SF2FormatInstrument.h"
#include "synth/generators/sampler/WaveSampler.h"
#include "synth/envelopers/EnvUtil.h"
#include "synth/source/AmpBuilder.h"

#include "instrument/SimpleWaveTable.h"
#include "synth/composed/AmpAdder.h"
#include "util/Util.h"
#include "lang/Boxing.h"
#include "lang/System.h"
#include "dsp/PCM.h"
#include "collection/HashMap.hpp"

using namespace yzrilyzr_util;
using namespace yzrilyzr_collection;
using namespace yzrilyzr_io;
using namespace yzrilyzr_soundbank;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_array;

namespace yzrilyzr_simplesynth{
	double SF2FormatInstrument::time(double in){
		return Util::clamp(pow(2, in / 1200.0) * 1000.0, 1.0, 100000.0);
	}
	int32_t SF2FormatInstrument::getGeneratorValue(int id, SF2LayerRegion & lregion, SF2InstrumentRegion & instRegion, SF2Layer & layer, SF2Instrument & in){
		auto g1=lregion.getGenerators();
		auto res=g1->get(id);
		if(res.has_value()) return res.value();
		//
		g1=instRegion.getGenerators();
		res=g1->get(id);
		if(res.has_value()) return res.value();
		//
		auto region1=layer.getGlobalRegion();
		if(region1 != nullptr){
			g1=region1->getGenerators();
			res=g1->get(id);
			if(res.has_value()) return res.value();
		}
		region1=in.getGlobalRegion();
		//
		if(region1 != nullptr){
			g1=region1->getGenerators();
			res=g1->get(id);
			if(res.has_value()) return res.value();
		}
		return NOT_FOUND;
	}
	int32_t SF2FormatInstrument::getGeneratorValue(int32_t id, std::shared_ptr<HashMap<int32_t, int16_t>> g1){
		auto res=g1->get(id);
		if(res.has_value()) return res.value();
		return NOT_FOUND;
	}
	SF2FormatInstrument::~SF2FormatInstrument(){}
	SF2FormatInstrument::SF2FormatInstrument(InputStream & inputStream){
		sbx=std::make_shared<SF2Soundbank>(inputStream);
		inputStream.close();
		for(auto & in : sbx->instruments){
			if(in->getBank() == 128) putDrumSet(*in);
			else putInst(*in);
		}
		sbx=nullptr;
	}
	NoteProcPtr SF2FormatInstrument::get(s_bank_id bank, s_program_id program, u_sample_rate sampleRate){
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

	NoteProcPtr SF2FormatInstrument::getDrumSet(s_bank_id bank, u_sample_rate sampleRate){
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
	void SF2FormatInstrument::putInst(SF2Instrument & instr){
		std::shared_ptr<RegionAmp> src1=std::make_shared<RegionAmp>();
		programMap[{instr.getBank(), instr.getPreset()}]=src1;
		for(std::shared_ptr<SF2InstrumentRegion> instrumentRegion : instr.getRegions()){
			SF2Layer * layer=instrumentRegion->getLayer();
			for(std::shared_ptr<SF2LayerRegion> r1 : layer->getRegions()){
				putLayerRegion(*src1, instr, *instrumentRegion, *r1, *layer);
			}
		}
		src1->build();
	}
	void SF2FormatInstrument::putDrumSet(SF2Instrument & instr){
		auto drums=std::make_shared<NonInterpolateAmpSet>();
		drumSetMap[instr.getPreset()]=drums;
		EqualTemperament tuning;
		for(std::shared_ptr<SF2InstrumentRegion> instrumentRegion : instr.getRegions()){
			SF2Layer * layer=instrumentRegion->getLayer();
			for(std::shared_ptr<SF2LayerRegion> r1 : layer->getRegions()){
				SF2Sample * sample=r1->getSample();
				int type=sample->getSampleType();
				uint8_t op=0;
				ModelByteBuffer * db=sample->getDataBuffer();
				auto sho=r1->getGenerators()->get(43);//L H键
				if(sho.has_value()){
					op=sho.value() & 0xff;
				}
				if(op == 0){
					sho=r1->getGenerators()->get(58);//根音
					if(sho.has_value()) op=sho.value();
				}
				if(op == 0) continue;
				drums->add(op, WaveSamplerBuilder()
						   .sampleFreq(tuning.getFrequencyByID(op),
									   (u_sample_rate)sample->getSampleRate())
						   .sample(getSampleProvider(*db))
						   .build());
			}
		}
	}
	int32_t SF2FormatInstrument::getOriginalPitch(SF2LayerRegion & layerRegion){
		SF2Sample * sample=layerRegion.getSample();
		int32_t op=sample->getOriginalPitch();
		auto layerRegionGenerators=layerRegion.getGenerators();
		if(op == 0 || op == 60){
			int32_t b=getGeneratorValue(SF2Region::GENERATOR_OVERRIDINGROOTKEY, layerRegionGenerators);
			if(b != NOT_FOUND) return b;
			int32_t lhKey=getGeneratorValue(SF2Region::GENERATOR_KEYRANGE, layerRegionGenerators);
			if(lhKey != NOT_FOUND){
				int32_t lKey=lhKey & 0xff;
				int32_t hKey=(lhKey >> 8) & 0xff;
				if(lKey == hKey)return lKey;
				if(op == lKey || op == hKey)return op;
			}
			return NOT_FOUND;
		}
		return op;
	}
	std::shared_ptr<SampleProvider> SF2FormatInstrument::getSampleProvider(ModelByteBuffer & buf){
		int64_t length=buf.capacity();
		int64_t offset=buf.arrayOffset();
		int64_t hash=offset ^ length;
		auto it=sampleMap.find(hash);
		if(it == sampleMap.end()){
			int8_t * byteArr=buf.array()->_array;
			ShortArray arr(length / 2);
			for(u_index si=0, bi=offset;bi < offset + length;){
				uint16_t val=0;
				val=byteArr[bi++] & 0xff;
				val|=(byteArr[bi++] & 0xff) << 8;
				arr[si++]=val;
			}
			it=sampleMap.emplace(hash, std::make_shared<ShortArrayProvider>(arr)).first;
		}
		return it->second;
	}
	void SF2FormatInstrument::putLayerRegion(RegionAmp & amp, SF2Instrument & in, SF2InstrumentRegion & iReg, SF2LayerRegion & lReg, SF2Layer & layer){
		int32_t op=getOriginalPitch(lReg);
		if(op == NOT_FOUND)return;
		SF2Sample * sample=lReg.getSample();
		int32_t type=sample->getSampleType();
		ModelByteBuffer * buf=sample->getDataBuffer();
		int32_t sloop=(int32_t)sample->getStartLoop();
		int32_t eloop=(int32_t)sample->getEndLoop();
		int32_t sampleMode=getGeneratorValue(SF2Region::GENERATOR_SAMPLEMODES, lReg, iReg, layer, in);
		bool sustainable=sampleMode == 1;
		double attack=getGeneratorValue(SF2Region::GENERATOR_ATTACKVOLENV, lReg, iReg, layer, in);
		double hold=getGeneratorValue(SF2Region::GENERATOR_HOLDVOLENV, lReg, iReg, layer, in);
		double release=getGeneratorValue(SF2Region::GENERATOR_RELEASEVOLENV, lReg, iReg, layer, in);
		double decay=getGeneratorValue(SF2Region::GENERATOR_DECAYVOLENV, lReg, iReg, layer, in);
		double sustain=getGeneratorValue(SF2Region::GENERATOR_SUSTAINVOLENV, lReg, iReg, layer, in);
		attack=time(Util::clamp(attack == NOT_FOUND?-12000.0:attack, -12000.0, 8000.0));
		hold=time(Util::clamp(hold == NOT_FOUND?-12000.0:hold, -12000.0, 8000.0));
		release=time(Util::clamp(release == NOT_FOUND?-12000.0:release, -12000.0, 8000.0));
		decay=time(Util::clamp(decay == NOT_FOUND?-12000.0:decay, -12000.0, 8000.0));
		sustain=pow(10.0, -(Util::clamp(sustain == NOT_FOUND?0.0:sustain, 0.0, 1440.0)) / 10.0);
		attack=3;
		hold=1;
		decay=3000;
		sustain=0.5;
		release=100;
		int32_t fineTune=getGeneratorValue(SF2Region::GENERATOR_FINETUNE, lReg, iReg, layer, in);
		int32_t coarseTune=getGeneratorValue(SF2Region::GENERATOR_COARSETUNE, lReg, iReg, layer, in);
		if(fineTune == NOT_FOUND)fineTune=0;
		if(coarseTune == NOT_FOUND)coarseTune=0;
		std::shared_ptr<WaveSampler> sampler=WaveSamplerBuilder()
			.loop(sloop, eloop, sustainable?WaveSampler::LOOP_LOOP:WaveSampler::LOOP_DISABLE)
			.sample(getSampleProvider(*buf))
			.sampleFreq(tuning.getFrequencyByID(op - coarseTune - (double)fineTune / 100.0),
						(u_sample_rate)sample->getSampleRate())
			.build();

		NoteProcPtr p1=AmpBuilder()
			.src(sampler)
			.AHDSR(attack, hold, decay, sustain, sustainable, release, EnvUtil::Line(), EnvUtil::Line(), EnvUtil::Line())
			.build();
		auto layerRegionGenerators=lReg.getGenerators();
		int32_t lhKey=getGeneratorValue(SF2Region::GENERATOR_KEYRANGE, layerRegionGenerators);
		int32_t lhVel=getGeneratorValue(SF2Region::GENERATOR_VELRANGE, lReg, iReg, layer, in);
		int32_t startKey=op;
		int32_t endKey=op;
		int32_t startVel=0;
		int32_t endVel=0;
		if(lhKey != NOT_FOUND){
			startKey=lhKey & 0xff;
			endKey=(lhKey >> 8) & 0xff;
		} else{
			startKey=0;
			endKey=127;
		}
		if(lhVel != NOT_FOUND){
			startVel=lhVel & 0xff;
			endVel=(lhVel >> 8) & 0xff;
		} else{
			startVel=0;
			endVel=127;
		}
		//std::cout << "KR:" << startKey << " " << endKey << std::endl;
		//std::cout << "VR:" << startVel << " " << endVel << std::endl;
		amp.put(startKey, endKey, startVel, endVel, p1);
	}
}