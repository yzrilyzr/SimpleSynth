#include "ChipInstrument.h"
#include "synth/source/AmpBuilder.h"
#include "synth/generators/pulse/Pulse.h"
#include "synth/generators/pulse/SawWave.h"
#include "synth/generators/pulse/TriWave.h"
#include "synth/generators/noise/LFSRNoise.h"
#include "synth/envelopers/EnvUtil.h"
#include "interface/NoteProcessor.h"
using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
namespace yzrilyzr_simplesynth{
	NoteProcPtr ChipInstrument::get(s_bank_id bank, s_program_id program, u_sample_rate sampleRate){
		if(program >= MIDIFile::Instruments::PIANO_ACOUSTIC_GRAND_PIANO && program <= MIDIFile::Instruments::PIANO_CLAVICHORD){
			//50 %
			return AmpBuilder()
				.src(mksp < Pulse>(0.5, 0, 0, 0))
				.biquad(sampleRate, FilterPassType::HIGHPASS, 30, 0.7, 0)
				.ADSR(5, 2000, 0, false, 100, Pow(5), Pow(5), Pow(5))
				.build();
		} else if(program >= MIDIFile::Instruments::CHROMATIC_PERCUSSION_CELESTA && program <= MIDIFile::Instruments::CHROMATIC_PERCUSSION_DULCIMER){
			//25 %
			return AmpBuilder()
				.src(mksp < Pulse>(0.25, 0, 0, 0))
				.biquad(sampleRate, FilterPassType::HIGHPASS, 30, 0.7, 0)
				.ADSR(5, 500, 0, false, 100, Pow(5), Pow(5), Pow(5))
				.build();
		} else if(program >= MIDIFile::Instruments::ORGAN_HAMMOND_ORGAN && program <= MIDIFile::Instruments::ORGAN_TANGO_ACCORDIAN){
			//75 %
			return AmpBuilder()
				.src(mksp < Pulse>(0.75, 0, 0, 0))
				.biquad(sampleRate, FilterPassType::HIGHPASS, 30, 0.7, 0)
				.ADSR(5, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
				.build();
		}else if(program >= MIDIFile::Instruments::GUITAR_ACOUSTIC_GUITAR_NYLON && program <= MIDIFile::Instruments::GUITAR_GUITAR_HARMONICS){
			//50 %
			return AmpBuilder()
				.src(mksp < Pulse>(0.5, 0, 0, 0))
				.biquad(sampleRate, FilterPassType::HIGHPASS, 30, 0.7, 0)
				.ADSR(5, 2000, 0, false, 100, Pow(5), Pow(5), Pow(5))
				.build();
		}else if(program >= MIDIFile::Instruments::BASS_ACOUSTIC_BASS && program <= MIDIFile::Instruments::BASS_SYNTH_BASS_2){
			//Tri
			return AmpBuilder()
				.src(mksp < TriWave>())
				.quantization(8)
				.biquad(sampleRate, FilterPassType::HIGHPASS, 30, 0.7, 0)
				.ADSR(5, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
				.build();
		}else if(program >= MIDIFile::Instruments::SOLO_STRING_VIOLIN && program <= MIDIFile::Instruments::SOLO_STRING_TIMPANI){
			//Saw
			return AmpBuilder()
				.src(mksp < SawWave>())
				.quantization(8).IIR(sampleRate, 2, 30, 20000)
				.autoMod(0.2,0.2,5.0,0.1)
				.ADSR(10, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
				.build();
		}else if(program >= MIDIFile::Instruments::ENSEMBLE_STRING_ENSEMBLE_1 && program <= MIDIFile::Instruments::ENSEMBLE_ORCHESTRA_HIT){
			//Saw
			return AmpBuilder()
				.src(mksp < SawWave>())
				.quantization(8).IIR(sampleRate, 2, 30, 20000)
				.autoMod(0.2, 0.2, 7.0, 0.1)
				.ADSR(10, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
				.build();
		}else if(program >= MIDIFile::Instruments::BRASS_TRUMPET && program <= MIDIFile::Instruments::BRASS_SYNTH_BRASS_2){
			//12.5 %
			return AmpBuilder()
				.src(mksp < Pulse>(0.125, 0, 0, 0))
				.biquad(sampleRate, FilterPassType::HIGHPASS, 30, 0.7, 0)
				.autoMod(0.2, 0.2, 7.0, 0.1)
				.ADSR(5, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
				.build();
		}else if(program >= MIDIFile::Instruments::REED_SOPRANO_SAX && program <= MIDIFile::Instruments::REED_CLARINET){
			//Saw
			return AmpBuilder()
				.src(mksp < SawWave>())
				.quantization(8).IIR(sampleRate, 2, 30, 20000)
				.autoMod(0.2, 0.2, 7.0, 0.1)
				.ADSR(10, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
				.build();
		}else if(program >= MIDIFile::Instruments::PIPE_PICCOLO && program <= MIDIFile::Instruments::PIPE_OCARINA){
			//50 %
			return AmpBuilder()
				.src(mksp < Pulse>(0.5, 0, 0, 0))
				.biquad(sampleRate, FilterPassType::HIGHPASS, 30, 0.7, 0)
				.autoMod(0.2, 0.2, 7.0, 0.1)
				.ADSR(5, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
				.build();
		}else if(program == MIDIFile::Instruments::LEAD_SQUARE){
			//50 %
			return AmpBuilder()
				.src(mksp < Pulse>(0.5, 0, 0, 0))
				.biquad(sampleRate, FilterPassType::HIGHPASS, 30, 0.7, 0)
				.autoMod(0.2, 0.2, 7.0, 0.1)
				.ADSR(5, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
				.build();
		}else if(program == MIDIFile::Instruments::LEAD_SAWTOOTH){
			//Saw
			return AmpBuilder()
				.src(mksp < SawWave>())
				.quantization(8).IIR(sampleRate, 2, 30, 20000)
				.autoMod(0.2, 0.2, 7.0, 0.1)
				.ADSR(10, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
				.build();
		}
		//default
		return AmpBuilder()
			.src(mksp < Pulse>(0.5, 0, 0, 0))
			.biquad(sampleRate, FilterPassType::HIGHPASS, 30, 0.7, 0)
			.ADSR(5, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
			.build();
	}
	NoteProcPtr ChipInstrument::getDrumSet(s_bank_id bank, u_sample_rate sampleRate){
		return AmpBuilder()
			.src(mksp < LFSRNoise>(14))
			.biquad(sampleRate, FilterPassType::HIGHPASS, 30, 0.7, 0)
			.ADSR(10, 0, 1, false, 200, Pow(5), Pow(5), Pow(5))
			.build();
	}
}