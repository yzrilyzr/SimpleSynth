#include "SimpleSynth.h"
#include "TR808DrumSet.h"
#include "interface/NoteProcessor.h"
#include "synth/envelopers/EnvUtil.h"
#include "synth/generators/noise/NoiseSrc.h"
#include "synth/generators/pulse/CymbalOsc.h"
#include "synth/generators/pulse/Pulse.h"
#include "synth/generators/pulse/TriWave.h"
#include "synth/generators/sine/SineWave.h"
#include "synth/generators/sine/SineWaveTable.h"
#include "synth/source/AmpBuilder.h"
#include "synth/source/AmplitudeSources.h"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
namespace yzrilyzr_simplesynth{
	void TR808DrumSet::init(ChannelConfig & cfg){
		u_sample_rate sampleRate=cfg.sampleRate;
		add(MIDIFile::DrumSet::SIDE_STICK, AmpBuilder()
			.src(SineAmp(2420))
			.ADSR(2, 0, 1, false, 50, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::CONGA_LOW, AmpBuilder()
			.src(SineAmp(254))
			.ADSR(2, 0, 1, false, 170, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::CONGA_HI, AmpBuilder()
			.src(SineAmp(300))
			.ADSR(2, 0, 1, false, 170, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::BASS_DRUM_1, AmpBuilder()
			.src(SineAmp(71))
			.ADSR(2, 0, 1, false, 1700, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::BASS_DRUM_ACOUSTIC, AmpBuilder()
			.src(SineAmp(91))
			.ADSR(2, 0, 1, false, 1700, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::TOM_LOW, AmpBuilder()
			.src(std::make_shared<SineWave>())
			.ADSR(2, 0, 1, false, 1700, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::TOM_LOW_FLOOR, AmpBuilder()
			.src(std::make_shared<SineWave>())
			.ADSR(2, 0, 1, false, 1700, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::TOM_LOW_MID, AmpBuilder()
			.src(std::make_shared<SineWave>())
			.ADSR(2, 0, 1, false, 1700, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::TOM_HIGH, AmpBuilder()
			.src(std::make_shared<SineWave>())
			.ADSR(2, 0, 1, false, 1700, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::TOM_HIGH_FLOOR, AmpBuilder()
			.src(std::make_shared<SineWave>())
			.ADSR(2, 0, 1, false, 1700, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::TOM_HIGH_MID, AmpBuilder()
			.src(std::make_shared<SineWave>())
			.ADSR(2, 0, 1, false, 1700, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::HAND_CLAP, AmpBuilder()
			.src(AmpBuilder()
				 .src(std::make_shared<NoiseSrc>())
				 .biquad(sampleRate, FilterPassType::BANDPASS, 1000, 0.7, 0)
				 .build())
			.mul(AmpBuilder()
				 .src(ConstAmp(1))
				 .addMul(std::make_shared<Pulse>(ConstPhase(100), 0, 0.05, 0.95, 0), 0.3)
				 .build()
			)
			.ADSR(2, 0, 1, false, 190, Line(), Line(), Pow(1))
			.build()
		);
		add(MIDIFile::DrumSet::MARACAS, AmpBuilder()
			.src(std::make_shared<NoiseSrc>())
			.biquad(sampleRate, FilterPassType::HIGHPASS, 10000, 0.7, 0)
			.ADSR(2, 0, 1, false, 50, Line(), Line(), Pow(1))
			.build()
		);
		add(MIDIFile::DrumSet::SNARE_ACOUSTIC, AmpBuilder()
			.src(SineAmp(168))
			.addMul(SineAmp(327), 0.161)
			.add(AmpBuilder()
				 .src(std::make_shared<NoiseSrc>())
				 .biquad(sampleRate, FilterPassType::BANDPASS, 4100, 0.7, 0)
				 .build())
			.AHDSR(5, 10, 0, 1, false, 130, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::SNARE_ELECTRIC, AmpBuilder()
			.src(SineAmp(168))
			.addMul(SineAmp(327), 0.161)
			.add(AmpBuilder()
				 .src(std::make_shared<NoiseSrc>())
				 .biquad(sampleRate, FilterPassType::BANDPASS, 4100, 0.7, 0)
				 .build())
			.AHDSR(5, 10, 0, 1, false, 130, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::CYMBAL_CRASH_1, AmpBuilder()
			.src(std::make_shared<Pulse>(ConstPhase(558), 0.45, 0.05, 0.05, 0))//cb
			.add(std::make_shared<Pulse>(ConstPhase(485), 0.45, 0.05, 0.05, 0))//1
			.add(std::make_shared<Pulse>(ConstPhase(630), 0.45, 0.05, 0.05, 0))//
			.add(std::make_shared<Pulse>(ConstPhase(824), 0.45, 0.05, 0.05, 0))//cb
			.add(std::make_shared<Pulse>(ConstPhase(851), 0.45, 0.05, 0.05, 0))//
			.add(std::make_shared<Pulse>(ConstPhase(1035), 0.45, 0.05, 0.05, 0))
			.mul(0.2)
			.biquad(sampleRate, FilterPassType::HIGHPASS, 3000, 1, 0)
			.addMul(AmpBuilder()
					.src(std::make_shared<NoiseSrc>())
					.biquad(sampleRate, FilterPassType::BANDPASS, 7000, 0.6, 0)
					.build(), 0.4)
			.ADSR(24, 0, 1, false, 900, Pow(-5), Line(), Pow(3))
			.build()
		);
		add(MIDIFile::DrumSet::CYMBAL_CRASH_2, AmpBuilder()
			.src(std::make_shared<Pulse>(ConstPhase(558), 0.45, 0.05, 0.05, 0))//cb
			.add(std::make_shared<Pulse>(ConstPhase(485), 0.45, 0.05, 0.05, 0))//1
			.add(std::make_shared<Pulse>(ConstPhase(630), 0.45, 0.05, 0.05, 0))//
			.add(std::make_shared<Pulse>(ConstPhase(824), 0.45, 0.05, 0.05, 0))//cb
			.add(std::make_shared<Pulse>(ConstPhase(851), 0.45, 0.05, 0.05, 0))//
			.add(std::make_shared<Pulse>(ConstPhase(1035), 0.45, 0.05, 0.05, 0))
			.mul(0.2)
			.biquad(sampleRate, FilterPassType::HIGHPASS, 3000, 1, 0)
			.addMul(AmpBuilder()
					.src(std::make_shared<NoiseSrc>())
					.biquad(sampleRate, FilterPassType::BANDPASS, 7000, 0.6, 0)
					.build(), 0.4)
			.ADSR(24, 0, 1, false, 900, Pow(-5), Line(), Pow(3))
			.build()
		);
		add(MIDIFile::DrumSet::CYMBAL_RIDE_1, AmpBuilder()
			//.src(std::make_shared<Pulse>(ConstPhase(558),0.45,0.05,0.05,0))//cb
			//.add(std::make_shared<Pulse>(ConstPhase(485), 0.45, 0.05, 0.05, 0))//1
			//.add(std::make_shared<Pulse>(ConstPhase(630), 0.45, 0.05, 0.05, 0))//
			.src(std::make_shared<Pulse>(ConstPhase(824), 0.45, 0.05, 0.05, 0))//cb
			.add(std::make_shared<Pulse>(ConstPhase(851), 0.45, 0.05, 0.05, 0))//
			.add(std::make_shared<Pulse>(ConstPhase(1035), 0.45, 0.05, 0.05, 0))
			.mul(0.2)
			.biquad(sampleRate, FilterPassType::HIGHPASS, 3000, 1, 0)
			.addMul(AmpBuilder()
					.src(std::make_shared<NoiseSrc>())
					.biquad(sampleRate, FilterPassType::BANDPASS, 7000, 0.6, 0)
					.build(), 0.4)
			.ADSR(24, 0, 1, false, 900, Pow(-5), Line(), Pow(3))
			.build()
		);
		add(MIDIFile::DrumSet::CYMBAL_RIDE_2, AmpBuilder()
			//.src(std::make_shared<Pulse>(ConstPhase(558),0.45,0.05,0.05,0))//cb
			//.add(std::make_shared<Pulse>(ConstPhase(485), 0.45, 0.05, 0.05, 0))//1
			//.add(std::make_shared<Pulse>(ConstPhase(630), 0.45, 0.05, 0.05, 0))//
			.src(std::make_shared<Pulse>(ConstPhase(824), 0.45, 0.05, 0.05, 0))//cb
			.add(std::make_shared<Pulse>(ConstPhase(851), 0.45, 0.05, 0.05, 0))//
			.add(std::make_shared<Pulse>(ConstPhase(1035), 0.45, 0.05, 0.05, 0))
			.mul(0.2)
			.biquad(sampleRate, FilterPassType::HIGHPASS, 3000, 1, 0)
			.addMul(AmpBuilder()
					.src(std::make_shared<NoiseSrc>())
					.biquad(sampleRate, FilterPassType::BANDPASS, 7000, 0.6, 0)
					.build(), 0.4)
			.ADSR(24, 0, 1, false, 900, Pow(-5), Line(), Pow(3))
			.build()
		);
		add(MIDIFile::DrumSet::HI_HAT_OPEN, AmpBuilder()
			.src(std::make_shared<Pulse>(ConstPhase(558), 0.45, 0.05, 0.05, 0))//cb
			.add(std::make_shared<Pulse>(ConstPhase(485), 0.45, 0.05, 0.05, 0))//1
			.add(std::make_shared<Pulse>(ConstPhase(630), 0.45, 0.05, 0.05, 0))//
			.add(std::make_shared<Pulse>(ConstPhase(824), 0.45, 0.05, 0.05, 0))//cb
			.add(std::make_shared<Pulse>(ConstPhase(851), 0.45, 0.05, 0.05, 0))//
			.add(std::make_shared<Pulse>(ConstPhase(1035), 0.45, 0.05, 0.05, 0))
			.mul(0.15)
			.addMul(AmpBuilder()
					.src(std::make_shared<NoiseSrc>())
					.biquad(sampleRate, FilterPassType::BANDPASS, 4000, 0.6, 0)
					.build(), 0.3)
			.biquad(sampleRate, FilterPassType::HIGHPASS, 8442, 1.5, 0)
			.ADSR(4, 0, 1, false, 900, Pow(-5), Line(), Pow(2))
			.build()
		);
		add(MIDIFile::DrumSet::HI_HAT_PEDAL, AmpBuilder()
			.src(std::make_shared<Pulse>(ConstPhase(558), 0.45, 0.05, 0.05, 0))//cb
			.add(std::make_shared<Pulse>(ConstPhase(485), 0.45, 0.05, 0.05, 0))//1
			.add(std::make_shared<Pulse>(ConstPhase(630), 0.45, 0.05, 0.05, 0))//
			.add(std::make_shared<Pulse>(ConstPhase(824), 0.45, 0.05, 0.05, 0))//cb
			.add(std::make_shared<Pulse>(ConstPhase(851), 0.45, 0.05, 0.05, 0))//
			.add(std::make_shared<Pulse>(ConstPhase(1035), 0.45, 0.05, 0.05, 0))
			.mul(0.15)
			.addMul(AmpBuilder()
					.src(std::make_shared<NoiseSrc>())
					.biquad(sampleRate, FilterPassType::BANDPASS, 4000, 0.6, 0)
					.build(), 0.3)
			.biquad(sampleRate, FilterPassType::HIGHPASS, 8442, 1.5, 0)
			.ADSR(4, 0, 1, false, 250, Pow(-5), Line(), Pow(2))
			.build()
		);
		add(MIDIFile::DrumSet::HI_HAT_CLOSED, AmpBuilder()
			.src(std::make_shared<Pulse>(ConstPhase(558), 0.45, 0.05, 0.05, 0))//cb
			.add(std::make_shared<Pulse>(ConstPhase(485), 0.45, 0.05, 0.05, 0))//1
			.add(std::make_shared<Pulse>(ConstPhase(630), 0.45, 0.05, 0.05, 0))//
			.add(std::make_shared<Pulse>(ConstPhase(824), 0.45, 0.05, 0.05, 0))//cb
			.add(std::make_shared<Pulse>(ConstPhase(851), 0.45, 0.05, 0.05, 0))//
			.add(std::make_shared<Pulse>(ConstPhase(1035), 0.45, 0.05, 0.05, 0))
			.mul(0.15)
			.addMul(AmpBuilder()
					.src(std::make_shared<NoiseSrc>())
					.biquad(sampleRate, FilterPassType::BANDPASS, 4000, 0.6, 0)
					.build(), 0.3)
			.biquad(sampleRate, FilterPassType::HIGHPASS, 10442, 2, 0)
			.ADSR(4, 0, 1, false, 50, Pow(-5), Line(), Pow(5))
			.build()
		);
		add(MIDIFile::DrumSet::COWBELL, AmpBuilder()
			.src(std::make_shared<Pulse>(ConstPhase(558), 0.45, 0.05, 0.05, 0))//cb
			.add(std::make_shared<Pulse>(ConstPhase(824), 0.45, 0.05, 0.05, 0))//cb
			.biquad(sampleRate, FilterPassType::BANDPASS, 1000, 0.7, 0)
			.ADSR(4, 0, 1, false, 300, Pow(-5), Line(), Pow(2))
			.build()
		);
		NonInterpolateAmpSet::init(cfg);
	}
}