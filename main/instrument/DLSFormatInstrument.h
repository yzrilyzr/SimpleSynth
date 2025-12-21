#pragma once
#include "array/SampleProvider.h"
#include "interface/InstrumentProvider.h"
#include "interface/NoteProcessor.h"
#include "io/InputStream.h"
#include "soundbank/dls/DLSInstrument.h"
#include "soundbank/dls/DLSRegion.h"
#include "soundbank/dls/DLSSample.h"
#include "soundbank/dls/DLSSoundbank.h"
#include "synth/composed/NonInterpolateAmpSet.h"
#include "synth/source/AmpBuilder.h"
#include "tuning/EqualTemperament.h"
#include <map>

namespace yzrilyzr_simplesynth{
	ECLASS(DLSFormatInstrument, public InstrumentProvider){
	private:
	u_sp<yzrilyzr_soundbank::DLSSoundbank> soundbank;
	std::map<std::pair<s_bank_id, s_program_id>, NoteProcPtr> programMap;
	std::map<s_bank_id, u_sp<NonInterpolateAmpSet>> drumSetMap;
	EqualTemperament tuning;

	public:
	DLSFormatInstrument(yzrilyzr_io::InputStream & inputStream);
	~DLSFormatInstrument();
	NoteProcPtr get(s_bank_id bank, s_program_id program, u_sample_rate sampleRate) override;
	NoteProcPtr getDrumSet(s_bank_id bank, u_sample_rate sampleRate) override;

	private:
	void buildInstruments();
	void buildMelodicInstrument(yzrilyzr_soundbank::DLSInstrument & instrument);
	void buildDrumInstrument(yzrilyzr_soundbank::DLSInstrument & instrument);
	void createSampler(yzrilyzr_soundbank::DLSRegion & region, AmpBuilder & builder);

	static u_time_ms timeCent2Ms(double timeCents);
	static u_normal_01 scale2Normal01(double scale);
	static u_sp<yzrilyzr_array::SampleProvider> convertToSample(yzrilyzr_soundbank::DLSSample & sample);
	static void procModulators(yzrilyzr_soundbank::DLSRegion & region, yzrilyzr_soundbank::DLSInstrument & instrument, AmpBuilder & builder);
	static float scaleToFloat(int32_t scale);
	};
}