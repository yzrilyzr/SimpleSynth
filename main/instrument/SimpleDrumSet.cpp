/**
 * 默认简单鼓组，开箱即用
 * 不喜欢的话，可以直接删掉，没有任何影响(～￣▽￣)～
 */

#include "SimpleDrumSet.h"
#include "SimpleSynth.h"
#include "dsp/DSPGroupBuilder.h"
#include "interpolator/GraphInterpolator.h"
#include "synth/envelopers/EnvUtil.h"
#include "synth/generators/drum/SimpleDrumAmp.h"
#include "synth/generators/noise/NoiseSrc.h"
#include "synth/generators/pulse/CymbalOsc.h"
#include "synth/generators/pulse/Pulse.h"
#include "synth/generators/pulse/SquareWave.h"
#include "synth/generators/pulse/TriWave.h"
#include "synth/generators/sine/SineWave.h"
#include "synth/source/AmplitudeSources.h"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
using namespace yzrilyzr_array;
using namespace yzrilyzr_interpolator;
namespace yzrilyzr_simplesynth{
	SimpleDrumSet::SimpleDrumSet(){}
	NoteProcPtr SimpleDrumSet::tom(u_sample_rate sampleRate){
		return tom(sampleRate, 1);
	}
	NoteProcPtr SimpleDrumSet::tom(u_sample_rate sampleRate, double scale){
		double base=1.5 * scale;
		double end=base / 2;
		return AmpBuilder()
			.src(mksp<SimpleDrumAmp>(risset(), base, end, 1.0, 1, Pow(-8)))
			.AR(10, 1000, Pow(-5), Pow(5))
			.clampV(3, 1)
			.addMul(AmpBuilder()
					.src(mksp<NoiseSrc>())
					.IIR(sampleRate, 2, 100, 2000.0)
					.AR(4, 150, Pow(5), Pow(5))
					.build(), 1)
			.mul(0.5)
			.build();
	}
	u_sp<SineWaveTable> SimpleDrumSet::risset(){
		return mksp<SineWaveTable>(100, DoubleArray({
			100, 1, 160, 0.1, 226, 0.12
													}));
	}

	NoteProcPtr SimpleDrumSet::kickBassRaw(){
		return AmpBuilder().src(mksp<SimpleDrumAmp>(mksp<SineWave>(), 550, 60, 0.5f, SimpleDrumAmp::MODE_FIXED, Pow(-5)))
			.AR(10, 500, 500, Pow(-5), Pow(-2))
			.clampV(10.35, 0.9)
			.build();
	}

