#include "SimpleSynth.h"
#include "TR808DrumSet.h"
#include "interface/NoteProcessor.h"
#include "interpolator/GraphInterpolator.h"
#include "synth/modulation/SimpleDrumAmp.h"
#include "synth/osc/noise/NoiseSrc.h"
#include "synth/osc/pulse/CymbalOsc.h"
#include "synth/osc/pulse/Pulse.h"
#include "synth/osc/pulse/TriWave.h"
#include "synth/osc/sine/SineWave.h"
#include "synth/osc/sine/SineWaveTable.h"
#include "synth/util/AmpBuilder.h"
#include "synth/util/AmplitudeSources.h"
#include "synth/util/EnvUtil.h"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
using namespace yzrilyzr_interpolator;
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
			.src(mksp<SimpleDrumAmp>(mksp<SineWave>(), 200, 50, 0.4f, SimpleDrumAmp::MODE_FIXED, Pow(-30)))
			.AR(2, 400, Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::BASS_DRUM_ACOUSTIC, AmpBuilder()
			.src(mksp<SimpleDrumAmp>(mksp<SineWave>(), 250, 60, 0.4f, SimpleDrumAmp::MODE_FIXED, Pow(-30)))
			.AR(2, 400, Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::TOM_LOW, AmpBuilder()
			.src(mksp<SineWave>())
			.AR(2, 1700, Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::TOM_LOW_FLOOR, AmpBuilder()
			.src(mksp<SineWave>())
			.ADSR(2, 0, 1, false, 1700, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::TOM_LOW_MID, AmpBuilder()
			.src(mksp<SineWave>())
			.ADSR(2, 0, 1, false, 1700, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::TOM_HIGH, AmpBuilder()
			.src(mksp<SineWave>())
			.ADSR(2, 0, 1, false, 1700, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::TOM_HIGH_FLOOR, AmpBuilder()
			.src(mksp<SineWave>())
			.ADSR(2, 0, 1, false, 1700, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::TOM_HIGH_MID, AmpBuilder()
			.src(mksp<SineWave>())
			.ADSR(2, 0, 1, false, 1700, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::HAND_CLAP, AmpBuilder()
			.src(mksp<NoiseSrc>())
			.biquad(RBJParams{FilterPassType::BANDPASS,1300,sampleRate,0.5, 0})
			.mul(2)
			.ADSR(2, 0, 1, false, 190, Line(), Line(), mksp<GraphInterpolator>(std::initializer_list<double>{
			0, 0,
				0.15, 0.01,
				0.35, 0.05,
				0.5, 0.1,
				0.65, 0.2,
				0.75, 0.4,
				0.84, 0.8,
				0.899, 1,
				0.900, 0,
				0.949, 1,
				0.950, 0,
				1, 1}))
			.build()
		);
		add(MIDIFile::DrumSet::MARACAS, AmpBuilder()
			.src(mksp<NoiseSrc>())
			.biquad(RBJParams{FilterPassType::HIGHPASS,10000,sampleRate,0.7, 0})
			.ADSR(2, 0, 1, false, 50, Line(), Line(), Pow(1))
			.build()
		);
		add(MIDIFile::DrumSet::SNARE_ACOUSTIC, AmpBuilder()
			.src(SineAmp(168))
			.addMul(SineAmp(327), 0.161)
			.add(AmpBuilder()
				 .src(mksp<NoiseSrc>())
				 .biquad(RBJParams{FilterPassType::BANDPASS,4100,sampleRate,0.7, 0})
				 .build())
			.AHDSR(5, 10, 0, 1, false, 130, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::SNARE_ELECTRIC, AmpBuilder()
			.src(SineAmp(168))
			.addMul(SineAmp(327), 0.161)
			.add(AmpBuilder()
				 .src(mksp<NoiseSrc>())
				 .biquad(RBJParams{FilterPassType::BANDPASS,4100,sampleRate,0.7, 0})
				 .build())
			.AHDSR(5, 10, 0, 1, false, 130, Line(), Line(), Pow(4))
			.build()
		);
		add(MIDIFile::DrumSet::CYMBAL_CRASH_1, AmpBuilder()
			.src(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 558))//cb
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 485))//1
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 630))//
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 824))//cb
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 851))//
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 1035))
			.mul(0.2)
			.addMul(mksp<NoiseSrc>(), 0.4)
			.biquad(RBJParams{FilterPassType::HIGHPASS,4000,sampleRate,0.8, 0})
			.biquad(RBJParams{FilterPassType::LOWPASS,10000,sampleRate,0.2, 0})
			.mul(8)
			.AR(24, 900, Pow(-5), Pow(3))
			.build()
		);
		add(MIDIFile::DrumSet::CYMBAL_CRASH_2, AmpBuilder()
			.src(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 558))//cb
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 485))//1
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 630))//
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 824))//cb
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 851))//
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 1035))
			.mul(0.2)
			.addMul(mksp<NoiseSrc>(), 0.4)
			.biquad(RBJParams{FilterPassType::HIGHPASS,4000,sampleRate,0.8, 0})
			.biquad(RBJParams{FilterPassType::LOWPASS,10000,sampleRate,0.2, 0})
			.mul(8)
			.AR(24, 900, Pow(-5), Pow(3))
			.build()
		);
		add(MIDIFile::DrumSet::CYMBAL_RIDE_1, AmpBuilder()
			.src(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 824))//cb
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 851))//
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 1035))
			.mul(0.1)
			.addMul(mksp<NoiseSrc>(), 0.2)
			.biquad(RBJParams{FilterPassType::HIGHPASS,3000,sampleRate,1, 0})
			.ADSR(24, 0, 1, false, 900, Pow(-5), Line(), Pow(3))
			.build()
		);
		add(MIDIFile::DrumSet::CYMBAL_RIDE_2, AmpBuilder()
			.src(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 824))//cb
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 851))//
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 1035))
			.mul(0.1)
			.addMul(mksp<NoiseSrc>(), 0.2)
			.biquad(RBJParams{FilterPassType::HIGHPASS,3000,sampleRate,1, 0})
			.ADSR(24, 0, 1, false, 900, Pow(-5), Line(), Pow(3))
			.build()
		);
		add(MIDIFile::DrumSet::HI_HAT_OPEN, AmpBuilder()
			.src(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 558))//cb
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 485))//1
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 630))//
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 824))//cb
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 851))//
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 1035))
			.mul(0.15)
			.addMul(AmpBuilder()
					.src(mksp<NoiseSrc>())
					.biquad(RBJParams{FilterPassType::BANDPASS,4000,sampleRate,0.6, 0})
					.build(), 0.3)
			.biquad(RBJParams{FilterPassType::HIGHPASS,8442,sampleRate,1.5, 0})
			.ADSR(4, 0, 1, false, 900, Pow(-5), Line(), Pow(2))
			.build()
		);
		add(MIDIFile::DrumSet::HI_HAT_PEDAL, AmpBuilder()
			.src(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 558))//cb
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 485))//1
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 630))//
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 824))//cb
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 851))//
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 1035))
			.mul(0.15)
			.addMul(AmpBuilder()
					.src(mksp<NoiseSrc>())
					.biquad(RBJParams{FilterPassType::BANDPASS,4000,sampleRate,0.6, 0})
					.build(), 0.3)
			.biquad(RBJParams{FilterPassType::HIGHPASS,8442,sampleRate,1.5, 0})
			.ADSR(4, 0, 1, false, 250, Pow(-5), Line(), Pow(2))
			.build()
		);
		add(MIDIFile::DrumSet::HI_HAT_CLOSED, AmpBuilder()
			.src(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 558))//cb
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 485))//1
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 630))//
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 824))//cb
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 851))//
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 1035))
			.mul(0.15)
			.addMul(AmpBuilder()
					.src(mksp<NoiseSrc>())
					.biquad(RBJParams{FilterPassType::BANDPASS,4000,sampleRate,0.6, 0})
					.build(), 0.3)
			.biquad(RBJParams{FilterPassType::HIGHPASS,10442,sampleRate,2, 0})
			.ADSR(4, 0, 1, false, 50, Pow(-5), Line(), Pow(5))
			.build()
		);
		add(MIDIFile::DrumSet::COWBELL, AmpBuilder()
			.src(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 558))//cb
			.add(ConstFreq(mksp<Pulse>(0.45, 0.05, 0.05, 0), 824))//cb
			.biquad(RBJParams{FilterPassType::BANDPASS,1000,sampleRate,0.7, 0})
			.ADSR(4, 0, 1, false, 300, Pow(-5), Line(), Pow(2))
			.build()
		);
		NonInterpolateAmpSet::init(cfg);
	}
}