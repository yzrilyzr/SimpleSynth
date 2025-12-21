#include "ChipInstrument.h"
#include "synth/source/AmpBuilder.h"
#include "synth/generators/pulse/Pulse.h"
#include "synth/generators/pulse/SawWave.h"
#include "synth/generators/pulse/TriWave.h"
#include "synth/generators/noise/LFSRNoise.h"
#include "synth/envelopers/EnvUtil.h"
#include "interface/NoteProcessor.h"

namespace yzrilyzr_simplesynth{
	NoteProcPtr ChipInstrument::get(s_bank_id bank, s_program_id program, u_sample_rate sampleRate){
		switch(program){
			case PULSE_12D5:
				return AmpBuilder()
					.src(mksp<Pulse>(0.125, 0, 0, 0))
					.IIR(sampleRate, 2, 30, 20000)
					.ADSR(10, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
					.build();
			case PULSE_25:
				return AmpBuilder()
					.src(mksp < Pulse>(0.25, 0, 0, 0))
					.IIR(sampleRate, 2, 30, 20000)
					.ADSR(10, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
					.build();
			case PULSE_50:
				return AmpBuilder()
					.src(mksp < Pulse>(0.5, 0, 0, 0))
					.IIR(sampleRate, 2, 30, 20000)
					.ADSR(10, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
					.build();
			case PULSE_75:
				return AmpBuilder()
					.src(mksp < Pulse>(0.75, 0, 0, 0))
					.IIR(sampleRate, 2, 30, 20000)
					.ADSR(10, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
					.build();
			case TRI:
				return AmpBuilder()
					.src(mksp < TriWave>())
					.quantization(8)
					.IIR(sampleRate, 2, 30, 20000)
					.ADSR(10, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
					.build();
			case SAW:
				return AmpBuilder()
					.src(mksp < SawWave>())
					.quantization(8).IIR(sampleRate, 2, 30, 20000)
					.ADSR(10, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
					.build();
			case NOISE_SHORT:
				return AmpBuilder().src(mksp < LFSRNoise>(6))
					.IIR(sampleRate, 2, 30, 20000)
					.ADSR(10, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
					.build();
			case NOISE_LONG:
				return AmpBuilder().src(mksp < LFSRNoise>(14))
					.IIR(sampleRate, 2, 30, 20000)
					.ADSR(10, 0, 1, true, 102, Pow(5), Pow(5), Pow(5))
					.build();
			default:
				return nullptr;
		}
	}
	NoteProcPtr ChipInstrument::getDrumSet(s_bank_id bank, u_sample_rate sampleRate){
		return AmpBuilder().src(mksp < LFSRNoise>(14)).IIR(sampleRate, 2, 30, 20000)
			.ADSR(10, 0, 1, false, 200, Pow(5), Pow(5), Pow(5))
			.build();
	}
}