	NoteProcPtr SimpleDrumSet::kickBassTrap(){
		return AmpBuilder().src(mksp<SimpleDrumAmp>(mksp<SineWave>(), 4, 1, 1.5f, 1, Pow(-50)))
			.AR(10, 1500, 100, Pow(-5), Pow(-10))
			.clampV(1.35, 0.9)
			.build();
	}
	std::vector<MSEPoint> releaseWithCompressorEffect(int count, u_time_ms sawPeriodStart, u_time_ms sawPeriodEnd, u_time_ms totalReleaseTime, float relValue){
		std::vector<MSEPoint> points;
		float sumStartTime=0;
		for(int i=0;i < count;i++){
			points.push_back({sumStartTime, 1, MSEPointType::DEFAULT, MSEPointMode::HOLD, 0});
			sumStartTime+=Util::linearMap(0, count - 1, sawPeriodStart, sawPeriodEnd, i) / 1000.0;
			points.push_back({sumStartTime, 0, MSEPointType::DEFAULT, MSEPointMode::SMOOTH, 0});
			sumStartTime+=0.001;
		}
		points.push_back({sumStartTime, 1, MSEPointType::DEFAULT, MSEPointMode::HOLD, 0});
		points.push_back({(float)(totalReleaseTime / 1000.0), 0, MSEPointType::DEFAULT, MSEPointMode::SINGLE_CURVE, relValue});
		return points;
	}
	void SimpleDrumSet::init(ChannelConfig & cfg){
		u_sample_rate sampleRate=cfg.sampleRate;
		add(MIDIFile::DrumSet::HIGH_Q,
			AmpBuilder().src(mksp<SineWave>())
			.drum(20000, 50, 0.05)
			.mul(0.8)
			.AR(5, 150, Pow(5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::SLAP,
			AmpBuilder().src(mksp<NoiseSrc>())
			.IIR(sampleRate, 2, 748.2, 3544.7)
			.mul(1.8)
			.AR(5, 200, Pow(5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::SCRATCH_PULL,
			AmpBuilder().src(mksp<SimpleDrumAmp>(mksp<SineWave>(), 150, 1000, 0.3f))
			.AR(10, 500, Pow(-5), Pow(5))
			.clampV(1.35, 0.5)
			.build());
		add(MIDIFile::DrumSet::SCRATCH_PUSH,
			AmpBuilder().src(mksp<SimpleDrumAmp>(mksp<SineWave>(), 1000, 150, 0.3f))
			.AR(10, 500, Pow(-5), Pow(5))
			.clampV(1.35, 0.5)
			.build());
		add(MIDIFile::DrumSet::STICKS,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 1545, 10.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 2348, 10.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 3459, 10.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 4331, 15.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 4537, 15.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 5549, 10.0, 20)
					 .biquad(sampleRate, FilterPassType::BANDPASS, 3000, 1.5, 0)
					 .build())
			.mul(0.25)
			.AR(5, 500, Pow(5), Pow(10))
			.build());
		add(MIDIFile::DrumSet::SQUARE_CLICK,
			AmpBuilder()
			.src(mksp<SquareWave>())
			.AR(5, 50, Pow(5), Pow(20))
			.build());
		add(MIDIFile::DrumSet::METRONOME_CLICK,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 484, 5.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 1174, 5.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 1917, 10.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 2544, 4.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 3220, 10.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 4154, 10.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 5549, 10.0, 15)
					 .biquad(sampleRate, FilterPassType::BANDPASS, 2000, 0.7, 0)
					 .build())
			.mul(0.4)
			.AR(5, 500, Pow(5), Pow(50))
			.build());
		add(MIDIFile::DrumSet::METRONOME_BELL,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 484, 5.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 1174, 5.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 1917, 10.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 2544, 4.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 3220, 10.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 4154, 10.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 5549, 10.0, 15)
					 .biquad(sampleRate, FilterPassType::BANDPASS, 2000, 0.7, 0)
					 .build())
			.mul(0.38)
			.AR(5, 500, Pow(5), Pow(50))
			.add(AmpBuilder()
				 .src(mksp<SineWave>(ConstPhase(5460)))
				 .addMul(mksp<SineWave>(ConstPhase(2048)), 0.3)
				 .mul(0.2)
				 .AR(5, 1000, Pow(5), Pow(5))
				 .build())
			.build());
		add(MIDIFile::DrumSet::BASS_DRUM_ACOUSTIC, AmpBuilder().src(mksp<SimpleDrumAmp>(mksp<SineWave>(), 350, 50, 1.0f, SimpleDrumAmp::MODE_FIXED, Pow(-50)))
			.AR(10, 500, 500, Pow(-5), Pow(10))
			.arctanDistortion(1.0, 15, 1)
			.add(AmpBuilder().src(mksp<SimpleDrumAmp>(mksp<SineWave>(), 1000, 100, 0.1f, SimpleDrumAmp::MODE_FIXED, Pow(-50)))
				 .AR(10, 100, 100, Pow(-5), Pow(5))
				 .arctanDistortion(1, 10, 0.3)
				 .build())
			.build());
		add(MIDIFile::DrumSet::BASS_DRUM_1, AmpBuilder().src(mksp<SimpleDrumAmp>(mksp<SineWave>(), 500, 40, 1.0f, SimpleDrumAmp::MODE_FIXED, Pow(-50)))
			.AR(10, 1000, 1000, Pow(-5), Pow(10))
			.arctanDistortion(1.0, 15, 1)
			.add(AmpBuilder().src(mksp<SimpleDrumAmp>(mksp<SineWave>(), 1000, 100, 0.1f, SimpleDrumAmp::MODE_FIXED, Pow(-50)))
				 .AR(10, 100, 100, Pow(-5), Pow(5))
				 .arctanDistortion(1, 10, 0.3)
				 .build())
			.build());
		add(MIDIFile::DrumSet::SIDE_STICK,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 211, 3.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 338, 7.0, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 700, 5.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 1163, 5.0, 18)
					 .biquad(sampleRate, FilterPassType::BELL, 1953, 4.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 2986, 5.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 4651, 5.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 6231, 7.0, 15)
					 .biquad(sampleRate, FilterPassType::BANDPASS, 2000, 0.7, 0)
					 .build())
			.mul(0.28)
			.AR(5, 500, Pow(5), Pow(20))
			.build());
		add(MIDIFile::DrumSet::SNARE_ACOUSTIC,
			AmpBuilder()
			.src(mksp<SimpleDrumAmp>(risset(), 180, 130, 0.3f, SimpleDrumAmp::MODE_FIXED, Pow(-5)))
			.arctanDistortion(1, 5, 2)
			.addMul(AmpBuilder()
					.src(mksp<NoiseSrc>())
					.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
							 .biquad(sampleRate, FilterPassType::BELL, 740, 0.5, 5)
							 .biquad(sampleRate, FilterPassType::BELL, 1337, 0.5, 5)
							 .biquad(sampleRate, FilterPassType::BELL, 5000, 0.5, 10)
							 .biquad(sampleRate, FilterPassType::BELL, 1254, 10, 7)
							 .biquad(sampleRate, FilterPassType::BELL, 1554, 10, 7)
							 .biquad(sampleRate, FilterPassType::HIGHPASS, 500, 0.5, 0)
							 .build())
					.build(), 0.3)
			.MultiStageEnv(releaseWithCompressorEffect(2, 10, 10, 500, 0.7))
			.mul(0.6)
			.build());
		add(MIDIFile::DrumSet::SNARE_ELECTRIC,
			AmpBuilder()
			.src(mksp<SimpleDrumAmp>(risset(), 210, 160, 0.3f, SimpleDrumAmp::MODE_FIXED, Pow(-5)))
			.arctanDistortion(1, 5, 2)
			.addMul(AmpBuilder()
					.src(mksp<NoiseSrc>())
					.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
							 .biquad(sampleRate, FilterPassType::BELL, 740, 0.5, 5)
							 .biquad(sampleRate, FilterPassType::BELL, 1337, 0.5, 5)
							 .biquad(sampleRate, FilterPassType::BELL, 5000, 0.5, 10)
							 .biquad(sampleRate, FilterPassType::BELL, 1254, 10, 7)
							 .biquad(sampleRate, FilterPassType::BELL, 1554, 10, 7)
							 .biquad(sampleRate, FilterPassType::HIGHPASS, 500, 0.5, 0)
							 .build())
					.build(), 0.3)
			.MultiStageEnv(releaseWithCompressorEffect(2, 10, 10, 500, 0.7))
			.mul(0.6)
			.build());
		add(MIDIFile::DrumSet::HAND_CLAP,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 962, 10.0, 6)
					 .biquad(sampleRate, FilterPassType::BELL, 1122, 10.0, 6)
					 .biquad(sampleRate, FilterPassType::BELL, 1347, 10.0, 6)
					 .biquad(sampleRate, FilterPassType::BELL, 1503, 10.0, 6)
					 .biquad(sampleRate, FilterPassType::BANDPASS, 1243, 0.3, 0)
					 .build())
			.mul(1.3)
			.MultiStageEnv(releaseWithCompressorEffect(3, 7, 7, 500, 1))
			.build());
		add(MIDIFile::DrumSet::CYMBAL_CRASH_1,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(1), 0.12, IntArray{305, 444, 558, 630, 794, 824, 1136}))
			.arctanDistortion(1,10,0.5)
			.noteDSP(DSPGroupBuilder()
					 .begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 4000, 0.5, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 8000, 0.1, 20)
					 .biquad(sampleRate, FilterPassType::HIGHPASS, 200, 0.5, 0)
					 .build())
			.mul(0.05)
			.AR(30, 2000, Pow(2), Pow(3))
			.addMul(AmpBuilder().src(SineAmp(490)).AR(3, 3, Pow(-5), Pow(4)).build(), 0.25)
			.build());
		add(MIDIFile::DrumSet::CYMBAL_CRASH_2,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(0.9), 0.12, IntArray{305, 444, 558, 630, 794, 824, 1136}))
			.arctanDistortion(1, 10, 0.5)
			.noteDSP(DSPGroupBuilder()
					 .begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 3600, 0.5, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 7200, 0.1, 20)
					 .biquad(sampleRate, FilterPassType::HIGHPASS, 200, 0.5, 0)
					 .build())
			.mul(0.05)
			.AR(30, 2000, Pow(2), Pow(3))
			.addMul(AmpBuilder().src(SineAmp(420)).AR(3, 3, Pow(-5), Pow(4)).build(), 0.25)
			.build());
		add(MIDIFile::DrumSet::CYMBAL_SPLASH,
			AmpBuilder(mksp<CymbalOsc>())
			.noteDSP(DSPGroupBuilder()
					 .begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 9347, 0.5, 10)
					 .biquad(sampleRate, FilterPassType::BELL, 831, 0.5, 5)
					 .biquad(sampleRate, FilterPassType::BELL, 1155, 0.5, 5)
					 .biquad(sampleRate, FilterPassType::BELL, 1617, 0.5, 5)
					 .biquad(sampleRate, FilterPassType::BELL, 2182, 0.5, -10)
					 .biquad(sampleRate, FilterPassType::LOWPASS, 17000, 0.7, 0)
					 .biquad(sampleRate, FilterPassType::HIGHPASS, 2000, 1, 0)
					 .build())
			.biquadEnv(AmpBuilder().src(ConstAmp(70))
					   .AR(1, 3000, Line(), Line())
					   .add(ConstAmp(70))
					   .build(), ConstAmp(1), FilterPassType::LOWPASS)
			.mul(0.3)
			.AR(50, 2000, Pow(-5), Pow(4))
			.build());
		add(MIDIFile::DrumSet::CYMBAL_CHINESE,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(0.8), 0.1))
			.IIR(sampleRate, 1, 2000, 10000.0)
			.biquadEnv(AmpBuilder().src(ConstAmp(60))
					   .AR(30, 3000, Line(), Line())
					   .add(ConstAmp(60))
					   .build(), ConstAmp(1), FilterPassType::LOWPASS)
			.AR(100, 2000, Pow(-5), Pow(4))
			.addMul(AmpBuilder().src(SineAmp(390)).AR(3, 3, Pow(-5), Pow(4)).build(), 0.2)
			.mul(1.7)
			.build());
		add(MIDIFile::DrumSet::HI_HAT_CLOSED,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(1), 0.25, IntArray{493, 534, 637, 753, 858, 1028, 1332}))
			.IIR(sampleRate, 2, 1000, 17000.0)
			.mul(0.9)
			.AR(5, 70, Pow(-5), Pow(3))
			.build());
		add(MIDIFile::DrumSet::HI_HAT_PEDAL,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(1), 0.15, IntArray{493, 534, 637, 753, 858, 1028, 1332}))
			.IIR(sampleRate, 2, 1000, 17000.0)
			.mul(0.9)
			.AR(10, 200, Pow(-5), Pow(3))
			.build());
		add(MIDIFile::DrumSet::HI_HAT_OPEN,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(1), 0.15, IntArray{493, 534, 637, 753, 858, 1000, 1028, 1332}))
			.IIR(sampleRate, 2, 1000, 18000.0)
			.mul(0.75)
			.AR(10, 500, Pow(-5), Pow(2))
			.build());
		add(MIDIFile::DrumSet::TAMBOURINE,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 2167, 10.0, 10)
					 .biquad(sampleRate, FilterPassType::BELL, 3072, 10.0, 10)
					 .biquad(sampleRate, FilterPassType::BELL, 4313, 10.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 5724, 10.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 6594, 10.0, 18)
					 .biquad(sampleRate, FilterPassType::BELL, 7595, 10.0, 35)
					 .biquad(sampleRate, FilterPassType::BELL, 9259, 10.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 12286, 10.0, 35)
					 .biquad(sampleRate, FilterPassType::BELL, 14153, 10.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 19875, 10.0, 20)
					 .biquad(sampleRate, FilterPassType::BANDPASS, 2000, 0.7, 0)
					 .build())
			.mul(0.08)
			.AR(5, 500, Pow(5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::BELL_RIDE,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(0.95), 0.12))
			.addMul(AmpBuilder(mksp<NoiseSrc>()).AR(1, 50, Line(), Pow(5)).build(), 1.0)
			.biquad(sampleRate, BANDPASS, 3049, 3, 0)
			.mul(2)
			.AR(1, 2000, Pow(5), Pow(5)).build());
		add(MIDIFile::DrumSet::CYMBAL_RIDE_1,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(1.3), 0.12))
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, BELL, 9982, 1, 20)
					 .biquad(sampleRate, BELL, 3561, 1, 20)
					 .biquad(sampleRate, BELL, 324, 1, 20)
					 .biquad(sampleRate, BELL, 540, 1, 20)
					 .biquad(sampleRate, LOWPASS, 18000, 0.5, 0)
					 .biquad(sampleRate, HIGHPASS, 5100, 0.5, 0)
					 .build())
			.addMul(AmpBuilder(mksp<NoiseSrc>()).AR(1, 50, Line(), Pow(5)).build(), 1.0)
			.mul(0.2)
			.AR(1, 2000, Pow(5), Pow(5)).build());
		add(MIDIFile::DrumSet::CYMBAL_RIDE_2,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(1.2), 0.12))
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, BELL, 9082, 1, 20)
					 .biquad(sampleRate, BELL, 3061, 1, 20)
					 .biquad(sampleRate, BELL, 304, 1, 20)
					 .biquad(sampleRate, BELL, 550, 1, 20)
					 .biquad(sampleRate, LOWPASS, 17000, 0.5, 0)
					 .biquad(sampleRate, HIGHPASS, 5000, 0.5, 0)
					 .build())
			.addMul(AmpBuilder(mksp<NoiseSrc>()).AR(1, 50, Line(), Pow(5)).build(), 1.0)
			.mul(0.2)
			.AR(1, 2000, Pow(5), Pow(5)).build());
		add(MIDIFile::DrumSet::TOM_LOW_FLOOR, tom(sampleRate));
		add(MIDIFile::DrumSet::TOM_HIGH_FLOOR, tom(sampleRate));
		add(MIDIFile::DrumSet::TOM_LOW, tom(sampleRate));
		add(MIDIFile::DrumSet::TOM_LOW_MID, tom(sampleRate));
		add(MIDIFile::DrumSet::TOM_HIGH_MID, tom(sampleRate));
		add(MIDIFile::DrumSet::TOM_HIGH, tom(sampleRate));
		add(MIDIFile::DrumSet::COWBELL,
			AmpBuilder().src(mksp<TriWave>(ConstPhase(800)))
			.addMul(mksp<TriWave>(ConstPhase(540)), 0.15)
			.clampV(0.8, 0.5f)
			.mul(2)
			.AR(10, 500, Pow(-5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::VIBRA_SLAP,
			AmpBuilder().src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 1000, 5.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 2381, 8.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 3962, 8.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 6847, 14.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 8596, 10.0, 25)
					 .biquad(sampleRate, FilterPassType::BANDPASS, 5724, 2.0, 0)
					 .build())
			.mul(1.5)
			.am(mksp<SineWave>(), 0.9, 53)
			.clampV(0.2, 0.3)
			.mul(2)
			.AR(5, 1500, Pow(5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::BONGO_HI,
			AmpBuilder().src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 361, 10.0, 25)
					 .biquad(sampleRate, FilterPassType::BELL, 692, 15.0, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 1029, 15.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 1431, 15.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 1603, 15.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 1812, 15.0, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 3043, 15.0, 20)
					 .biquad(sampleRate, FilterPassType::BANDPASS, 1744, 2.0, 0)
					 .build())
			.mul(0.5)
			.addMul(AmpBuilder().src(mksp<SineWave>()).drum(300, 10, 0.05).build(), 0.05)
			.AR(5, 1000, Pow(5), Pow(20))
			.build());
		add(MIDIFile::DrumSet::BONGO_LOW,
			AmpBuilder().src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 291, 10.0, 25)
					 .biquad(sampleRate, FilterPassType::BELL, 475, 10.0, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 673, 15.0, 50)
					 .biquad(sampleRate, FilterPassType::BELL, 929, 15.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 1331, 15.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 1503, 15.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 1712, 15.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 3243, 15.0, 20)
					 .biquad(sampleRate, FilterPassType::BANDPASS, 1744, 2.0, 0)
					 .build())
			.mul(0.3)
			.addMul(AmpBuilder().src(mksp<SineWave>()).drum(300, 10, 0.1).build(), 0.1)
			.AR(5, 1000, Pow(5), Pow(20))
			.build());
		add(MIDIFile::DrumSet::CONGA_HI,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.add(mksp<SineWave>(ConstPhase(66)))
			.add(mksp<SineWave>(ConstPhase(276)))
			.add(mksp<SineWave>(ConstPhase(326)))
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 66, 30, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 276, 30, 50)
					 .biquad(sampleRate, FilterPassType::BELL, 326, 30, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 495, 30, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 598, 5, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 693, 30, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 1013, 10, 30)
					 .biquad(sampleRate, FilterPassType::LOWPASS, 4000, 0.5, 0)
					 .build())
			.mul(0.04)
			.AR(1, 1000, Pow(5), Pow(15))
			.build());
		add(MIDIFile::DrumSet::CONGA_MUTE,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 66, 30, 50)
					 .biquad(sampleRate, FilterPassType::BELL, 276, 30, 50)
					 .biquad(sampleRate, FilterPassType::BELL, 329, 30, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 502, 30, 50)
					 .biquad(sampleRate, FilterPassType::BELL, 708, 10, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 762, 10, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 991, 10, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 1216, 10, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 1872, 10, 20)
					 .biquad(sampleRate, FilterPassType::LOWPASS, 5000, 0.5, 0)
					 .build())
			.mul(0.04)
			.AR(1, 1000, Pow(5), Pow(15))
			.build());
		add(MIDIFile::DrumSet::CONGA_LOW,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.add(mksp<SineWave>(ConstPhase(48)))
			.add(mksp<SineWave>(ConstPhase(153)))
			.add(mksp<SineWave>(ConstPhase(183)))
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 49, 30, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 80, 30, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 120, 30, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 153, 30, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 183, 30, 50)
					 .biquad(sampleRate, FilterPassType::BELL, 290, 30, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 412, 30, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 447, 30, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 491, 30, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 653, 3, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 908, 7, 20)
					 .biquad(sampleRate, FilterPassType::LOWPASS, 4000, 0.5, 0)
					 .build())
			.mul(0.06)
			.AR(1, 1000, Pow(5), Pow(15))
			.build());
		add(MIDIFile::DrumSet::TIMBALE_HI,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, BELL, 206, 30, 70)
					 .biquad(sampleRate, BELL, 415, 30, 70)
					 .biquad(sampleRate, BELL, 594, 30, 50)
					 .biquad(sampleRate, BELL, 767, 30, 50)
					 .biquad(sampleRate, BELL, 1082, 5, 30)
					 .biquad(sampleRate, BELL, 1318, 15, 20)
					 .biquad(sampleRate, BELL, 1492, 15, 20)
					 .biquad(sampleRate, BELL, 1970, 15, 20)
					 .biquad(sampleRate, BELL, 2658, 15, 20)
					 .biquad(sampleRate, BELL, 4402, 15, 20)
					 .biquad(sampleRate, LOWPASS, 5000, 0.3, 0)
					 .build())
			.mul(0.03)
			.addMul(AmpBuilder()
					.src(mksp<NoiseSrc>())
					.biquad(sampleRate, BANDPASS, 560, 0.8, 0)
					.AR(1, 1000, Pow(5), Pow(15))
					.build(), 0.3)
			.AR(1, 1000, Pow(5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::TIMBALE_LOW,
			AmpBuilder()
			.src(mksp<SineWaveTable>(114, DoubleArray({
				114, 0.3, 215, 1, 306, 0.1, 383, 0.5, 412, 0.5
													  })))
			.arctanDistortion(1, 1.5, 1)
			.drum(125, 114, 2, SimpleDrumAmp::MODE_FIXED, Pow(-15))
			.add(AmpBuilder()
				 .src(mksp<NoiseSrc>())
				 .noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
						  .biquad(sampleRate, BANDPASS, 802, 1, 0)
						  .biquad(sampleRate, LOWPASS, 10000, 0.3, 0)
						  .build())
				 .AR(5, 1000, Pow(5), Pow(10))
				 .build())
			//.mul(0.5)
			.AR(5, 1000, Pow(5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::AGOGO_HI,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.add(mksp<SquareWave>(ConstPhase(668)))
			.add(mksp<SquareWave>(ConstPhase(1559)))
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 668, 30, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 1559, 30, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 2198, 30, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 2777, 30, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 3192, 30, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 3262, 30, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 3667, 30, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 4633, 30, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 5132, 30, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 6438, 30, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 7785, 30, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 8688, 30, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 9347, 30, 40)
					 .biquad(sampleRate, FilterPassType::HIGHPASS, 2000, 0.5, 0)
					 .biquad(sampleRate, FilterPassType::LOWPASS, 8000, 0.5, 0)
					 .build())
			.mul(0.015)
			.AR(1, 1000, Pow(5), Pow(10))
			.build());
		add(MIDIFile::DrumSet::AGOGO_LOW,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.add(mksp<SquareWave>(ConstPhase(529)))
			.add(mksp<SquareWave>(ConstPhase(1243)))
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 529, 30, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 1243, 30, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 1752, 30, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 2182, 30, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 2526, 30, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 2582, 30, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 3667, 30, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 4032, 30, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 5095, 30, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 6207, 30, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 7451, 30, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 8688, 30, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 9347, 30, 40)
					 .biquad(sampleRate, FilterPassType::HIGHPASS, 2000, 0.5, 0)
					 .biquad(sampleRate, FilterPassType::LOWPASS, 8000, 0.5, 0)
					 .build())
			.mul(0.02)
			.AR(1, 1000, Pow(5), Pow(10))
			.build());
		add(MIDIFile::DrumSet::CABASA,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.biquad(sampleRate, FilterPassType::BELL, 13639, 0.7, 30)
			.biquad(sampleRate, FilterPassType::BANDPASS, 7290, 5.0, 0)
			.mul(0.3)
			.AR(100, 100, Pow(5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::MARACAS,
			AmpBuilder().src(mksp<NoiseSrc>())
			.IIR(sampleRate, 2, 5715.8, 20000.0)
			.clampV(0.3, 0.3)
			.mul(3)
			.AR(11, 88, Pow(5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::WHISTLE_HIGH,
			AmpBuilder().src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 2450, 20.0, 50)
					 .biquad(sampleRate, FilterPassType::BELL, 2650, 20.0, 50)
					 .biquad(sampleRate, FilterPassType::BANDPASS, 2520, 5.0, 0)
					 .build())
			.mul(0.005)
			.am(mksp<SineWave>(), 1, 40)
			.ADSR(5, 100, 0.5, false, 1000, Pow(5), Pow(5), Pow(-5))
			.build());
		add(MIDIFile::DrumSet::WHISTLE_LOW,
			AmpBuilder().src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 1850, 20.0, 50)
					 .biquad(sampleRate, FilterPassType::BELL, 2050, 20.0, 50)
					 .biquad(sampleRate, FilterPassType::BANDPASS, 1920, 5.0, 0)
					 .build())
			.mul(0.005)
			.am(mksp<SineWave>(), 1, 40)
			.ADSR(5, 100, 0.5, false, 1000, Pow(5), Pow(5), Pow(-5))
			.build());
		add(MIDIFile::DrumSet::GUIRO_SHORT,
			AmpBuilder().src(mksp<NoiseSrc>())
			.add(0.5)
			.MultiStageEnv(releaseWithCompressorEffect(10, 20, 5, 500, 1))
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 740, 10, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 838, 10, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 1188, 10.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 1418, 10.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 1984, 10.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 2544, 10.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 3238, 10.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 3803, 10.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 4275, 10.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 5984, 3.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 7373, 5.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 9077, 5.0, 20)
					 .biquad(sampleRate, FilterPassType::HIGHSHELF, 1000, 0.7, 10)
					 .build())
			.mul(0.01)
			.AR(5, 500, Pow(5), Pow(-5))
			.build());
		add(MIDIFile::DrumSet::GUIRO_LONG,
			AmpBuilder().src(mksp<NoiseSrc>())
			.add(0.5)
			.MultiStageEnv(releaseWithCompressorEffect(30, 30, 2, 1000, 1))
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 740, 10, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 838, 10, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 1188, 10.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 1418, 10.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 1984, 10.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 2544, 10.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 3238, 10.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 3803, 10.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 4275, 10.0, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 5984, 3.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 7373, 5.0, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 9077, 5.0, 20)
					 .biquad(sampleRate, FilterPassType::HIGHSHELF, 1000, 0.7, 10)
					 .build())
			.mul(0.01)
			.AR(5, 1000, Pow(5), Pow(-5))
			.build());
		add(MIDIFile::DrumSet::CUICA_HIGH,
			AmpBuilder().src(mksp<SimpleDrumAmp>(mksp<Pulse>(), 873, 644, 0.3f, Pow(3)))
			.AR(100, 500, Pow(-5), Pow(5))
			.mul(0.3)
			.build());
		add(MIDIFile::DrumSet::CUICA_LOW,
			AmpBuilder().src(mksp<SimpleDrumAmp>(mksp<Pulse>(0.112, 0.139, 0.617, 0), 321, 288, 0.2f, Pow(3)))
			.AR(100, 300, Pow(-5), Pow(5))
			.mul(0.3)
			.build());
		add(MIDIFile::DrumSet::CLAVES,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 2280, 15, 60)
					 .biquad(sampleRate, FilterPassType::BELL, 4912, 10, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 8315, 10, 20)
					 .biquad(sampleRate, FilterPassType::BELL, 9077, 10, 20)
					 .biquad(sampleRate, FilterPassType::LOWPASS, 10000, 0.5, 0)
					 .build())
			.mul(0.06)
			.AR(1, 1000, Pow(5), Pow(15))
			.build());
		add(MIDIFile::DrumSet::WOOD_BLOCK_HIGH,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 740, 10, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 838, 10, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 1139, 15, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 1641, 15, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 1844, 15, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 2382, 10, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 3010, 5, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 3286, 15, 25)
					 .biquad(sampleRate, FilterPassType::BELL, 4032, 15, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 4667, 15, 30)
					 .biquad(sampleRate, FilterPassType::LOWPASS, 10000, 0.5, 0)
					 .build())
			.mul(0.02)
			.AR(1, 1000, Pow(5), Pow(20))
			.build());
		add(MIDIFile::DrumSet::WOOD_BLOCK_LOW,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 548, 10, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 612, 10, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 825, 15, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 1216, 15, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 1337, 15, 40)
					 .biquad(sampleRate, FilterPassType::BELL, 1727, 10, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 2166, 5, 15)
					 .biquad(sampleRate, FilterPassType::BELL, 2417, 15, 25)
					 .biquad(sampleRate, FilterPassType::BELL, 2923, 15, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 3408, 15, 30)
					 .biquad(sampleRate, FilterPassType::LOWPASS, 10000, 0.5, 0)
					 .build())
			.mul(0.025)
			.AR(1, 1000, Pow(5), Pow(20))
			.build());
		add(MIDIFile::DrumSet::TRIANGLE_OPEN,
			AmpBuilder()
			.src(mksp<SineWave>(ConstPhase(5512)))
			.addMul(mksp<SineWave>(ConstPhase(1696)), 0.2)
			.addMul(mksp<SineWave>(ConstPhase(2692)), 0.5)
			.addMul(mksp<SineWave>(ConstPhase(3101)), 1.0)
			.addMul(mksp<SineWave>(ConstPhase(8505)), 0.5)
			.addMul(mksp<SineWave>(ConstPhase(10972)), 0.7)
			.mul(0.18)
			.AR(5, 1000, Pow(5), Pow(3))
			.build());
		add(MIDIFile::DrumSet::TRIANGLE_MUTE,
			AmpBuilder()
			.src(mksp<SineWave>(ConstPhase(5512)))
			.addMul(mksp<SineWave>(ConstPhase(1696)), 0.2)
			.addMul(mksp<SineWave>(ConstPhase(2692)), 0.5)
			.addMul(mksp<SineWave>(ConstPhase(3101)), 1.0)
			.addMul(mksp<SineWave>(ConstPhase(8505)), 0.5)
			.addMul(mksp<SineWave>(ConstPhase(10972)), 0.7)
			.mul(0.18)
			.AR(5, 100, Pow(5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::SHAKER,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.biquad(sampleRate, FilterPassType::BELL, 14639, 0.7, 30)
			.biquad(sampleRate, FilterPassType::BANDPASS, 6290, 4.0, 0)
			.mul(0.3)
			.AR(100, 100, Pow(5), Pow(5))
			.build());

		add(MIDIFile::DrumSet::JINGLE_BELL,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 2568, 10, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 2697, 10, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 4948, 10, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 5409, 10, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 6114, 10, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 7043, 10, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 9172, 15, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 10869, 10, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 11944, 10, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 14559, 20, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 16772, 10, 30)
					 .biquad(sampleRate, FilterPassType::HIGHPASS, 2000, 1, 0)
					 .build())
			.mul(0.009)
			.AR(10, 1000, Pow(5), Pow(3))
			.build());

		add(MIDIFile::DrumSet::BELL_TREE,
			AmpBuilder()
			.src(mksp<SquareWave>(ConstPhase(5058)))
			.AR(5, 3000, 3000, Pow(5), Pow(2))
			.add(AmpBuilder().src(mksp<SquareWave>(ConstPhase(4736)))
				 .DAHDSR(50, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(mksp<SquareWave>(ConstPhase(4092)))
				 .DAHDSR(100, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(mksp<SquareWave>(ConstPhase(3535)))
				 .DAHDSR(150, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(mksp<SquareWave>(ConstPhase(3032)))
				 .DAHDSR(200, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(mksp<SquareWave>(ConstPhase(2717)))
				 .DAHDSR(250, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(mksp<SquareWave>(ConstPhase(2435)))
				 .DAHDSR(300, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(mksp<SquareWave>(ConstPhase(2182)))
				 .DAHDSR(350, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(mksp<SquareWave>(ConstPhase(1970)))
				 .DAHDSR(400, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(mksp<SquareWave>(ConstPhase(1765)))
				 .DAHDSR(480, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(mksp<SquareWave>(ConstPhase(1617)))
				 .DAHDSR(560, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(mksp<SquareWave>(ConstPhase(1470)))
				 .DAHDSR(640, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(mksp<SquareWave>(ConstPhase(1289)))
				 .DAHDSR(720, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(mksp<SquareWave>(ConstPhase(1139)))
				 .DAHDSR(800, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(mksp<SquareWave>(ConstPhase(1043)))
				 .DAHDSR(880, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.mul(0.1)
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, BELL, 10000, 4, 20)
					 .biquad(sampleRate, BELL, 6581, 4, 20)
					 .biquad(sampleRate, HIGHPASS, 6000, 0.5, 0)
					 .build())
			.AR(5, 3000, 3000, Pow(5), Pow(2))
			.build());
		add(MIDIFile::DrumSet::CASTANETS,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, FilterPassType::BELL, 1618, 5, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 4522, 10, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 7110, 15, 30)
					 .biquad(sampleRate, FilterPassType::BELL, 11287, 15, 30)
					 .biquad(sampleRate, FilterPassType::HIGHPASS, 1000, 1.0, 0)
					 .build())
			.mul(0.085)
			.AR(3, 500, Pow(5), Pow(20))
			.build());
		add(MIDIFile::DrumSet::SURDO_MUTE,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(sampleRate, BELL, 153, 10, 5)
					 .biquad(sampleRate, BELL, 179, 10, 5)
					 .biquad(sampleRate, BELL, 215, 10, 5)
					 .biquad(sampleRate, BELL, 280, 10, 5)
					 .biquad(sampleRate, BELL, 303, 10, 5)
					 .biquad(sampleRate, BELL, 343, 10, 5)
					 .biquad(sampleRate, BELL, 400, 5, 5)
					 .biquad(sampleRate, BELL, 498, 5, 5)
					 .biquad(sampleRate, BELL, 649, 3, 5)
					 .biquad(sampleRate, BELL, 838, 10, 5)
					 .biquad(sampleRate, BELL, 1000, 10, 5)
					 .biquad(sampleRate, BANDPASS, 180, 2, 0)
					 .build())
			.mul(3)
			.AR(5, 1000, Pow(5), Pow(20))
			.build());

		add(MIDIFile::DrumSet::SURDO_OPEN,
			AmpBuilder()
			.src(mksp<SineWaveTable>(84, DoubleArray({
				84, 1, 108, 0.1, 143, 0.1
													 })))
			.drum(90, 84, 2)
			.arctanDistortion(1, 3, 1)
			.add(AmpBuilder()
				 .src(mksp<NoiseSrc>())
				 .noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
						  .biquad(sampleRate, BELL, 153, 10, 5)
						  .biquad(sampleRate, BELL, 179, 10, 5)
						  .biquad(sampleRate, BELL, 215, 10, 5)
						  .biquad(sampleRate, BELL, 280, 10, 5)
						  .biquad(sampleRate, BELL, 303, 10, 5)
						  .biquad(sampleRate, BELL, 343, 10, 5)
						  .biquad(sampleRate, BELL, 400, 5, 5)
						  .biquad(sampleRate, BELL, 498, 5, 5)
						  .biquad(sampleRate, BELL, 649, 3, 5)
						  .biquad(sampleRate, BELL, 838, 10, 5)
						  .biquad(sampleRate, BELL, 1000, 10, 5)
						  .biquad(sampleRate, BANDPASS, 180, 2, 0)
						  .build())
				 .AR(5, 1000, Pow(5), Pow(20))
				 .build())
			.mul(1.5)
			.AR(5, 1000, Pow(5), Pow(5))
			.build());

		//以下非GM标准音色映射

		add(22, AmpBuilder()
			.src(mksp<TriWave>(ConstPhase(1066)))
			.AR(10, 100, Pow(-5), Pow(5))
			.build());
		add(23, AmpBuilder()
			.src(mksp<TriWave>(ConstPhase(2135)))
			.AR(10, 100, Pow(-5), Pow(5))
			.build());
		//GS标准附加底鼓
		for(s_note_id i=1;i < 16;i++){
			add(i, AmpBuilder().src(mksp<SimpleDrumAmp>(mksp<SineWave>(), 350, 50, 1.0f, SimpleDrumAmp::MODE_FIXED, Pow(-50)))
				.AR(10, 500, 500, Pow(-5), Pow(10))
				.arctanDistortion(1, 10, 1)
				.add(AmpBuilder().src(mksp<SimpleDrumAmp>(mksp<SineWave>(), 1000, 100, 0.1f, SimpleDrumAmp::MODE_FIXED, Pow(-50)))
					 .AR(10, 100, 100, Pow(-5), Pow(5))
					 .arctanDistortion(1, 10, 0.3)
					 .build())
				.build());
		}
		//GS标准附加军鼓
		for(s_note_id i=97;i < 127;i++){
			add(i,
				AmpBuilder()
				.src(mksp<SimpleDrumAmp>(risset(), 180, 130, 0.3f, SimpleDrumAmp::MODE_FIXED, Pow(-5)))
				.arctanDistortion(1, 5, 1.5)
				.addMul(AmpBuilder()
						.src(mksp<NoiseSrc>())
						.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
								 .biquad(sampleRate, FilterPassType::BELL, 740, 0.5, 5)
								 .biquad(sampleRate, FilterPassType::BELL, 1337, 0.5, 5)
								 .biquad(sampleRate, FilterPassType::BELL, 5000, 0.5, 10)
								 .biquad(sampleRate, FilterPassType::BELL, 1254, 10, 7)
								 .biquad(sampleRate, FilterPassType::BELL, 1554, 10, 7)
								 .biquad(sampleRate, FilterPassType::HIGHPASS, 500, 0.5, 0)
								 .build())
						.build(), 0.2)
				.MultiStageEnv(releaseWithCompressorEffect(2, 10, 10, 500, 0.7))
				.mul(0.8)
				.build());
		}

		NonInterpolateAmpSet::init(cfg);
	}
}