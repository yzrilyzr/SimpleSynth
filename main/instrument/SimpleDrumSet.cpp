/**
 * 默认简单鼓组，开箱即用
 * 不喜欢的话，可以直接删掉，没有任何影响(～￣▽￣)～
 */

#include "SimpleDrumSet.h"
#include "SimpleSynth.h"
#include "dsp/DSPGroupBuilder.h"
#include "interpolator/GraphInterpolator.h"
#include "synth/util/AmpBuilder.h"
#include "synth/modulation/SimpleDrumAmp.h"
#include "synth/osc/noise/NoiseSrc.h"
#include "synth/osc/pulse/CymbalOsc.h"
#include "synth/osc/pulse/Pulse.h"
#include "synth/osc/pulse/SquareWave.h"
#include "synth/osc/pulse/TriWave.h"
#include "synth/osc/sine/SineWave.h"
#include "synth/util/AmplitudeSources.h"
#include "synth/util/EnvUtil.h"
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
					.iir(ButterworthParams{FilterPassType::BANDPASS, 100, 2000.0, sampleRate, 2})
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
			.iir(ButterworthParams{FilterPassType::BANDPASS, 748.2, 3544.7, sampleRate, 2})
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
					 .biquad(RBJParams{FilterPassType::BELL, 1545, sampleRate, 10.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 2348, sampleRate, 10.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 3459, sampleRate, 10.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 4331, sampleRate, 15.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 4537, sampleRate, 15.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 5549, sampleRate, 10.0, 20})
					 .biquad(RBJParams{FilterPassType::BANDPASS, 3000, sampleRate, 1.5, 0})
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
					 .biquad(RBJParams{FilterPassType::BELL, 484, sampleRate, 5.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 1174, sampleRate, 5.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 1917, sampleRate, 10.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 2544, sampleRate, 4.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 3220, sampleRate, 10.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 4154, sampleRate, 10.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 5549, sampleRate, 10.0, 15})
					 .biquad(RBJParams{FilterPassType::BANDPASS, 2000, sampleRate, 0.7, 0})
					 .build())
			.mul(0.4)
			.AR(5, 500, Pow(5), Pow(50))
			.build());
		add(MIDIFile::DrumSet::METRONOME_BELL,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 484, sampleRate, 5.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 1174, sampleRate, 5.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 1917, sampleRate, 10.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 2544, sampleRate, 4.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 3220, sampleRate, 10.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 4154, sampleRate, 10.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 5549, sampleRate, 10.0, 15})
					 .biquad(RBJParams{FilterPassType::BANDPASS, 2000, sampleRate, 0.7, 0})
					 .build())
			.mul(0.38)
			.AR(5, 500, Pow(5), Pow(50))
			.add(AmpBuilder()
				 .src(SineAmp(5460))
				 .addMul(SineAmp(2048), 0.3)
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
			.clampV(1,1,1)
			.build());
		add(MIDIFile::DrumSet::BASS_DRUM_1, AmpBuilder().src(mksp<SimpleDrumAmp>(mksp<SineWave>(), 500, 40, 1.0f, SimpleDrumAmp::MODE_FIXED, Pow(-50)))
			.AR(10, 1000, 1000, Pow(-5), Pow(10))
			.arctanDistortion(1.0, 15, 1)
			.add(AmpBuilder().src(mksp<SimpleDrumAmp>(mksp<SineWave>(), 1000, 100, 0.1f, SimpleDrumAmp::MODE_FIXED, Pow(-50)))
				 .AR(10, 100, 100, Pow(-5), Pow(5))
				 .arctanDistortion(1, 10, 0.3)
				 .build())
			.clampV(1, 1, 1)
			.build());
		add(MIDIFile::DrumSet::SIDE_STICK,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 211, sampleRate, 3.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 338, sampleRate, 7.0, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 700, sampleRate, 5.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 1163, sampleRate, 5.0, 18})
					 .biquad(RBJParams{FilterPassType::BELL, 1953, sampleRate, 4.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 2986, sampleRate, 5.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 4651, sampleRate, 5.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 6231, sampleRate, 7.0, 15})
					 .biquad(RBJParams{FilterPassType::BANDPASS, 2000, sampleRate, 0.7, 0})
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
							 .biquad(RBJParams{FilterPassType::BELL, 740, sampleRate, 0.5, 5})
							 .biquad(RBJParams{FilterPassType::BELL, 1337, sampleRate, 0.5, 5})
							 .biquad(RBJParams{FilterPassType::BELL, 5000, sampleRate, 0.5, 10})
							 .biquad(RBJParams{FilterPassType::BELL, 1254, sampleRate, 10, 7})
							 .biquad(RBJParams{FilterPassType::BELL, 1554, sampleRate, 10, 7})
							 .biquad(RBJParams{FilterPassType::HIGHPASS, 500, sampleRate, 0.5, 0})
							 .build())
					.build(), 0.25)
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
							 .biquad(RBJParams{FilterPassType::BELL, 740, sampleRate, 0.5, 5})
							 .biquad(RBJParams{FilterPassType::BELL, 1337, sampleRate, 0.5, 5})
							 .biquad(RBJParams{FilterPassType::BELL, 5000, sampleRate, 0.5, 10})
							 .biquad(RBJParams{FilterPassType::BELL, 1254, sampleRate, 10, 7})
							 .biquad(RBJParams{FilterPassType::BELL, 1554, sampleRate, 10, 7})
							 .biquad(RBJParams{FilterPassType::HIGHPASS, 500, sampleRate, 0.5, 0})
							 .build())
					.build(), 0.25)
			.MultiStageEnv(releaseWithCompressorEffect(2, 10, 10, 500, 0.7))
			.mul(0.6)
			.build());
		add(MIDIFile::DrumSet::HAND_CLAP,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 962, sampleRate, 10.0, 6})
					 .biquad(RBJParams{FilterPassType::BELL, 1122, sampleRate, 10.0, 6})
					 .biquad(RBJParams{FilterPassType::BELL, 1347, sampleRate, 10.0, 6})
					 .biquad(RBJParams{FilterPassType::BELL, 1503, sampleRate, 10.0, 6})
					 .biquad(RBJParams{FilterPassType::BANDPASS, 1243, sampleRate, 0.3, 0})
					 .build())
			.mul(1.3)
			.MultiStageEnv(releaseWithCompressorEffect(3, 7, 7, 500, 1))
			.build());
		add(MIDIFile::DrumSet::CYMBAL_CRASH_1,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(1), 0.05, IntArray{305, 444, 558, 630, 794, 824, 1136}))
			.arctanDistortion(1, 10, 0.15)
			.noteDSP(DSPGroupBuilder()
					 .begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 4000, sampleRate, 0.5, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 500, sampleRate, 0.8, 19})
					 .biquad(RBJParams{FilterPassType::HIGHPASS, 652, sampleRate, 0.5, 0})
					 .build())
			.AR(50, 2000, Line(), Pow(3))
			.addMul(AmpBuilder().src(SineAmp(490)).AR(3, 3, Pow(-5), Pow(4)).build(), 1)
			.build());
		add(MIDIFile::DrumSet::CYMBAL_CRASH_2,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(0.9), 0.05, IntArray{305, 444, 558, 630, 794, 824, 1136}))
			.arctanDistortion(1, 10, 0.15)
			.noteDSP(DSPGroupBuilder()
					 .begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 3600, sampleRate, 0.5, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 480, sampleRate, 0.8, 19})
					 .biquad(RBJParams{FilterPassType::HIGHPASS, 546, sampleRate, 0.5, 0})
					 .build())
			.AR(50, 2000, Line(), Pow(3))
			.addMul(AmpBuilder().src(SineAmp(420)).AR(3, 3, Pow(-5), Pow(4)).build(), 1)
			.build());
		add(MIDIFile::DrumSet::CYMBAL_SPLASH,
			AmpBuilder(mksp<CymbalOsc>())
			.noteDSP(DSPGroupBuilder()
					 .begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 9347, sampleRate, 0.5, 10})
					 .biquad(RBJParams{FilterPassType::BELL, 831, sampleRate, 0.5, 5})
					 .biquad(RBJParams{FilterPassType::BELL, 1155, sampleRate, 0.5, 5})
					 .biquad(RBJParams{FilterPassType::BELL, 1617, sampleRate, 0.5, 5})
					 .biquad(RBJParams{FilterPassType::BELL, 2182, sampleRate, 0.5, -10})
					 .biquad(RBJParams{FilterPassType::LOWPASS, 17000, sampleRate, 0.7, 0})
					 .biquad(RBJParams{FilterPassType::HIGHPASS, 2000, sampleRate, 1, 0})
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
			.iir(ButterworthParams{FilterPassType::BANDPASS, 2000, 10000.0, sampleRate, 1})
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
			.iir(ButterworthParams{FilterPassType::BANDPASS, 1000, 17000.0, sampleRate, 2})
			.mul(0.9)
			.AR(5, 70, Pow(-5), Pow(3))
			.build());
		add(MIDIFile::DrumSet::HI_HAT_PEDAL,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(1), 0.15, IntArray{493, 534, 637, 753, 858, 1028, 1332}))
			.iir(ButterworthParams{FilterPassType::BANDPASS, 1000, 17000.0, sampleRate, 2})
			.mul(0.9)
			.AR(10, 200, Pow(-5), Pow(3))
			.build());
		add(MIDIFile::DrumSet::HI_HAT_OPEN,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(1), 0.15, IntArray{493, 534, 637, 753, 858, 1000, 1028, 1332}))
			.iir(ButterworthParams{FilterPassType::BANDPASS, 1000, 18000.0, sampleRate, 2})
			.mul(0.75)
			.AR(10, 500, Pow(-5), Pow(2))
			.build());
		add(MIDIFile::DrumSet::TAMBOURINE,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 2167, sampleRate, 10.0, 10})
					 .biquad(RBJParams{FilterPassType::BELL, 3072, sampleRate, 10.0, 10})
					 .biquad(RBJParams{FilterPassType::BELL, 4313, sampleRate, 10.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 5724, sampleRate, 10.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 6594, sampleRate, 10.0, 18})
					 .biquad(RBJParams{FilterPassType::BELL, 7595, sampleRate, 10.0, 35})
					 .biquad(RBJParams{FilterPassType::BELL, 9259, sampleRate, 10.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 12286, sampleRate, 10.0, 35})
					 .biquad(RBJParams{FilterPassType::BELL, 14153, sampleRate, 10.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 19875, sampleRate, 10.0, 20})
					 .biquad(RBJParams{FilterPassType::BANDPASS, 2000, sampleRate, 0.7, 0})
					 .build())
			.mul(0.08)
			.AR(5, 500, Pow(5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::BELL_RIDE,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(0.95), 0.12))
			.addMul(AmpBuilder(mksp<NoiseSrc>()).AR(1, 50, Line(), Pow(5)).build(), 1.0)
			.biquad(RBJParams{FilterPassType::BANDPASS, 3049, sampleRate, 3, 0})
			.mul(2)
			.AR(1, 2000, Pow(5), Pow(5)).build());
		add(MIDIFile::DrumSet::CYMBAL_RIDE_1,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(1.3), 0.12))
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 9982, sampleRate, 1, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 3561, sampleRate, 1, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 324, sampleRate, 1, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 540, sampleRate, 1, 20})
					 .biquad(RBJParams{FilterPassType::LOWPASS, 18000, sampleRate, 0.5, 0})
					 .biquad(RBJParams{FilterPassType::HIGHPASS, 5100, sampleRate, 0.5, 0})
					 .build())
			.addMul(AmpBuilder(mksp<NoiseSrc>()).AR(1, 50, Line(), Pow(5)).build(), 1.0)
			.mul(0.2)
			.AR(1, 2000, Pow(5), Pow(5)).build());
		add(MIDIFile::DrumSet::CYMBAL_RIDE_2,
			AmpBuilder(mksp<CymbalOsc>(ConstAmp(1.2), 0.12))
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 9082, sampleRate, 1, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 3061, sampleRate, 1, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 304, sampleRate, 1, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 550, sampleRate, 1, 20})
					 .biquad(RBJParams{FilterPassType::LOWPASS, 17000, sampleRate, 0.5, 0})
					 .biquad(RBJParams{FilterPassType::HIGHPASS, 5000, sampleRate, 0.5, 0})
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
			AmpBuilder().src(ConstFreq(mksp<TriWave>(), 800))
			.addMul(ConstFreq(mksp<TriWave>(), 540), 0.15)
			.clampV(0.8, 0.5f)
			.mul(2)
			.AR(10, 500, Pow(-5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::VIBRA_SLAP,
			AmpBuilder().src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 1000, sampleRate, 5.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 2381, sampleRate, 8.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 3962, sampleRate, 8.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 6847, sampleRate, 14.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 8596, sampleRate, 10.0, 25})
					 .biquad(RBJParams{FilterPassType::BANDPASS, 5724, sampleRate, 2.0, 0})
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
					 .biquad(RBJParams{FilterPassType::BELL, 361, sampleRate, 10.0, 25})
					 .biquad(RBJParams{FilterPassType::BELL, 692, sampleRate, 15.0, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 1029, sampleRate, 15.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 1431, sampleRate, 15.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 1603, sampleRate, 15.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 1812, sampleRate, 15.0, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 3043, sampleRate, 15.0, 20})
					 .biquad(RBJParams{FilterPassType::BANDPASS, 1744, sampleRate, 2.0, 0})
					 .build())
			.mul(0.5)
			.addMul(AmpBuilder().src(mksp<SineWave>()).drum(300, 10, 0.05).build(), 0.05)
			.AR(5, 1000, Pow(5), Pow(20))
			.build());
		add(MIDIFile::DrumSet::BONGO_LOW,
			AmpBuilder().src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 291, sampleRate, 10.0, 25})
					 .biquad(RBJParams{FilterPassType::BELL, 475, sampleRate, 10.0, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 673, sampleRate, 15.0, 50})
					 .biquad(RBJParams{FilterPassType::BELL, 929, sampleRate, 15.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 1331, sampleRate, 15.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 1503, sampleRate, 15.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 1712, sampleRate, 15.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 3243, sampleRate, 15.0, 20})
					 .biquad(RBJParams{FilterPassType::BANDPASS, 1744, sampleRate, 2.0, 0})
					 .build())
			.mul(0.3)
			.addMul(AmpBuilder().src(mksp<SineWave>()).drum(300, 10, 0.1).build(), 0.1)
			.AR(5, 1000, Pow(5), Pow(20))
			.build());
		add(MIDIFile::DrumSet::CONGA_HI,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.add(SineAmp(66))
			.add(SineAmp(276))
			.add(SineAmp(326))
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 66, sampleRate, 30, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 276, sampleRate, 30, 50})
					 .biquad(RBJParams{FilterPassType::BELL, 326, sampleRate, 30, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 495, sampleRate, 30, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 598, sampleRate, 5, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 693, sampleRate, 30, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 1013, sampleRate, 10, 30})
					 .biquad(RBJParams{FilterPassType::LOWPASS, 4000, sampleRate, 0.5, 0})
					 .build())
			.mul(0.04)
			.AR(1, 1000, Pow(5), Pow(15))
			.build());
		add(MIDIFile::DrumSet::CONGA_MUTE,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 66, sampleRate, 30, 50})
					 .biquad(RBJParams{FilterPassType::BELL, 276, sampleRate, 30, 50})
					 .biquad(RBJParams{FilterPassType::BELL, 329, sampleRate, 30, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 502, sampleRate, 30, 50})
					 .biquad(RBJParams{FilterPassType::BELL, 708, sampleRate, 10, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 762, sampleRate, 10, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 991, sampleRate, 10, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 1216, sampleRate, 10, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 1872, sampleRate, 10, 20})
					 .biquad(RBJParams{FilterPassType::LOWPASS, 5000, sampleRate, 0.5, 0})
					 .build())
			.mul(0.04)
			.AR(1, 1000, Pow(5), Pow(15))
			.build());
		add(MIDIFile::DrumSet::CONGA_LOW,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.add(SineAmp(48))
			.add(SineAmp(153))
			.add(SineAmp(183))
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 49, sampleRate, 30, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 80, sampleRate, 30, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 120, sampleRate, 30, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 153, sampleRate, 30, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 183, sampleRate, 30, 50})
					 .biquad(RBJParams{FilterPassType::BELL, 290, sampleRate, 30, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 412, sampleRate, 30, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 447, sampleRate, 30, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 491, sampleRate, 30, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 653, sampleRate, 3, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 908, sampleRate, 7, 20})
					 .biquad(RBJParams{FilterPassType::LOWPASS, 4000, sampleRate, 0.5, 0})
					 .build())
			.mul(0.06)
			.AR(1, 1000, Pow(5), Pow(15))
			.build());
		add(MIDIFile::DrumSet::TIMBALE_HI,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 206, sampleRate, 30, 70})
					 .biquad(RBJParams{FilterPassType::BELL, 415, sampleRate, 30, 70})
					 .biquad(RBJParams{FilterPassType::BELL, 594, sampleRate, 30, 50})
					 .biquad(RBJParams{FilterPassType::BELL, 767, sampleRate, 30, 50})
					 .biquad(RBJParams{FilterPassType::BELL, 1082, sampleRate, 5, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 1318, sampleRate, 15, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 1492, sampleRate, 15, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 1970, sampleRate, 15, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 2658, sampleRate, 15, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 4402, sampleRate, 15, 20})
					 .biquad(RBJParams{FilterPassType::LOWPASS, 5000, sampleRate, 0.3, 0})
					 .build())
			.mul(0.03)
			.addMul(AmpBuilder()
					.src(mksp<NoiseSrc>())
					.biquad(RBJParams{FilterPassType::BANDPASS, 560, sampleRate, 0.8, 0})
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
						  .biquad(RBJParams{FilterPassType::BANDPASS, 802, sampleRate, 1, 0})
						  .biquad(RBJParams{FilterPassType::LOWPASS, 10000, sampleRate, 0.3, 0})
						  .build())
				 .AR(5, 1000, Pow(5), Pow(10))
				 .build())
			//.mul(0.5)
			.AR(5, 1000, Pow(5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::AGOGO_HI,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.add(ConstFreq(mksp<SquareWave>(), 668))
			.add(ConstFreq(mksp<SquareWave>(), 1559))
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 668, sampleRate, 30, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 1559, sampleRate, 30, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 2198, sampleRate, 30, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 2777, sampleRate, 30, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 3192, sampleRate, 30, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 3262, sampleRate, 30, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 3667, sampleRate, 30, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 4633, sampleRate, 30, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 5132, sampleRate, 30, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 6438, sampleRate, 30, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 7785, sampleRate, 30, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 8688, sampleRate, 30, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 9347, sampleRate, 30, 40})
					 .biquad(RBJParams{FilterPassType::HIGHPASS, 2000, sampleRate, 0.5, 0})
					 .biquad(RBJParams{FilterPassType::LOWPASS, 8000, sampleRate, 0.5, 0})
					 .build())
			.mul(0.015)
			.AR(1, 1000, Pow(5), Pow(10))
			.build());
		add(MIDIFile::DrumSet::AGOGO_LOW,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.add(ConstFreq(mksp<SquareWave>(), 529))
			.add(ConstFreq(mksp<SquareWave>(), 1243))
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 529, sampleRate, 30, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 1243, sampleRate, 30, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 1752, sampleRate, 30, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 2182, sampleRate, 30, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 2526, sampleRate, 30, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 2582, sampleRate, 30, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 3667, sampleRate, 30, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 4032, sampleRate, 30, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 5095, sampleRate, 30, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 6207, sampleRate, 30, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 7451, sampleRate, 30, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 8688, sampleRate, 30, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 9347, sampleRate, 30, 40})
					 .biquad(RBJParams{FilterPassType::HIGHPASS, 2000, sampleRate, 0.5, 0})
					 .biquad(RBJParams{FilterPassType::LOWPASS, 8000, sampleRate, 0.5, 0})
					 .build())
			.mul(0.02)
			.AR(1, 1000, Pow(5), Pow(10))
			.build());
		add(MIDIFile::DrumSet::CABASA,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.biquad(RBJParams{FilterPassType::BELL, 13639, sampleRate, 0.7, 30})
			.biquad(RBJParams{FilterPassType::BANDPASS, 7290, sampleRate, 5.0, 0})
			.mul(0.3)
			.AR(100, 100, Pow(5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::MARACAS,
			AmpBuilder().src(mksp<NoiseSrc>())
			.iir(ButterworthParams{FilterPassType::BANDPASS, 5715.8, 20000.0, sampleRate, 2})
			.clampV(0.3, 0.3)
			.mul(3)
			.AR(11, 88, Pow(5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::WHISTLE_HIGH,
			AmpBuilder().src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 2450, sampleRate, 20.0, 50})
					 .biquad(RBJParams{FilterPassType::BELL, 2650, sampleRate, 20.0, 50})
					 .biquad(RBJParams{FilterPassType::BANDPASS, 2520, sampleRate, 5.0, 0})
					 .build())
			.mul(0.005)
			.am(mksp<SineWave>(), 1, 40)
			.ADSR(5, 100, 0.5, false, 1000, Pow(5), Pow(5), Pow(-5))
			.build());
		add(MIDIFile::DrumSet::WHISTLE_LOW,
			AmpBuilder().src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 1850, sampleRate, 20.0, 50})
					 .biquad(RBJParams{FilterPassType::BELL, 2050, sampleRate, 20.0, 50})
					 .biquad(RBJParams{FilterPassType::BANDPASS, 1920, sampleRate, 5.0, 0})
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
					 .biquad(RBJParams{FilterPassType::BELL, 740, sampleRate, 10, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 838, sampleRate, 10, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 1188, sampleRate, 10.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 1418, sampleRate, 10.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 1984, sampleRate, 10.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 2544, sampleRate, 10.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 3238, sampleRate, 10.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 3803, sampleRate, 10.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 4275, sampleRate, 10.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 5984, sampleRate, 3.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 7373, sampleRate, 5.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 9077, sampleRate, 5.0, 20})
					 .biquad(RBJParams{FilterPassType::HIGHSHELF, 1000, sampleRate, 0.7, 10})
					 .build())
			.mul(0.01)
			.AR(5, 500, Pow(5), Pow(-5))
			.build());
		add(MIDIFile::DrumSet::GUIRO_LONG,
			AmpBuilder().src(mksp<NoiseSrc>())
			.add(0.5)
			.MultiStageEnv(releaseWithCompressorEffect(30, 30, 2, 1000, 1))
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 740, sampleRate, 10, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 838, sampleRate, 10, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 1188, sampleRate, 10.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 1418, sampleRate, 10.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 1984, sampleRate, 10.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 2544, sampleRate, 10.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 3238, sampleRate, 10.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 3803, sampleRate, 10.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 4275, sampleRate, 10.0, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 5984, sampleRate, 3.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 7373, sampleRate, 5.0, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 9077, sampleRate, 5.0, 20})
					 .biquad(RBJParams{FilterPassType::HIGHSHELF, 1000, sampleRate, 0.7, 10})
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
					 .biquad(RBJParams{FilterPassType::BELL, 2280, sampleRate, 15, 60})
					 .biquad(RBJParams{FilterPassType::BELL, 4912, sampleRate, 10, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 8315, sampleRate, 10, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 9077, sampleRate, 10, 20})
					 .biquad(RBJParams{FilterPassType::LOWPASS, 10000, sampleRate, 0.5, 0})
					 .build())
			.mul(0.06)
			.AR(1, 1000, Pow(5), Pow(15))
			.build());
		add(MIDIFile::DrumSet::WOOD_BLOCK_HIGH,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 740, sampleRate, 10, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 838, sampleRate, 10, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 1139, sampleRate, 15, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 1641, sampleRate, 15, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 1844, sampleRate, 15, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 2382, sampleRate, 10, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 3010, sampleRate, 5, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 3286, sampleRate, 15, 25})
					 .biquad(RBJParams{FilterPassType::BELL, 4032, sampleRate, 15, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 4667, sampleRate, 15, 30})
					 .biquad(RBJParams{FilterPassType::LOWPASS, 10000, sampleRate, 0.5, 0})
					 .build())
			.mul(0.02)
			.AR(1, 1000, Pow(5), Pow(20))
			.build());
		add(MIDIFile::DrumSet::WOOD_BLOCK_LOW,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 548, sampleRate, 10, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 612, sampleRate, 10, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 825, sampleRate, 15, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 1216, sampleRate, 15, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 1337, sampleRate, 15, 40})
					 .biquad(RBJParams{FilterPassType::BELL, 1727, sampleRate, 10, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 2166, sampleRate, 5, 15})
					 .biquad(RBJParams{FilterPassType::BELL, 2417, sampleRate, 15, 25})
					 .biquad(RBJParams{FilterPassType::BELL, 2923, sampleRate, 15, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 3408, sampleRate, 15, 30})
					 .biquad(RBJParams{FilterPassType::LOWPASS, 10000, sampleRate, 0.5, 0})
					 .build())
			.mul(0.025)
			.AR(1, 1000, Pow(5), Pow(20))
			.build());
		add(MIDIFile::DrumSet::TRIANGLE_OPEN,
			AmpBuilder()
			.src(SineAmp(5512))
			.addMul(SineAmp(1696), 0.2)
			.addMul(SineAmp(2692), 0.5)
			.addMul(SineAmp(3101), 1.0)
			.addMul(SineAmp(8505), 0.5)
			.addMul(SineAmp(10972), 0.7)
			.mul(0.18)
			.AR(5, 1000, Pow(5), Pow(3))
			.build());
		add(MIDIFile::DrumSet::TRIANGLE_MUTE,
			AmpBuilder()
			.src(SineAmp(5512))
			.addMul(SineAmp(1696), 0.2)
			.addMul(SineAmp(2692), 0.5)
			.addMul(SineAmp(3101), 1.0)
			.addMul(SineAmp(8505), 0.5)
			.addMul(SineAmp(10972), 0.7)
			.mul(0.18)
			.AR(5, 100, Pow(5), Pow(5))
			.build());
		add(MIDIFile::DrumSet::SHAKER,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.biquad(RBJParams{FilterPassType::BELL, 14639, sampleRate, 0.7, 30})
			.biquad(RBJParams{FilterPassType::BANDPASS, 6290, sampleRate, 4.0, 0})
			.mul(0.3)
			.AR(100, 100, Pow(5), Pow(5))
			.build());

		add(MIDIFile::DrumSet::JINGLE_BELL,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 2568, sampleRate, 10, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 2697, sampleRate, 10, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 4948, sampleRate, 10, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 5409, sampleRate, 10, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 6114, sampleRate, 10, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 7043, sampleRate, 10, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 9172, sampleRate, 15, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 10869, sampleRate, 10, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 11944, sampleRate, 10, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 14559, sampleRate, 20, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 16772, sampleRate, 10, 30})
					 .biquad(RBJParams{FilterPassType::HIGHPASS, 2000, sampleRate, 1, 0})
					 .build())
			.mul(0.009)
			.AR(10, 1000, Pow(5), Pow(3))
			.build());

		add(MIDIFile::DrumSet::BELL_TREE,
			AmpBuilder()
			.src(ConstFreq(mksp<SquareWave>(), 5058))
			.AR(5, 3000, 3000, Pow(5), Pow(2))
			.add(AmpBuilder().src(ConstFreq(mksp<SquareWave>(), 4736))
				 .DAHDSR(50, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(ConstFreq(mksp<SquareWave>(), 4092))
				 .DAHDSR(100, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(ConstFreq(mksp<SquareWave>(), 3535))
				 .DAHDSR(150, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(ConstFreq(mksp<SquareWave>(), 3032))
				 .DAHDSR(200, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(ConstFreq(mksp<SquareWave>(), 2717))
				 .DAHDSR(250, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(ConstFreq(mksp<SquareWave>(), 2435))
				 .DAHDSR(300, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(ConstFreq(mksp<SquareWave>(), 2182))
				 .DAHDSR(350, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(ConstFreq(mksp<SquareWave>(), 1970))
				 .DAHDSR(400, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(ConstFreq(mksp<SquareWave>(), 1765))
				 .DAHDSR(480, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(ConstFreq(mksp<SquareWave>(), 1617))
				 .DAHDSR(560, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(ConstFreq(mksp<SquareWave>(), 1470))
				 .DAHDSR(640, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(ConstFreq(mksp<SquareWave>(), 1289))
				 .DAHDSR(720, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(ConstFreq(mksp<SquareWave>(), 1139))
				 .DAHDSR(800, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.add(AmpBuilder().src(ConstFreq(mksp<SquareWave>(), 1043))
				 .DAHDSR(880, 5, 1, 1, 1, false, 3000, 3000, Pow(5), Pow(5), Pow(2))
				 .build())
			.mul(0.1)
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 10000, sampleRate, 4, 20})
					 .biquad(RBJParams{FilterPassType::BELL, 6581, sampleRate, 4, 20})
					 .biquad(RBJParams{FilterPassType::HIGHPASS, 6000, sampleRate, 0.5, 0})
					 .build())
			.AR(5, 3000, 3000, Pow(5), Pow(2))
			.build());
		add(MIDIFile::DrumSet::CASTANETS,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 1618, sampleRate, 5, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 4522, sampleRate, 10, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 7110, sampleRate, 15, 30})
					 .biquad(RBJParams{FilterPassType::BELL, 11287, sampleRate, 15, 30})
					 .biquad(RBJParams{FilterPassType::HIGHPASS, 1000, sampleRate, 1.0, 0})
					 .build())
			.mul(0.085)
			.AR(3, 500, Pow(5), Pow(20))
			.build());
		add(MIDIFile::DrumSet::SURDO_MUTE,
			AmpBuilder()
			.src(mksp<NoiseSrc>())
			.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
					 .biquad(RBJParams{FilterPassType::BELL, 153, sampleRate, 10, 5})
					 .biquad(RBJParams{FilterPassType::BELL, 179, sampleRate, 10, 5})
					 .biquad(RBJParams{FilterPassType::BELL, 215, sampleRate, 10, 5})
					 .biquad(RBJParams{FilterPassType::BELL, 280, sampleRate, 10, 5})
					 .biquad(RBJParams{FilterPassType::BELL, 303, sampleRate, 10, 5})
					 .biquad(RBJParams{FilterPassType::BELL, 343, sampleRate, 10, 5})
					 .biquad(RBJParams{FilterPassType::BELL, 400, sampleRate, 5, 5})
					 .biquad(RBJParams{FilterPassType::BELL, 498, sampleRate, 5, 5})
					 .biquad(RBJParams{FilterPassType::BELL, 649, sampleRate, 3, 5})
					 .biquad(RBJParams{FilterPassType::BELL, 838, sampleRate, 10, 5})
					 .biquad(RBJParams{FilterPassType::BELL, 1000, sampleRate, 10, 5})
					 .biquad(RBJParams{FilterPassType::BANDPASS, 180, sampleRate, 2, 0})
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
						  .biquad(RBJParams{FilterPassType::BELL, 153, sampleRate, 10, 5})
						  .biquad(RBJParams{FilterPassType::BELL, 179, sampleRate, 10, 5})
						  .biquad(RBJParams{FilterPassType::BELL, 215, sampleRate, 10, 5})
						  .biquad(RBJParams{FilterPassType::BELL, 280, sampleRate, 10, 5})
						  .biquad(RBJParams{FilterPassType::BELL, 303, sampleRate, 10, 5})
						  .biquad(RBJParams{FilterPassType::BELL, 343, sampleRate, 10, 5})
						  .biquad(RBJParams{FilterPassType::BELL, 400, sampleRate, 5, 5})
						  .biquad(RBJParams{FilterPassType::BELL, 498, sampleRate, 5, 5})
						  .biquad(RBJParams{FilterPassType::BELL, 649, sampleRate, 3, 5})
						  .biquad(RBJParams{FilterPassType::BELL, 838, sampleRate, 10, 5})
						  .biquad(RBJParams{FilterPassType::BELL, 1000, sampleRate, 10, 5})
						  .biquad(RBJParams{FilterPassType::BANDPASS, 180, sampleRate, 2, 0})
						  .build())
				 .AR(5, 1000, Pow(5), Pow(20))
				 .build())
			.mul(1.5)
			.AR(5, 1000, Pow(5), Pow(5))
			.build());

		//以下非GM标准音色映射

		add(22, AmpBuilder()
			.src(ConstFreq(mksp<TriWave>(), 1066))
			.AR(10, 100, Pow(-5), Pow(5))
			.build());
		add(23, AmpBuilder()
			.src(ConstFreq(mksp<TriWave>(), 2135))
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
								 .biquad(RBJParams{FilterPassType::BELL, 740, sampleRate, 0.5, 5})
								 .biquad(RBJParams{FilterPassType::BELL, 1337, sampleRate, 0.5, 5})
								 .biquad(RBJParams{FilterPassType::BELL, 5000, sampleRate, 0.5, 10})
								 .biquad(RBJParams{FilterPassType::BELL, 1254, sampleRate, 10, 7})
								 .biquad(RBJParams{FilterPassType::BELL, 1554, sampleRate, 10, 7})
								 .biquad(RBJParams{FilterPassType::HIGHPASS, 500, sampleRate, 0.5, 0})
								 .build())
						.build(), 0.2)
				.MultiStageEnv(releaseWithCompressorEffect(2, 10, 10, 500, 0.7))
				.mul(0.8)
				.build());
		}

		NonInterpolateAmpSet::init(cfg);
	}
}