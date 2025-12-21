#pragma once
#include "array/Array.hpp"
#include "collection/ArrayList.hpp"
#include "collection/HashMap.hpp"
#include "interface/InstrumentProvider.h"
#include "soundbank/ModelByteBuffer.h"
#include "soundbank/sf2/SF2Instrument.h"
#include "soundbank/sf2/SF2InstrumentRegion.h"
#include "soundbank/sf2/SF2Layer.h"
#include "soundbank/sf2/SF2LayerRegion.h"
#include "soundbank/sf2/SF2Soundbank.h"
#include "synth/composed/NonInterpolateAmpSet.h"
#include "synth/composed/RegionAmp.h"
#include "tuning/EqualTemperament.h"
#include <map>

namespace yzrilyzr_simplesynth{
	ECLASS(SF2FormatInstrument, public InstrumentProvider){
	private:
	static constexpr int NOT_FOUND=48964944;
	std::map<std::pair<s_bank_id, s_program_id>, NoteProcPtr> programMap;
	std::map<s_bank_id, u_sp<NonInterpolateAmpSet>> drumSetMap;
	std::unordered_map<int64_t, u_sp<yzrilyzr_array::SampleProvider>> sampleMap;
	EqualTemperament tuning;
	u_sp<yzrilyzr_soundbank::SF2Soundbank> sbx;
	public:
	NoteProcPtr get(s_bank_id bank, s_program_id program, u_sample_rate sampleRate) override;
	NoteProcPtr getDrumSet(s_bank_id bank, u_sample_rate sampleRate) override;
	SF2FormatInstrument(yzrilyzr_io::InputStream & inputStream);
	~SF2FormatInstrument();
	private:
	void putDrumSet(yzrilyzr_soundbank::SF2Instrument & in);
	void putInst(yzrilyzr_soundbank::SF2Instrument & in);
	void putLayerRegion(RegionAmp & amp, yzrilyzr_soundbank::SF2Instrument & in, yzrilyzr_soundbank::SF2InstrumentRegion & instRegion, yzrilyzr_soundbank::SF2LayerRegion & layerRegion, yzrilyzr_soundbank::SF2Layer & layer);
	int32_t getGeneratorValue(int32_t id, yzrilyzr_soundbank::SF2LayerRegion & lregion, yzrilyzr_soundbank::SF2InstrumentRegion & instRegion, yzrilyzr_soundbank::SF2Layer & layer, yzrilyzr_soundbank::SF2Instrument & in);
	int32_t getGeneratorValue(int32_t id, u_sp<yzrilyzr_collection::HashMap<int32_t, int16_t>> g1);
	void addSamplerByID(int32_t id, int32_t vel, NoteProcPtr sampler);
	int32_t getOriginalPitch(yzrilyzr_soundbank::SF2LayerRegion & layerRegion);
	u_sp<yzrilyzr_array::SampleProvider> getSampleProvider(yzrilyzr_soundbank::ModelByteBuffer & buf);
	static double time(double in);
	};
}