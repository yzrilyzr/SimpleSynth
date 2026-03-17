/**
 * 默认简单音色，开箱即用
 * 不喜欢的话，可以直接删掉，没有任何影响(～￣▽￣)～
 */

#include "SimpleDrumSet.h"
#include "SimpleMIDIInstrument.h"
#include "dsp/Chorus.h"
#include "dsp/DSPGroupBuilder.h"
#include "dsp/Delayer.h"
#include "synth/composed/NearestAmpSet.h"
#include "synth/composed/NoteVolBalance.h"
#include "synth/composed/Matrix6x6Modulation.h"
#include "synth/envelopers/EnvUtil.h"
#include "synth/envelopers/TimeEnvelop.h"
#include "synth/filters/BiquadEnvFilterGroup.h"
#include "synth/generators/drum/SimpleDrumAmp.h"
#include "synth/generators/noise/NoiseSrc.h"
#include "synth/generators/physic/BowedString.h"
#include "synth/generators/physic/KarplusStrongSrc.h"
#include "synth/generators/physic/PianoSrc.h"
#include "synth/generators/physic/TwoStringResonator.h"
#include "synth/generators/physic/Sitar.h"
#include "synth/generators/pulse/CymbalOsc.h"
#include "synth/generators/pulse/Pulse.h"
#include "synth/generators/pulse/SawWave.h"
#include "synth/generators/pulse/SquareWave.h"
#include "synth/generators/pulse/TriWave.h"
#include "synth/generators/sine/AHarmonicWave.h"
#include "synth/generators/sine/SineBasePowHarmonicWave.h"
#include "synth/generators/sine/SineWave.h"
#include "synth/source/AmpBuilder.h"
#include "synth/source/AmplitudeSources.h"

using namespace yzrilyzr_util;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_array;

namespace yzrilyzr_simplesynth{
	u_sp<PianoSrc> acoustic_grand_piano(){
		u_sp<PianoSrc> sp=mksp<PianoSrc>();
		auto & sb=sp->soundboardParameters;
		sb.eq1=200;
		sb.eq2=500;
		sb.eq3=3000;
		sb.eq4=5000;
		sb.eq5=8000;
		sb.c1=9;
		sb.c3=30;
		GraphInterpolator vol(DoubleArray({//
			36, 1,//
			48, 1,//
			60, 1,//
			66, 2,//
			72, 3,//
			78, 4,//
			83, 4,//
			96, 18,//
			120, 40//
										  }));
		GraphInterpolator pos(DoubleArray({//
			36, 0.4,//
			48, 0.4,//
			60, 0.3,//
			72, 0.2,//
			84, 0.1,//
			120, 0.05//
										  }));
		GraphInterpolator weight(DoubleArray({//
			36, 8,//
			42, 5,//
			48, 4,//
			54, 6,//
			60, 5,//
			72, 3,//
			78, 3,//
			84, 3,//
			90, 2,//
			120, 2//
											 }));
		GraphInterpolator detune(DoubleArray({//
			0, 0.00015,  // 低音区更小的detune
			36, 0.00015, // 低音区
			48, 0.00020, // 低音到中音过渡
			60, 0.00040, // 中音区
			72, 0.00080, // 中音到高音过渡
			84, 0.00120, // 高音区
			96, 0.00180, // 高音区
			108, 0.00200, // 极高音区
			128, 0.00250  // 极高音区
											 }));
		for(u_index i=0;i < CHANNEL_MAX_NOTE_ID;i++){
			auto & kp=sp->keyParams[i];
			kp.noteID=i;
			kp.minr=0.34;
			kp.maxr=2;
			kp.minL=0.07;
			kp.maxL=1.39;
			kp.hammer_type=2;
			kp.weight=weight.y(i) * 3;
			kp.ampLl=-4;
			kp.ampLr=4;
			kp.amprl=4;
			kp.amprr=8;
			kp.mult_density_string=1;
			kp.mult_modulus_string=1;
			kp.mult_impedance_bridge=1;
			kp.mult_impedance_hammer=0;
			kp.mult_mass_hammer=1;
			kp.mult_force_hammer=1;
			kp.mult_hysteresis_hammer=1;
			kp.mult_stiffness_exponent_hammer=1;
			kp.position_hammer=pos.y(i);
			kp.mult_loss_filter=1;
			kp.detune=detune.y(i);
			kp.mult_radius_core_string=1;
			kp.outputVolume=vol.y(i) * 15;
		}
		return sp;
	}
	u_sp<PianoSrc> bright_acoustic_piano(){
		u_sp<PianoSrc> sp=mksp<PianoSrc>();
		auto & sb=sp->soundboardParameters;
		sb.eq1=200;
		sb.eq2=500;
		sb.eq3=3000;
		sb.eq4=5000;
		sb.eq5=8000;
		sb.c1=9;
		sb.c3=30;
		GraphInterpolator vol(DoubleArray({//
			36, 1,//
			48, 1,//
			60, 1,//
			66, 2,//
			72, 3,//
			78, 4,//
			83, 4,//
			96, 18,//
			120, 40//
										  }));
		GraphInterpolator pos(DoubleArray({//
			36, 0.4,//
			48, 0.4,//
			60, 0.3,//
			72, 0.2,//
			84, 0.1,//
			120, 0.05//
										  }));
		GraphInterpolator weight(DoubleArray({//
			36, 8,//
			42, 5,//
			48, 4,//
			54, 6,//
			60, 5,//
			72, 3,//
			78, 3,//
			84, 3,//
			90, 2,//
			120, 2//
											 }));
		GraphInterpolator detune(DoubleArray({//
			0, 0.00015,  // 低音区更小的detune
			36, 0.00015, // 低音区
			48, 0.00020, // 低音到中音过渡
			60, 0.00040, // 中音区
			72, 0.00080, // 中音到高音过渡
			84, 0.00120, // 高音区
			96, 0.00180, // 高音区
			108, 0.00200, // 极高音区
			128, 0.00250  // 极高音区
											 }));
		for(u_index i=0;i < CHANNEL_MAX_NOTE_ID;i++){
			auto & kp=sp->keyParams[i];
			kp.noteID=i;
			kp.minr=0.34;
			kp.maxr=2;
			kp.minL=0.07;
			kp.maxL=1.39;
			kp.hammer_type=2;
			kp.weight=weight.y(i) * 4;
			kp.ampLl=-4;
			kp.ampLr=4;
			kp.amprl=4;
			kp.amprr=8;
			kp.mult_density_string=1;
			kp.mult_modulus_string=1;
			kp.mult_impedance_bridge=1;
			kp.mult_impedance_hammer=0;
			kp.mult_mass_hammer=1.1;
			kp.mult_force_hammer=1.1;
			kp.mult_hysteresis_hammer=1;
			kp.mult_stiffness_exponent_hammer=1.1;
			kp.position_hammer=pos.y(i);
			kp.mult_loss_filter=1;
			kp.detune=detune.y(i);
			kp.mult_radius_core_string=1;
			kp.outputVolume=vol.y(i) * 15;
		}
		return sp;
	}
	u_sp<PianoSrc> honky_tonk_piano(){
		u_sp<PianoSrc> sp=mksp<PianoSrc>();
		auto & sb=sp->soundboardParameters;
		sb.eq1=200;
		sb.eq2=500;
		sb.eq3=3000;
		sb.eq4=5000;
		sb.eq5=8000;
		sb.c1=9;
		sb.c3=30;
		GraphInterpolator vol(DoubleArray({//
			36, 1,//
			48, 1,//
			60, 1,//
			66, 2,//
			72, 3,//
			78, 4,//
			83, 4,//
			96, 18,//
			120, 40//
										  }));
		GraphInterpolator pos(DoubleArray({//
			36, 0.4,//
			48, 0.4,//
			60, 0.3,//
			72, 0.2,//
			84, 0.1,//
			120, 0.05//
										  }));
		GraphInterpolator weight(DoubleArray({//
			36, 8,//
			42, 5,//
			48, 4,//
			54, 6,//
			60, 5,//
			72, 3,//
			78, 3,//
			84, 3,//
			90, 2,//
			120, 2//
											 }));
		GraphInterpolator detune(DoubleArray({//
			0, 0.003,  // 低音区更小的detune
			36, 0.003, // 低音区
			48, 0.0040, // 低音到中音过渡
			60, 0.0050, // 中音区
			72, 0.0080, // 中音到高音过渡
			84, 0.0120, // 高音区
			96, 0.0180, // 高音区
			108, 0.0200, // 极高音区
			128, 0.0250  // 极高音区
											 }));
		for(u_index i=0;i < CHANNEL_MAX_NOTE_ID;i++){
			auto & kp=sp->keyParams[i];
			kp.noteID=i;
			kp.minr=0.34;
			kp.maxr=2;
			kp.minL=0.07;
			kp.maxL=1.39;
			kp.hammer_type=2;
			kp.weight=weight.y(i) * 3;
			kp.ampLl=-4;
			kp.ampLr=4;
			kp.amprl=4;
			kp.amprr=8;
			kp.mult_density_string=1;
			kp.mult_modulus_string=1;
			kp.mult_impedance_bridge=1;
			kp.mult_impedance_hammer=0;
			kp.mult_mass_hammer=1;
			kp.mult_force_hammer=1;
			kp.mult_hysteresis_hammer=1;
			kp.mult_stiffness_exponent_hammer=1;
			kp.position_hammer=pos.y(i);
			kp.mult_loss_filter=1;
			kp.detune=detune.y(i);
			kp.mult_radius_core_string=1;
			kp.outputVolume=vol.y(i) * 15;
		}
		return sp;
	}
	NoteProcPtr SimpleMIDIInstrument::get(s_bank_id bank, s_program_id program, u_sample_rate sampleRate){
		switch(program){
			case MIDIFile::Instruments::PIANO_ACOUSTIC_GRAND_PIANO:
				return acoustic_grand_piano();
			case MIDIFile::Instruments::PIANO_BRIGHT_ACOUSTIC_PIANO:
				return bright_acoustic_piano();
				/*return TwoStringResonatorBuilder()
					.exciter(0.676, 1)
					.exciterHiCut(0.839, 0.996)
					.exciterHiCutEnv(0, 0, 298.611, 0)
					.exciterLowCut(0.033, 1.184)
					.string1(1, 0.817, 0.675)
					.string2(2, -0.787, 0.772)
					.stringMix(-0.323)
					.stringEnv(27.875, 0.1, 5000, false, 1, 145.919)
					.stringLevel(0.442)
					.comb(0.497, 0.366, -0.396, 0.124)
					.resonatorLevel(1)
					.build();*/
			case MIDIFile::Instruments::PIANO_ELECTRIC_GRAND_PIANO:
				return SynthUtil::getDefault();
			case MIDIFile::Instruments::PIANO_HONKY_TONK_PIANO:
				return honky_tonk_piano();
			case MIDIFile::Instruments::PIANO_ELECTRIC_PIANO_1:
				return
					AmpBuilder()
					.src(AmpBuilder(mksp<SineWave>())
						 .pm(AmpBuilder(mksp<SineWave>())
							 .freqSrc(MulPhase(NotePhase, 24))
							 .ADSR(0, 700, 0, false, 1000, Line(), Pow(5), Pow(2))
							 .build(), 0.15)
						 .ADSR(10, 1500, 0.5, false, 1000, Pow(5), Pow(5), Pow(2))
						 .build())
					.add(AmpBuilder(mksp<SineWave>())
						 .pm(AmpBuilder()
							 .src(AmpBuilder(mksp<SineWave>())
								  .freqSrc(NotePhase)
								  .ADSR(0, 700, 0.43, true, 1000, Line(), Pow(5), Pow(2))
								  .mul(0.22).build())
							 .add(AmpBuilder(mksp<SineWave>())
								  .freqSrc(NotePhase)
								  .ADSR(0, 300, 1, true, 1000, Line(), Pow(5), Pow(2))
								  .mul(-0.62).build())
							 .build(), 1)
						 .ADSR(0, 1000, 0.25, true, 1000, Line(), Pow(5), Pow(2))
						 .build())
					.ADSR(10, 8000, 0.5, false, 500, Pow(5), Pow(5), Pow(2))
					.noteDSP(mksp<Chorus>(1.0, 2.0, 1.0, 0.0))
					.noteDSP(mksp<Delayer>(220, -0.8, 0.05))
					.build();
			case MIDIFile::Instruments::PIANO_ELECTRIC_PIANO_2:
				return AmpBuilder(Matrix6x6ModulationBuilder()//alg 5
								  .setOp(0, AmpBuilder(SineW()).ADSR(1, 3000, 0, false, 100, Pow(-5), Pow(2), Pow(2)).build(), 1.0, 0.8, 0.0, 1.0, 0.99)
								  .setOp(1, AmpBuilder(SineW()).ADSR(1, 300, 0, false, 100, Pow(-5), Pow(4), Pow(2)).build(), 14.0, 0.0, 0.0, 1.0, 0.0)
								  .setOp(2, AmpBuilder(SineW()).ADSR(1, 7500, 0, false, 100, Pow(-5), Pow(2), Pow(2)).build(), 1.0, 0.0, 0.0, 1.0, 0.99)
								  .setOp(3, AmpBuilder(SineW()).velMix(0.143).ADSR(1, 5000, 0, false, 100, Pow(-5), Pow(2), Pow(2)).build(), 1.0, 0.0, 0.0, 1.0, 0.0)
								  .setOp(4, AmpBuilder(SineW()).ADSR(1, 7000, 0, false, 100, Pow(-5), Pow(5), Pow(5)).build(), 1.0, -1.0, 0.0, 1.0, 0.99)
								  .setOp(5, AmpBuilder(SineW()).velMix(0.143).ADSR(1, 3500, 0, false, 100, Pow(-5), Pow(5), Pow(5)).build(), 1.0, 1.0, 0.0, 1.0, 0.0)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 1, 0, 0.58 * 6)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 3, 2, 0.89 * 6)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 5, 4, 0.79 * 6)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 5, 5, 0.006)
								  .build())
					.mul(0.3)
					.ADSR(1, 9000, 0, false, 500, Pow(-5), Pow(3), Pow(1))
					.noteDSP(mksp<Chorus>(1.0, 2.0, 1.0, 0.0))
					.noteDSP(mksp<Delayer>(220, -0.8, 0.05))
					.build();
			case MIDIFile::Instruments::PIANO_HARPSICHORD:
				return AmpBuilder()
					.ks(0.85)
					.biquad(sampleRate, BELL, 200.0, 0.5, 15.0)
					.biquad(sampleRate, BELL, 1000.0, 0.5, -3.0)
					.biquad(sampleRate, HIGHSHELF, 2000.0, 1, -3.0)
					.biquadEnvVel(70, 127, 1, LOWPASS)
					.ADSR(5, 5000, 0, false, 100, Pow(-5), Pow(8), Pow(15)).build();
			case MIDIFile::Instruments::PIANO_CLAVICHORD:
				return AmpBuilder()
					.ks(0.95)
					.biquad(sampleRate, BELL, 300.0, 0.5, 15.0)
					.biquad(sampleRate, BELL, 1000.0, 0.5, -3.0)
					.biquadEnvVel(90, 127, 1, LOWPASS)
					.mul(0.5)
					.ADSR(5, 5000, 0, false, 100, Pow(-5), Pow(8), Pow(15)).build();

			//=====================================

			case MIDIFile::Instruments::CHROMATIC_PERCUSSION_CELESTA:
				return AmpBuilder().src(mksp<SineWave>())
					.pm(mksp<SineWave>(), 0.3, 7)
					.ks(0.9)
					.ADSR(10, 5000, 0, false, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::CHROMATIC_PERCUSSION_GLOCKENSPIEL:
				return AmpBuilder(Matrix6x6ModulationBuilder()
								  .setOp(0, AmpBuilder(SineW()).ADSR(1, 8281, 0, false, 100, Pow(-5), Pow(5), Pow(5)).build(), 0.9998, 0.0, 0.0, 1.0, 0.84)
								  .setOp(1, AmpBuilder(SineW()).ADSR(1, 4450, 0, false, 100, Pow(-5), Pow(5), Pow(5)).build(), 6.0, 0.0, 0.0, 1.0, 0.52)
								  .setOp(2, AmpBuilder(SineW()).ADSR(21, 1600, 0, false, 100, Pow(-5), Pow(5), Pow(5)).build(), 3.0007, 0.0, 0.0, 1.0, 0.63)
								  .setOp(3, AmpBuilder(SineW()).ADSR(70, 1200, 0, false, 100, Pow(-5), Pow(5), Pow(5)).build(), 12.9989, 0.0, 0.0, 1.0, 0)
								  .setOp(4, AmpBuilder(SineW()).ADSR(96, 2000, 0, false, 100, Pow(-5), Pow(5), Pow(5)).build(), 4.0, 0.0, 0.0, 1.0, 0.73)
								  .setOp(5, AmpBuilder(SineW()).ADSR(65, 1400, 0, false, 100, Pow(-5), Pow(5), Pow(5)).build(), 12.0035, 0.0, 0.0, 1.0, 0)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 3, 2, 3.2)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 5, 4, 3.2)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 5, 5, 0.006)
								  .build())
					.mul(0.5)
					.ADSR(1, 9000, 0, false, 100, Pow(-5), Pow(5), Pow(1))
					.build();
			case MIDIFile::Instruments::CHROMATIC_PERCUSSION_MUSIC_BOX:
				return AmpBuilder().src(mksp<SineWave>())
					.pm(mksp<SineWave>(), 0.3, 6)
					.addMul(mksp<NoiseSrc>(), 0.1)
					.ks(0.9)
					.ADSR(10, 5000, 0, false, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::CHROMATIC_PERCUSSION_VIBRAPHONE:
				return AmpBuilder().src(mksp<SineWave>())
					.pm(mksp<SineWave>(), 0.3, 5)
					.addMul(mksp<NoiseSrc>(), 0.1)
					.ks(0.8)
					.ADSR(10, 5000, 0, false, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::CHROMATIC_PERCUSSION_MARIMBA:
				return AmpBuilder().src(mksp<SineWave>())
					.pm(mksp<SineWave>(), 0.2, 5)
					.ks(0.2)
					.ADSR(10, 2000, 0, false, 100, Pow(-5), Pow(7), Pow(5))
					.build();
			case MIDIFile::Instruments::CHROMATIC_PERCUSSION_XYLOPHONE:
				return AmpBuilder().src(mksp<SineWave>())
					.pm(mksp<SineWave>(), 0.2, 5)
					.ks(0.15)
					.ADSR(10, 2000, 0, false, 100, Pow(-5), Pow(7), Pow(5))
					.build();
			case MIDIFile::Instruments::CHROMATIC_PERCUSSION_TUBULAR_BELLS:
				return AmpBuilder(Matrix6x6ModulationBuilder()//alg 5
								  .setOp(0, AmpBuilder(SineW()).ADSR(1, 2000, 0, false, 100, Pow(-5), Pow(2), Pow(2)).build(), 1.0, 0.8, 0.0, 1.0, 0.95)
								  .setOp(1, AmpBuilder(SineW()).ADSR(1, 8000, 0, false, 100, Pow(-5), Pow(2), Pow(2)).build(), 3.5, 1.2, 0.0, 1.0, 0.0)
								  .setOp(2, AmpBuilder(SineW()).ADSR(1, 2000, 0, false, 100, Pow(-5), Pow(2), Pow(2)).build(), 1.0, -2.0, 0.0, 1.0, 0.99)
								  .setOp(3, AmpBuilder(SineW()).ADSR(1, 8000, 0, false, 100, Pow(-5), Pow(2), Pow(2)).build(), 3.5, -0.8, 0.0, 1.0, 0.0)
								  .setOp(4, AmpBuilder(SineW()).ADSR(1, 100, 0, false, 100, Pow(-5), Pow(5), Pow(5)).build(), 0, 323.594, 0.0, 1.0, 0.99)
								  .setOp(5, AmpBuilder(SineW()).ADSR(1, 100, 0, false, 100, Pow(-5), Pow(5), Pow(5)).build(), 2.0, -2.8, 0.0, 1.0, 0.0)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 1, 0, 0.78 * 2)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 3, 2, 0.75 * 2)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 5, 4, 0.85 * 2)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 5, 5, 0.01)
								  .build())
					.mul(0.5)
					.ADSR(1, 9000, 0, false, 300, Pow(-5), Pow(3), Pow(1))
					.build();
			case MIDIFile::Instruments::CHROMATIC_PERCUSSION_DULCIMER:
				return AmpBuilder().ks(0.9).ADSR(10, 5000, 0, false, 100, Pow(-5), Pow(5), Pow(5)).build();

			//=====================================

			case MIDIFile::Instruments::ORGAN_HAMMOND_ORGAN:
				return AmpBuilder()
					.src(mksp<SineBasePowHarmonicWave>(DoubleArray({
					0.9, 0.9, 1.0, 0.7, 0.6
																   })))
					.detune(2, 0.2)
					.biquad(sampleRate, HIGHSHELF, 2000.0, 0.5, -12.0)
					.mul(0.4)
					.ADSR(10, 1, 0.7, true, 200, Pow(-5), Pow(5), Pow(5)).build();
			case MIDIFile::Instruments::ORGAN_PERCUSSIVE_ORGAN:
				return TwoStringResonatorBuilder()
					.exciter(0.833, 1)
					.exciterHiCut(0.829, 4.861)
					.exciterHiCutEnv(0.1, 0.1, 215.356, 0.541667)
					.exciterLowCut(0.314, 1)
					.string1(1, 1, 0.5)
					.string2(2, 1, 0.591)
					.stringMix(0)
					.stringEnv(100, 100, 100, true, 0.541667, 100)
					.stringLevel(0.141)
					.comb(1, 1, 1, 0.155)
					.resonator(0, 648, 0)
					.resonatorLevel(0.397)
					.build();
			case MIDIFile::Instruments::ORGAN_ROCK_ORGAN:
				return TwoStringResonatorBuilder()
					.exciter(0.833, 1)
					.exciterHiCut(0.8, 4.861)
					.exciterHiCutEnv(0.1, 0.1, 215.356, 0.618056)
					.exciterLowCut(0.257, 1)
					.string1(1, 1, 0.5)
					.string2(0.5, -1, 0.805)
					.stringMix(-0.622)
					.stringEnv(100, 100, 100, true, 0.541667, 100)
					.stringLevel(0.141)
					.comb(0.5, 1, 1, 0.073)
					.resonator(0, 648, 0)
					.resonatorLevel(0.397)
					.build();
			case MIDIFile::Instruments::ORGAN_CHURCH_ORGAN:
				return TwoStringResonatorBuilder()
					.exciter(0.052, 1)
					.exciterHiCut(0.919, 2.41)
					.exciterHiCutEnv(0.1, 0.1, 215.356, 0.493056)
					.exciterLowCut(0.262, 0.666)
					.string1(1, 1, 0.933)
					.string2(2, 1, 0.933)
					.stringMix(-0.006)
					.stringEnv(132.031, 27.875, 27.875, true, 0.911111, 100)
					.stringLevel(1)
					.comb(0.25, 1, 1, 0.699)
					.resonator(0, 648, 0)
					.resonator(1, 1446.1, 0)
					.resonatorLevel(0.358)
					.build();
			case MIDIFile::Instruments::ORGAN_REED_ORGAN:
				return TwoStringResonatorBuilder()
					.exciter(0.376, 1)
					.exciterHiCut(0.828, 2.269)
					.exciterHiCutEnv(0.1, 0.1, 27.875, 0)
					.exciterLowCut(0.248, 0.666)
					.string1(1, 1, 1)
					.string2(2, 1, 1)
					.stringMix(-0.5)
					.stringEnv(201.469, 27.875, 27.875, true, 0.611111, 100)
					.stringLevel(0.954)
					.comb(1, 1, 1, 0)
					.resonatorLevel(0.358)
					.build();
			case MIDIFile::Instruments::ORGAN_ACCORDIAN:
				return AmpBuilder().src(mksp<SquareWave>())
					.pm(mksp<SineWave>(), 0.65, 1)
					.IIR(sampleRate, 1, 10.1, 9346.7)
					.mul(0.4)
					.biquadEnvVel(70, 127, 1, LOWPASS)
					.ADSR(37, 2000, 0.71, true, 100, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::ORGAN_HARMONICA:
				return TwoStringResonatorBuilder()
					.exciter(0.095, 1)
					.exciterHiCut(0.81, 1.844)
					.exciterHiCutEnv(13.9875, 62.5938, 55.65, 0.270833)
					.exciterLowCut(0.248, 0.666)
					.string1(1, 1, 1)
					.string2(2, 1, 1)
					.stringMix(-0.5)
					.stringEnv(277.85, 27.875, 27.875, true, 0.694444, 100)
					.stringLevel(0.598)
					.comb(0.5, 1, 1, 0.848)
					.resonator(0, 648, 0)
					.resonator(1, 3226.8, 0)
					.resonator(2, 4647.6, 0)
					.resonatorLevel(0.358)
					.build();
			case MIDIFile::Instruments::ORGAN_TANGO_ACCORDIAN:
				return AmpBuilder().src(mksp<SquareWave>())
					.pm(mksp<SineWave>(), 0.65, 1)
					.IIR(sampleRate, 1, 10.1, 9346.7)
					.mul(0.4)
					.biquadEnvVel(70, 127, 1, LOWPASS)
					.ADSR(37, 2000, 0.71, true, 100, Pow(5), Pow(5), Pow(5))
					.build();

			//=====================================

			case MIDIFile::Instruments::GUITAR_ACOUSTIC_GUITAR_NYLON:
				return AmpBuilder()
					.src(mksp<SquareWave>())
					.add(mksp<NoiseSrc>())
					.mul(0.5)
					.ks(0.5)
					.ADSR(10, 5000, 0.1, false, 100, Pow(5), Pow(5), Pow(5))
					.biquad(sampleRate, HIGHPASS, 10, 0.7, 0)
					.biquadEnvVel(70, 127, 1, LOWPASS)
					.clamp(1.2, 1)
					.build();
			case MIDIFile::Instruments::GUITAR_ACOUSTIC_GUITAR_STEEL:
				return AmpBuilder(
					KarplusStrongBuilder()
					.burst(AmpBuilder(mksp<SquareWave>()).add(mksp<NoiseSrc>()).mul(0.5).build())
					.alpha(mksp<GraphInterpolator>(DoubleArray({0, 0.5, 55, 0.5, 61, 0.7, 62, 0.95, 128, 0.95})))
					.build()
				)
					.ADSR(10, 5000, 0.1, false, 100, Pow(5), Pow(5), Pow(5))
					.biquad(sampleRate, HIGHPASS, 10, 0.7, 0)
					.biquadEnvVel(70, 127, 1, LOWPASS)
					.clamp(1.2, 1)
					.build();
			case MIDIFile::Instruments::GUITAR_ELECTRIC_GUITAR_JAZZ:
				return AmpBuilder().src(AmpBuilder().src(mksp<Pulse>(0.1, 0.3, 0.1, 0)).add(mksp<NoiseSrc>()).mul(0.5).build())
					.ks(0.5)
					.ADSR(10, 5000, 0.1, false, 100, Pow(5), Pow(5), Pow(5))
					.biquad(sampleRate, HIGHPASS, 10, 0.7, 0)
					.biquadEnvVel(70, 127, 1, LOWPASS)
					.clamp(1.2, 1)
					.build();
			case MIDIFile::Instruments::GUITAR_ELECTRIC_GUITAR_CLEAN:
				return AmpBuilder().src(AmpBuilder().src(mksp<SquareWave>()).add(mksp<NoiseSrc>()).mul(0.5).build())
					.ks(0.75)
					.ADSR(10, 5000, 0.1, false, 100, Pow(5), Pow(5), Pow(5))
					.biquad(sampleRate, HIGHPASS, 10, 0.7, 0)
					.biquadEnvVel(70, 127, 1, LOWPASS)
					.clamp(1.2, 1)
					.build();
			case MIDIFile::Instruments::GUITAR_ELECTRIC_GUITAR_MUTED:
				return AmpBuilder().src(AmpBuilder().src(mksp<SquareWave>()).add(mksp<NoiseSrc>()).mul(0.5).build())
					.ks(0.5)
					.ADSR(10, 400, 0, false, 100, Pow(5), Pow(5), Pow(5))
					.biquad(sampleRate, HIGHPASS, 10, 0.7, 0)
					.biquadEnvVel(70, 127, 1, LOWPASS)
					.clamp(1.2, 1)
					.build();
			case MIDIFile::Instruments::GUITAR_OVERDRIVEN_GUITAR:
				return AmpBuilder()
					.ks(0.85)
					.arctanDistortion(1, 3, 1)
					.biquad(sampleRate, FilterPassType::BELL, 200, 0.6, 4)
					.biquad(sampleRate, FilterPassType::LOWPASS, 4000, 1, 1)
					.clampV(0.9, 0.9)
					.biquadEnvVel(100, 127, 1, LOWPASS)
					.ADSR(10, 50, 1, true, 100, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::GUITAR_DISTORTION_GUITAR:
				return AmpBuilder()
					.ks(0.85)
					.biquad(sampleRate, FilterPassType::BELL, 200, 0.6, 5)
					.clampV(100, 0.9)
					.arctanDistortion(1, 25, 1)
					.clampV(0.8, 0.8)
					.noteDSP(DSPGroupBuilder().begin(DSPGroupBuilder::TYPE_CHAIN)
							 .biquad(sampleRate, FilterPassType::BELL, 500, 0.6, 4)
							 .biquad(sampleRate, FilterPassType::LOWPASS, 3000, 0.5, 0)
							 .biquad(sampleRate, FilterPassType::HIGHPASS, 10, 1, 0)
							 .build())
					.mul(0.8)
					.biquadEnvVel(100, 127, 1, LOWPASS)
					.ADSR(10, 50, 1, true, 100, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::GUITAR_GUITAR_HARMONICS:
				return TwoStringResonatorBuilder()
					.exciter(0.619, 1)
					.exciterHiCut(1, 1)
					.exciterHiCutEnv(100, 100, 100, 0.784722)
					.exciterLowCut(0.01, 1)
					.string1(1, 1, 1)
					.string2(1, 1, 1)
					.stringMix(0)
					.stringEnv(100, 100, 100, true, 0.5, 100)
					.stringLevel(0.125)
					.comb(1, 1, -1, 1)
					.resonatorLevel(1)
					.build();

			//=====================================

			case MIDIFile::Instruments::BASS_ACOUSTIC_BASS:
				return AmpBuilder()
					.src(mksp<SquareWave>())
					.ks(0.3)
					.mul(1.2)
					.ADSR(5, 1000, 0.3, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::BASS_ELECTRIC_BASS_FINGER:
				return TwoStringResonatorBuilder()
					.exciter(0.105, 1)
					.exciterHiCut(0.866, 1)
					.exciterHiCutEnv(0.1, 0.1, 41.7625, 0.125)
					.exciterLowCut(0.01, 1)
					.string1(1, 1, 0.579)
					.string2(1, 1, 0.576)
					.stringMix(0)
					.stringEnv(41.7625, 0.1, 1000, true, 0.5, 141.662)
					.stringLevel(1.0)
					.comb(0.5, 0.274, 0.262, 0.161)
					.resonator(0, 1889.6, 0)
					.resonatorLevel(0.54)
					.build();
			case MIDIFile::Instruments::BASS_ELECTRIC_BASS_PICK:
				return TwoStringResonatorBuilder()
					.exciter(0.105, 1)
					.exciterHiCut(0.866, 1)
					.exciterHiCutEnv(0.1, 0.1, 41.7625, 0.125)
					.exciterLowCut(0.01, 1)
					.string1(1, 1, 0.305)
					.string2(1, 1, 0.323)
					.stringMix(0)
					.stringEnv(41.7625, 0.1, 1000, true, 0.5, 141.662)
					.stringLevel(0.5)
					.comb(0.5, 0.274, 0.262, 0.161)
					.resonator(0, 1889.6, 0)
					.resonatorLevel(0.54)
					.build();
			case MIDIFile::Instruments::BASS_FRETLESS_BASS:
				return AmpBuilder()
					.src(mksp<SquareWave>())
					.ks(0.3)
					.mul(1.2)
					.ADSR(5, 1000, 0.3, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::BASS_SLAP_BASS_1:
			case MIDIFile::Instruments::BASS_SLAP_BASS_2:
				return AmpBuilder().src(mksp<Pulse>(0.3, 0.4, 0.3, 0))
					.ks(0.9)
					.biquad(sampleRate, HIGHPASS, 10, 0.7, 0)
					.mul(1.2)
					.ADSR(10, 1000, 0.3, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::BASS_SYNTH_BASS_1:
				return AmpBuilder().src(mksp<SawWave>()).ks(0.3).mul(1.5).ADSR(5, 1000, 0.3, true, 100, Pow(-5), Pow(5), Pow(5)).build();
			case MIDIFile::Instruments::BASS_SYNTH_BASS_2:
				return AmpBuilder().src(mksp<SawWave>())
					.IIR(sampleRate, 2, 10.1, 1045.8)
					.mul(1.2)
					.ADSR(5, 1000, 0.3, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();

			//=====================================

			case MIDIFile::Instruments::SOLO_STRING_VIOLIN:
				return AmpBuilder()
					.src(mksp<BowedString>(150, 300, 500))
					.biquadEnvVel(70, 127, 1, LOWPASS)
					.ADSR(30, 100, 0.9, true, 500, Pow(-5), Pow(5), Pow(5))
					.build();

			case MIDIFile::Instruments::SOLO_STRING_VIOLA:
				return AmpBuilder()
					.src(mksp<BowedString>(130, 250, 400))
					.biquadEnvVel(70, 127, 1, LOWPASS)
					.ADSR(30, 100, 0.9, true, 500, Pow(-5), Pow(5), Pow(5))
					.build();

			case MIDIFile::Instruments::SOLO_STRING_CELLO:
				return AmpBuilder()
					.src(mksp<BowedString>(100, 200, 300))
					.biquadEnvVel(70, 127, 1, LOWPASS)
					.ADSR(30, 100, 0.9, true, 500, Pow(-5), Pow(5), Pow(5))
					.build();

			case MIDIFile::Instruments::SOLO_STRING_CONTRABASS:
				return AmpBuilder()
					.src(mksp<BowedString>(70, 150, 200))
					.biquadEnvVel(70, 127, 1, LOWPASS)
					.ADSR(30, 100, 0.9, true, 500, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::SOLO_STRING_TREMOLO_STRINGS:
				return AmpBuilder()
					.src(mksp<BowedString>(150, 300, 500))
					.biquadEnvVel(70, 127, 1, LOWPASS)
					.ADSR(30, 100, 0.9, true, 500, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::SOLO_STRING_PIZZICATO_STRINGS:
				return AmpBuilder()
					.src(mksp<BowedString>(70, 150, 200))
					.ADSR(1, 1, 1, false, 1000, Pow(-5), Pow(5), Pow(8))
					.addMul(AmpBuilder()
							.src(mksp<Pulse>(0.4, 0.1, 0.1, 0))
							.ks(0.3)
							.ADSR(10, 1, 1, false, 1500, Pow(-5), Pow(5), Pow(10))
							.build(), 0.7)
					.biquad(sampleRate, LOWPASS, 2000, 0.5, 0)
					.biquadEnvVel(70, 127, 1, LOWPASS)
					.build();
			case MIDIFile::Instruments::SOLO_STRING_ORCHESTRAL_HARP:
				return AmpBuilder().src(mksp<Pulse>(0.4, 0.1, 0.1, 0))
					.addMul(mksp<NoiseSrc>(), 0.2)
					.ks(0.3)
					.mul(0.8)
					.ADSR(5, 5000, 0, false, 300, Pow(-5), Pow(8), Pow(5))
					.biquadEnvVel(70, 127, 1, LOWPASS)
					.biquad(sampleRate, HIGHPASS, 10, 0.7, 0)
					.build();
			case MIDIFile::Instruments::SOLO_STRING_TIMPANI:
				return AmpBuilder(Matrix6x6ModulationBuilder()
								  .setOp(0, AmpBuilder(SineW()).ADSR(1, 3112, 0, false, 100, Pow(-5), Pow(5), Pow(5)).build(), 0.58, 0.0, 0.0, 1.0, 1.0)
								  .setOp(1, AmpBuilder(SineW()).ADSR(1, 100, 0, false, 100, Pow(-5), Pow(50), Pow(5)).build(), 0.86, 0.0, 0.0, 1.0, 0.0)
								  .setOp(2, AmpBuilder(SineW()).ADSR(1, 3111, 0, false, 100, Pow(-5), Pow(5), Pow(5)).build(), 0.58, 0.0, 0.0, 1.0, 1.0)
								  .setOp(3, AmpBuilder(SineW()).ADSR(1, 1514, 0.05, false, 100, Pow(-5), Pow(5), Pow(2)).build(), 0.0, 47.86, 0.0, 0.1, 0.0)
								  .setOp(4, AmpBuilder(SineW()).ADSR(1, 1139, 0.05, true, 100, Pow(-5), Pow(3), Pow(5)).build(), 0.0, 53.70, 0.0, 0.8, 0.0)
								  .setOp(5, AmpBuilder(SineW()).ADSR(1, 1961, 0.13, true, 100, Pow(-5), Pow(2), Pow(5)).build(), 2.0, 0.0, 0.0, 0.4, 0.0)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 1, 0, 8.8)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 1, 1, 0.084)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 3, 2, 10.88)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 4, 3, 10.56)
								  .setMatrix(Matrix6x6ModulationBuilder::TYPE_FM, 5, 4, 10.36)
								  .build())
					.ADSR(1, 5000, 0, false, 100, Pow(-5), Pow(1), Pow(1))
					.build();

			//=====================================

			case MIDIFile::Instruments::ENSEMBLE_STRING_ENSEMBLE_1:
				return TwoStringResonatorBuilder()
					.exciter(0.142, 0.612)
					.exciterHiCut(0.693, 1.322)
					.exciterHiCutEnv(0, 0, 0, 1)
					.exciterLowCut(0.407, 0.756)
					.string1(1, 0.866, 0.694)
					.string2(1, 0.866, 0.716)
					.stringMix(0)
					.stringEnv(201.469, 2.78742, 0.1, true, 1, 534.769)
					.stringLevel(0.336)
					.comb(1, 1, 1, 0)
					.resonatorLevel(1)
					.build();
			case MIDIFile::Instruments::ENSEMBLE_STRING_ENSEMBLE_2:
				return TwoStringResonatorBuilder()
					.exciter(0.142, 0.612).exciterHiCut(0.706, 1.994)
					.exciterHiCutEnv(0, 0, 0, 1)
					.exciterLowCut(0.407, 0.756)
					.string1(1, 0.866, 0.694)
					.string2(1, 0.866, 0.716)
					.stringMix(0)
					.stringEnv(200, 2.78742, 0.1, true, 1, 1000)
					.stringLevel(0.336)
					.comb(1, 1, 1, 0)
					.resonatorLevel(1)
					.build();
			case MIDIFile::Instruments::ENSEMBLE_SYNTH_STRINGS_1:
				return TwoStringResonatorBuilder()
					.exciter(mksp<Pulse>(0.0, 0.714, 0.04, 0))
					.exciterHiCut(0.802, 1.994)
					.exciterHiCutEnv(0, 0, 0, 1)
					.exciterLowCut(0.098, 0.756)
					.string1(1, 0.343, 0.47)
					.string2(1, 0.433, 0.559)
					.stringMix(0)
					.stringEnv(200, 2.78742, 0.1, true, 1, 1000)
					.stringLevel(0.007)
					.comb(1, 1, 1, 0)
					.resonatorLevel(1)
					.build();
			case MIDIFile::Instruments::ENSEMBLE_SYNTH_STRINGS_2:
				return TwoStringResonatorBuilder()
					.exciter(mksp<Pulse>(0.0, 0.714, 0.04, 0))
					.exciterHiCut(1, 1.994)
					.exciterHiCutEnv(0, 0, 0, 1)
					.exciterLowCut(0.098, 0.756)
					.string1(1, 0.343, 0.47)
					.string2(1, -0.537, 0.649)
					.stringMix(0.045)
					.stringEnv(200, 2.78742, 0.1, true, 1, 1000)
					.stringLevel(0.022)
					.comb(1, 1, 1, 0)
					.resonatorLevel(1)
					.build();
			case MIDIFile::Instruments::ENSEMBLE_CHOIR_AAHS:
			case MIDIFile::Instruments::ENSEMBLE_SYNTH_VOICE:
				return AmpBuilder().src(mksp<AHarmonicWave>())
					.autoMod(0.2, 0.3, 5.0, 0.75)
					.ADSR(100, 200, 0.9, true, 200, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::ENSEMBLE_VOICE_OOHS:
				return AmpBuilder()
					.src(mksp<SineWaveTable>(269.53125, DoubleArray({
					269.53125, 1.0,
					525, 0.005,
					784, 0.01,
					1051, 0.01,
					1308, 0.005,
					3122, 0.01
																	})))
					.autoMod(0.1, 0.3, 5, 0.8)
					.addMul(
						AmpBuilder(mksp<NoiseSrc>())
						.biquad(sampleRate, BANDPASS, 500, 3, 0)
						.ADSR(1, 20, 0, false, 20, Line(), Line(), Line())
						.build(), 0.3)
					.AHDSR(5, 50, 180, 0.8, true, 630, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::ENSEMBLE_ORCHESTRA_HIT:
				return AmpBuilder()
					.src(TwoStringResonatorBuilder()
						 .exciter(1, 0.257)
						 .exciterHiCut(1, 1.043)
						 .exciterHiCutEnv(1, 105.625, 1000, 0.222222)
						 .exciterLowCut(0.01, 1.004)
						 .string1(1, -1, 0.494)
						 .string2(0.25, 1, 0.485)
						 .stringMix(0.019)
						 .stringEnv(1, 0.1, 1000, false, 0, 104.256)
						 .stringLevel(0.098)
						 .comb(0.251, 0.792, 0.902, 0.593)
						 .resonator(0, 336, 0.007)
						 .resonator(1, 192.1, 0.014)
						 .resonator(2, 136.6, 0)
						 .resonatorLevel(0.177)
						 .build())
					.multyKey(IntArray({0, 12, 24}), DoubleArray({0.5, 0.5, 0.5}))
					.mul(3)
					.ADSR(1, 1000, 0, false, 100, Pow(5), Pow(5), Pow(5))
					.build();

			//=====================================

			case MIDIFile::Instruments::BRASS_TRUMPET:
				return
					AmpBuilder().src(mksp<Pulse>(0.0, 0.04, 0.03, 0.0))
					.biquadEnvGroup(DSPGroupBuilder::TYPE_CHAIN, {
						{FilterPassType::BELL, NoteFreqAmp, 0.7, 10},
									{FilterPassType::LOWPASS, NoteFreqAmp * 40, 0.5, 0}
									}
					)
					.biquad(sampleRate, LOWPASS, 17000, 0.20, 0)
					.biquad(sampleRate, HIGHPASS, 250, 0.20, 0)
					.autoMod(0.7, 0.3, 5.0, 0.1,
							 AmpBuilder().src(
								 mksp<Pulse>(0.2, 0.3, 0.35, 0))
							 .DAHDSR(100, 300, 10, 1, 1, true, 100, 100, Line(), Line(), Line()).build())
					.mul(1.5)
					.ADSR(30, 108, 0.9, true, 100, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::BRASS_MUTED_TRUMPET:
				return AmpBuilder().src(mksp<SawWave>())
					.pm(mksp<SineWave>(), 0.15, 1)
					.autoMod(0.2, 0.3, 5.2, 0.75)
					.IIR(sampleRate, 2, 10.1, 5000)
					.ADSR(50, 108, 0.8, true, 100, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::BRASS_TROMBONE:
				return AmpBuilder().src(mksp<Pulse>(0.0, 0.04, 0.1, 0.46))
					.biquad(sampleRate, BANDPASS, 1500, 0.30, 0)
					.autoMod(0.1, 0.1, 5.3, 0.5)
					.mul(3)
					.ADSR(50, 108, 0.8, true, 100, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::BRASS_TUBA:
				return AmpBuilder().src(mksp<Pulse>(0.0, 0.04, 0.1, 0.46))
					.biquad(sampleRate, BANDPASS, 100, 0.30, 0)
					.autoMod(0.1, 0.1, 5.3, 0.5)
					.mul(8)
					.ADSR(50, 762, 0.6, true, 100, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::BRASS_FRENCH_HORN:
				return AmpBuilder().src(SawW())
					.autoMod(0.1, 0.1, 5.3, 0.5)
					.detune(3, 0.05)
					.biquad(sampleRate, BANDPASS, 500, 0.30, 0)
					.ADSR(50, 762, 0.6, true, 100, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::BRASS_BRASS_SECTION:
				return AmpBuilder(mksp<Pulse>(0.0, 0.0, 0.1, 0))
					.detune(3, 0.05)
					.biquad(sampleRate, HIGHPASS, 500, 1, 0)
					.biquad(sampleRate, LOWPASS, 10000, 0.5, 0)
					.ADSR(50, 100, 0.8, true, 1000, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::BRASS_SYNTH_BRASS_1:
				return AmpBuilder(SawW())
					.detune(3, 0.07)
					.biquadEnv(AmpBuilder().src(ConstAmp(45)).ADSR(100, 500, 0, true, 100, Pow(5), Pow(3), Pow(5)).add(ConstAmp(75)).build(), ConstAmp(1), LOWPASS)
					.ADSR(50, 100, 0.8, true, 1000, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::BRASS_SYNTH_BRASS_2:
				return AmpBuilder(SawW())
					.detune(3, 0.1)
					.biquadEnv(AmpBuilder().src(ConstAmp(30)).ADSR(100, 1000, 0, true, 100, Pow(5), Pow(3), Pow(5)).add(ConstAmp(90)).build(), ConstAmp(1), LOWPASS)
					.ADSR(50, 100, 0.8, true, 1000, Pow(-5), Pow(5), Pow(5))
					.build();

			//=====================================

			case MIDIFile::Instruments::REED_SOPRANO_SAX:
				return AmpBuilder().src(mksp<Pulse>(0.0, 0.91, 0.06, 0.0))
					.pm(mksp<SineWave>(), 0.3, 3)
					.biquadEnv(AmpBuilder().src(NoteIDAmp).add(34.6).clamp(90).build(), ConstAmp(7), FilterPassType::LOWPASS)
					.autoMod(0.2, 0.3, 5.11, 0.75)
					.mul(0.5)
					.ADSR(70, 100, 0.8, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::REED_ALTO_SAX:
				return AmpBuilder().src(mksp<Pulse>(0.0, 0.91, 0.06, 0.0))
					.pm(mksp<SineWave>(), 0.3, 3)
					.biquadEnv(AmpBuilder().src(NoteIDAmp).add(44.6).clamp(100).build(), ConstAmp(7), FilterPassType::LOWPASS)
					.autoMod(0.2, 0.3, 5.11, 0.75)
					.mul(0.5)
					.ADSR(70, 100, 0.8, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::REED_TENOR_SAX:
				return AmpBuilder().src(mksp<Pulse>(0.0, 0.91, 0.06, 0.0))
					.pm(mksp<SineWave>(), 0.3, 3)
					.biquadEnv(AmpBuilder().src(NoteIDAmp).add(54.6).clamp(105).build(), ConstAmp(7), FilterPassType::LOWPASS)
					.autoMod(0.2, 0.3, 5.11, 0.75)
					.mul(0.5)
					.ADSR(70, 100, 0.8, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::REED_BARITONE_SAX:
				return AmpBuilder().src(mksp<Pulse>(0.0, 0.91, 0.06, 0.0))
					.pm(mksp<SineWave>(), 0.3, 3)
					.biquadEnv(AmpBuilder().src(NoteIDAmp).add(60.6).clamp(115).build(), ConstAmp(5), FilterPassType::LOWPASS)
					.autoMod(0.2, 0.3, 5.11, 0.75)
					.mul(0.5)
					.ADSR(70, 100, 0.8, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::REED_OBOE:
				return AmpBuilder().src(mksp<Pulse>(0.0, 0.71, 0.06, 0.0))
					.pm(mksp<SineWave>(), 0.2, 3)
					.add(mksp< AHarmonicWave>())
					.biquadEnv(AmpBuilder().src(NoteIDAmp).add(24.6).build(), ConstAmp(7), FilterPassType::LOWPASS)
					.autoMod(0.2, 0.3, 5.11, 0.75)
					.mul(0.3)
					.ADSR(70, 100, 0.8, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::REED_ENGLISH_HORN:
				return AmpBuilder().src(mksp<Pulse>(0.0, 0.91, 0.06, 0.0))
					.pm(mksp<SineWave>(), 0.3, 3)
					.biquadEnv(AmpBuilder().src(NoteIDAmp).add(60.6).clamp(110).build(), ConstAmp(7), FilterPassType::LOWPASS)
					.autoMod(0.2, 0.3, 5.11, 0.75)
					.mul(0.5)
					.ADSR(70, 100, 0.8, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::REED_BASSOON:
				return AmpBuilder().src(mksp<Pulse>(0.0, 0.91, 0.06, 0.0))
					.pm(mksp<SineWave>(), 0.3, 3)
					.biquadEnv(AmpBuilder().src(NoteIDAmp).add(50.6).clamp(100).build(), ConstAmp(5), FilterPassType::LOWPASS)
					.autoMod(0.2, 0.3, 5.11, 0.75)
					.mul(0.5)
					.ADSR(70, 100, 0.8, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::REED_CLARINET:
				return AmpBuilder().src(mksp<SineWave>())
					.pm(mksp<SineWave>(), 0.3, 3)
					.autoMod(0.2, 0.3, 5.11, 0.75)
					.ADSR(50, 100, 0.7, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();

			//=====================================

			case MIDIFile::Instruments::PIPE_PICCOLO:
				return AmpBuilder().src(mksp<SineWave>())
					.pm(mksp<SineWave>(), 0.05, 2)
					.autoMod(0.2, 0.3, 5.0, 0.75)
					.ADSR(85, 834, 0.8, true, 100, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::PIPE_FLUTE:
				return AmpBuilder().src(mksp<SineWave>())
					.pm(mksp<SineWave>(), 0.2, 2)
					.autoMod(0.2, 0.3, 6.0, 0.75)
					.ADSR(50, 100, 0.7, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::PIPE_RECORDER:
				return AmpBuilder().src(mksp<TriWave>())
					.pm(mksp<SineWave>(), 0.02, 2)
					.autoMod(0.2, 0.3, 5.5, 0.75)
					.ADSR(50, 100, 0.7, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::PIPE_PAN_FLUTE:
				return TwoStringResonatorBuilder()
					.exciter(AmpBuilder().src(mksp<Pulse>(0.3, 0.2, 0.2, 0)).add(mksp<NoiseSrc>()).build())
					.exciterHiCut(0.93, 1)
					.exciterHiCutEnv(97.2222, 0, 0, 0.791667)
					.exciterLowCut(0.11, 1)
					.string1(1, 0.015, 0.466)
					.string2(2, -0.015, 0.477)
					.stringMix(0.015)
					.stringEnv(0, 0, 62.5, true, 0.875, 100)
					.stringLevel(0.022)
					.comb(1, 1, -1, 0)
					.resonatorLevel(1)
					.build();
			case MIDIFile::Instruments::PIPE_BOTTLE_BLOW:
				return TwoStringResonatorBuilder()
					.exciter(AmpBuilder().src(mksp<Pulse>(0.3, 0.2, 0.2, 0)).add(mksp<NoiseSrc>()).build())
					.exciterHiCut(0.76, 1)
					.exciterHiCutEnv(97.2222, 0, 0, 0.791667)
					.exciterLowCut(0.11, 1)
					.string1(1, 0.015, 0.789)
					.string2(2, -0.09, 0.808)
					.stringMix(0.015)
					.stringEnv(0, 0, 62.5, true, 0.875, 100)
					.stringLevel(0.022)
					.comb(1, 1, -1, 0)
					.resonatorLevel(1)
					.build();
			case MIDIFile::Instruments::PIPE_SHAKUHACHI:
				return TwoStringResonatorBuilder()
					.exciter(AmpBuilder().src(mksp<Pulse>(0.3, 0.2, 0.2, 0)).add(mksp<NoiseSrc>()).build())
					.exciterHiCut(0.91, 1)
					.exciterHiCutEnv(256.944, 0, 0, 0.909722)
					.exciterLowCut(0.04, 1)
					.string1(1, 0.015, 0.789)
					.string2(2, -0.09, 0.808)
					.stringMix(0.015)
					.stringEnv(0, 0, 62.5, true, 0.875, 100)
					.stringLevel(0.015)
					.comb(0.508, 1, -1, 0)
					.resonatorLevel(1)
					.build();
			case MIDIFile::Instruments::PIPE_WHISTLE:
				return AmpBuilder().src(mksp<SineWave>())
					.am(mksp<SineWave>(), 0.3, 5)
					.ADSR(50, 100, 0.7, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::PIPE_OCARINA:
				return AmpBuilder().src(mksp<SineWave>())
					.pm(mksp<SineWave>(), 0.2, 2)
					.autoMod(0.2, 0.3, 5.3, 0.75)
					.ADSR(50, 100, 0.7, true, 100, Pow(-5), Pow(5), Pow(5))
					.build();

			//=====================================

			case MIDIFile::Instruments::LEAD_SQUARE:
				return AmpBuilder().src(NearestAmpSetBuilder()
										.add(24, mksp<Pulse>(0.125, 0, 0, 0))
										.add(60, mksp<Pulse>(0.25, 0, 0, 0))
										.add(72, mksp<Pulse>(0.5, 0, 0, 0))
										.build())
					//.autoMod(0.2, 0.2, 7, 0.75)
					.detune(3, 0.15)
					.mul(0.707)
					.biquad(sampleRate, HIGHPASS, 10, 0.7, 0)
					.biquadEnvVel(90, 127, 1, LOWPASS)
					.AHDSR(5, 30, 300, 0.75, true, 300, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::LEAD_SAWTOOTH:
				return AmpBuilder().src(mksp<SawWave>())
					//.autoMod(0.2, 0.2, 7.5, 0.75)
					.detune(3, 0.15)
					.biquadEnvVel(90, 127, 1, LOWPASS)
					.AHDSR(5, 30, 300, 0.75, true, 300, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::LEAD_CALIOPE_LEAD:
				return TwoStringResonatorBuilder()
					.exciter(mksp<SawWave>())
					.exciterHiCut(0.82, 1.01)
					.exciterHiCutEnv(0, 0, 145.833, 0.638889)
					.exciterLowCut(0.35, 0.93)
					.string1(1, 0.181, 0.502)
					.string2(1, 0.536, 0.495)
					.stringMix(0.024)
					.stringEnv(100, 0, 100, true, 0.6875, 100)
					.stringLevel(0.03)
					.comb(1, 1, -1, 0)
					.resonatorLevel(1)
					.build();
			case MIDIFile::Instruments::LEAD_CHIFF_LEAD:
				return AmpBuilder().src(mksp<SawWave>())
					.biquadEnv(NoteIDAmp + ConstAmp(10) + (AmpBuilder(ConstAmp(30)).ADSR(5, 1000, 0, false, 100, Pow(-5), Pow(5), Pow(5)).build()), ConstAmp(1), LOWPASS)
					.ADSR(100, 200, 0.9, true, 200, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::LEAD_CHARANG:
				return AmpBuilder().src(mksp<SawWave>())
					.biquadEnv(NoteIDAmp + ConstAmp(30) + (AmpBuilder(ConstAmp(10)).ADSR(5, 1000, 0, false, 100, Pow(5), Pow(5), Pow(5)).build()), ConstAmp(1), LOWPASS)
					.biquad(sampleRate, LOWPASS, 6000, 1, 0)
					.ADSR(100, 200, 0.9, true, 200, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::LEAD_VOICE:
				return AmpBuilder().src(mksp<AHarmonicWave>())
					.detune(3, 0.2)
					.autoMod(0.2, 0.3, 5.5, 0.75)
					.ADSR(100, 200, 0.9, true, 200, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::LEAD_FIFTHS:
				return AmpBuilder().src(mksp<SawWave>())
					.add(mksp<SawWave>(MulPhase(NotePhase, 3.0 / 2.0)))
					.autoMod(0.3, 0.3, 5.4, 0.75)
					.mul(0.5)
					.detune(3, 0.15)
					.biquadEnvVel(90, 127, 1, LOWPASS)
					.AHDSR(5, 30, 300, 0.75, true, 300, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::LEAD_BASS_LEAD:
				return AmpBuilder().src(mksp<SawWave>())
					.biquadEnv(NoteIDAmp + ConstAmp(30) + (AmpBuilder(ConstAmp(30)).ADSR(5, 1000, 0, false, 100, Pow(-5), Pow(5), Pow(5)).build()), ConstAmp(1), LOWPASS)
					.biquad(sampleRate, LOWPASS, 6000, 1, 0)
					.ADSR(50, 200, 0.9, true, 200, Pow(-5), Pow(5), Pow(5))
					.build();

			//=====================================

			case MIDIFile::Instruments::PAD_NEW_AGE:
				return  AmpBuilder().src(TwoStringResonatorBuilder()
										 .exciter(0, 1)
										 .exciterHiCut(0.677, 0.76)
										 .exciterHiCutEnv(0, 0, 48.7062, 1)
										 .exciterLowCut(0.248, 0.666)
										 .string1(1, -0.512, 0.716)
										 .string2(1, 1, 0.774)
										 .stringMix(0.018)
										 .stringEnv(0.1, 27.875, 27.875, true, 1, 2000)
										 .stringLevel(1)
										 .comb(0.251, 1, -0.36, 0.387)
										 .resonatorLevel(0.358)
										 .build())
					.mul(2)
					.build();
			case MIDIFile::Instruments::PAD_WARM:
				return TwoStringResonatorBuilder()
					.exciter(0.09, 1)
					.exciterHiCut(0.547, 2.41)
					.exciterHiCutEnv(0.1, 48.7062, 659.756, 0.0972218)
					.exciterLowCut(0.248, 0.666)
					.string1(1, 1, 0.555)
					.string2(1, 1, 0.558)
					.stringMix(-0.006)
					.stringEnv(2000, 27.875, 27.875, true, 1, 2000)
					.stringLevel(1)
					.comb(1, 1, 1, 0)
					.resonator(0, 648, 0)
					.resonator(1, 1446.1, 0)
					.resonatorLevel(0.358)
					.build();
			case MIDIFile::Instruments::PAD_POLYSYNTH:
				return TwoStringResonatorBuilder()
					.exciter(0.09, 1)
					.exciterHiCut(0.843, 0.76)
					.exciterHiCutEnv(0.1, 69.5375, 319.512, 0.631944)
					.exciterLowCut(0.248, 0.666)
					.string1(1, 1, 0.732)
					.string2(1, 1, 0.738)
					.stringMix(-0.006)
					.stringEnv(0.1, 27.875, 27.875, true, 1, 1000)
					.stringLevel(0.872)
					.comb(1, -0.463, -0.47, 0.314)
					.resonatorLevel(0.358)
					.build();
			case MIDIFile::Instruments::PAD_CHOIR:
				return AmpBuilder().src(mksp<AHarmonicWave>())
					.autoMod(0.2, 0.3, 5.0, 0.75)
					.ADSR(100, 200, 0.9, true, 200, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::PAD_BOWED:
				return  AmpBuilder().src(TwoStringResonatorBuilder()
										 .exciter(0.148, 1)
										 .exciterHiCut(0.648, 1.279)
										 .exciterHiCutEnv(48.7062, 0.1, 0.1, 0.611111)
										 .exciterLowCut(0.048, 0.666)
										 .string1(2, 1, 0.512)
										 .string2(1, 1, 0.497)
										 .stringMix(0.018)
										 .stringEnv(1000, 27.875, 27.875, true, 1, 2000)
										 .stringLevel(0.96)
										 .comb(0.251, 1, -0.36, 0.256)
										 .resonatorLevel(0.082)
										 .build())
					.mul(2)
					.build();
			case MIDIFile::Instruments::PAD_METALLIC:
				return AmpBuilder().src(TwoStringResonatorBuilder()
										.exciter(0.09, 1)
										.exciterHiCut(0.843, 0.76)
										.exciterHiCutEnv(0.1, 69.5375, 319.512, 0.618055)
										.exciterLowCut(0.248, 0.666)
										.string1(1, 1, 0.497)
										.string2(1, 1, 0.491)
										.stringMix(-0.006)
										.stringEnv(3000, 27.875, 27.875, true, 1, 2000)
										.stringLevel(0.369)
										.comb(0.509, -0.226, -0.207, 0.25)
										.resonatorLevel(0.358)
										.build())
					.mul(8)
					.build();
			case MIDIFile::Instruments::PAD_HALO:
				return AmpBuilder().src(TwoStringResonatorBuilder()
										.exciter(0.09, 1)
										.exciterHiCut(0.738, 0.76)
										.exciterHiCutEnv(0.1, 0.1, 319.512, 0.520833)
										.exciterLowCut(0.248, 0.666)
										.string1(2, 1, 0.497)
										.string2(1, 1, 0.491)
										.stringMix(-0.006)
										.stringEnv(1000, 27.875, 27.875, true, 1, 2000)
										.stringLevel(1)
										.comb(0.25, -0.518, -0.39, 0.799)
										.resonatorLevel(0.358)
										.build())
					.mul(2)
					.build();
			case MIDIFile::Instruments::PAD_SWEEP:
				return  AmpBuilder().src(TwoStringResonatorBuilder()
										 .exciter(0, 1)
										 .exciterHiCut(0.724, 0.76)
										 .exciterHiCutEnv(0.1, 0.1, 48.7058, 1)
										 .exciterLowCut(0.248, 0.666)
										 .string1(1, -0.512, 1)
										 .string2(1, 1, 1)
										 .stringMix(-0.006)
										 .stringEnv(1000, 27.875, 27.875, true, 1, 2000)
										 .stringLevel(1)
										 .comb(0.25, 1, -0.39, 0.799)
										 .resonatorLevel(0.358)
										 .build())
					.mul(2)
					.build();

			//=====================================

			case MIDIFile::Instruments::FX_SOUNDTRACK:
				return AmpBuilder().src(mksp<TriWave>())
					.pm(mksp<TriWave>(), 0.61, 1)
					.ADSR(90, 1628, 0.49, true, 991, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::FX_CRYSTAL:
				return AmpBuilder().src(mksp<SineWave>())
					.pm(mksp<SineWave>(), 0.09, 6)
					.am(mksp<SineWave>(), 0.35, 6.3)
					.ADSR(10, 0, 1, false, 1500, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::FX_ATMOSPHERE:
				return AmpBuilder().src(mksp<SineWave>())
					.pm(mksp<SineWave>(), 0.2, 1)
					.ADSR(10, 1025, 0.26, true, 100, Pow(5), Pow(5), Pow(5))
					.clamp(0.87, 0.29)
					.build();
			case MIDIFile::Instruments::FX_BRIGHTNESS:
				return AmpBuilder().src(mksp<SineWave>())
					.pm(mksp<SineWave>(), 0.19, 1)
					.ADSR(33, 694, 0.68, true, 171, Pow(5), Pow(5), Pow(5))
					.IIR(sampleRate, 1, 10.1, 552.1)
					.build();
			case MIDIFile::Instruments::FX_GOBLINS:
				return AmpBuilder().src(mksp<SineWave>())
					.pm(mksp<SineWave>(), 0.04, 0.09090909090909091)
					.am(mksp<SineWave>(), 0.51, 6)
					.ADSR(200, 2455, 0.44, true, 1500, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::FX_ECHOES:
				return AmpBuilder().src(mksp<AHarmonicWave>())
					.autoMod(0.2, 0.3, 5.5, 0.75)
					.detune(3, 0.2)
					.ADSR(100, 200, 0.9, true, 200, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::FX_SCI_FI:
				return AmpBuilder().src(mksp<SawWave>())
					.biquadEnv(NoteIDAmp + ConstAmp(25), ConstAmp(1), LOWPASS)
					.ADSR(100, 200, 0.9, true, 200, Pow(-5), Pow(5), Pow(5))
					.build();

			//=====================================

			case MIDIFile::Instruments::ETHNIC_SITAR:
				return AmpBuilder().src(mksp<Sitar>()).ADSR(50, 5000, 0, false, 100, Pow(5), Pow(5), Pow(5)).build();
			case MIDIFile::Instruments::ETHNIC_BANJO:
				return AmpBuilder().ks(0.22)
					.IIR(sampleRate, 2, 100, 6000.0)
					.ADSR(10, 1500, 0, false, 100, Pow(-20), Pow(8), Pow(5))
					.build();
			case MIDIFile::Instruments::ETHNIC_SHAMISEN:
				return AmpBuilder().ks(0.2).ADSR(10, 1500, 0, false, 100, Pow(-20), Pow(8), Pow(5)).build();
			case MIDIFile::Instruments::ETHNIC_KOTO:
				return AmpBuilder().ks(0.85).ADSR(10, 3000, 0, false, 100, Pow(-20), Pow(8), Pow(5)).build();
			case MIDIFile::Instruments::ETHNIC_KALIMBA:
				return AmpBuilder().src(mksp<SineWave>())
					.pm(mksp<SineWave>(), 0.09, 1)
					.addMul(AmpBuilder().src(mksp<SineWave>())
							.pm(mksp<SineWave>(), 0.7, 1)
							.noteShift(32)
							.ADSR(4, 1, 1, false, 300, Pow(5), Pow(5), Pow(5))
							.build(), 0.05)
					.ADSR(4, 359, 0.4, false, 937, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::ETHNIC_BAGPIPE:
				return AmpBuilder()
					.src(mksp<Pulse>(0.136, 0.015, 0.0087, 0))
					.add(mksp<Pulse>(MulPhase(NotePhase, 1.5), 0.136, 0.015, 0.0087, 0))
					.add(mksp<Pulse>(MulPhase(NotePhase, 2), 0.136, 0.015, 0.0087, 0))
					.add(mksp<Pulse>(MulPhase(NotePhase, 4), 0.136, 0.015, 0.0087, 0))
					.biquadEnvGroup(DSPGroupBuilder::TYPE_CHAIN, {
						{FilterPassType::LOWSHELF, NoteFreqAmp * 4, 1, -15},
									{FilterPassType::BELL, NoteFreqAmp * 8, 10, 5},
									{FilterPassType::BELL, NoteFreqAmp * 11, 1, -12},
									{FilterPassType::NOTCH, 13000, 3, 0},
									{FilterPassType::NOTCH, NoteFreqAmp * 1.5, 10, 0},
									{FilterPassType::NOTCH, NoteFreqAmp * 4.5, 10, 0},
									{FilterPassType::NOTCH, NoteFreqAmp * 7.5, 10, 0},
									{FilterPassType::HIGHPASS, 10, 1, 0},
									{FilterPassType::HIGHSHELF, 5000, 0.3, -9},
									{FilterPassType::LOWPASS, 15000, 0.3, 0}
									})
					.ADSR(50, 359, 0.9, true, 300, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::ETHNIC_FIDDLE:
				return AmpBuilder().src(mksp<BowedString>(150, 300, 500)).ADSR(100, 100, 0.9, true, 500, Pow(-5), Pow(5), Pow(5)).build();
			case MIDIFile::Instruments::ETHNIC_SHANAI:
				return AmpBuilder().src(mksp<Pulse>(0.08, 0.02, 0.037, 0))
					.biquadEnvGroup(DSPGroupBuilder::TYPE_CHAIN, {
						{FilterPassType::BANDPASS, 1431, 1.7, 0},
									{FilterPassType::NOTCH, NoteFreqAmp * 8, 5, 0},
									{FilterPassType::BELL, NoteFreqAmp, 10, -5},
									{FilterPassType::BELL, NoteFreqAmp * 2, 10, -5},
									{FilterPassType::BELL, NoteFreqAmp * 6, 10, -27},
									{FilterPassType::BELL, NoteFreqAmp * 9, 10, 12},
									{FilterPassType::BELL, NoteFreqAmp * 16, 2, 7}
									})
					.ADSR(50, 359, 0.9, true, 300, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::PERCUSSIVE_TINKLE_BELL:
				return AmpBuilder(mksp<SineWave>())
					.pm(AmpBuilder(mksp<SineWave>())
						.freqSrc(MulPhase(NotePhase, 6.5))
						.ADSR(5, 700, 0, false, 100, Pow(5), Pow(3), Pow(5))
						.build(), 0.308)
					.ADSR(5, 1500, 0, false, 100, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::PERCUSSIVE_AGOGO:
				return AmpBuilder()
					.src(mksp<NoiseSrc>())
					.add(mksp<SquareWave>())
					.add(mksp<SquareWave>(MulPhase(NotePhase, 1243.0 / 529.0)))
					.biquadEnvGroup(DSPGroupBuilder::TYPE_CHAIN, {
						{FilterPassType::BELL, NoteFreqAmp, 30, 90},
									{FilterPassType::BELL, NoteFreqAmp * (1243.0 / 529.0), 30, 30},
									{FilterPassType::BELL, NoteFreqAmp * (1752.0 / 529.0), 30, 20},
									{FilterPassType::BELL, NoteFreqAmp * (2182.0 / 529.0), 30, 20},
									{FilterPassType::BELL, NoteFreqAmp * (2526.0 / 529.0), 30, 30},
									{FilterPassType::BELL, NoteFreqAmp * (2582.0 / 529.0), 30, 30},
									{FilterPassType::BELL, NoteFreqAmp * (3667.0 / 529.0), 30, 30},
									{FilterPassType::BELL, NoteFreqAmp * (4032.0 / 529.0), 30, 30},
									{FilterPassType::BELL, NoteFreqAmp * (5095.0 / 529.0), 30, 30},
									{FilterPassType::BELL, NoteFreqAmp * (6207.0 / 529.0), 30, 30},
									{FilterPassType::BELL, NoteFreqAmp * (7451.0 / 529.0), 30, 30},
									{FilterPassType::BELL, NoteFreqAmp * (8688.0 / 529.0), 30, 30},
									{FilterPassType::BELL, NoteFreqAmp * (9347.0 / 529.0), 30, 30},
									{FilterPassType::HIGHPASS, NoteFreqAmp * (2000.0 / 529.0), 0.5, 0},
									{FilterPassType::LOWPASS, NoteFreqAmp * (8000.0 / 529.0), 0.5, 0}
									})
					.mul(0.01)
					.AHDSR(1, 10, 1, 1, false, 1000, Pow(5), Pow(5), Pow(10))
					.build();
			case MIDIFile::Instruments::PERCUSSIVE_STEEL_DRUMS:
				return AmpBuilder()
					.src(mksp<SineWaveTable>(269.53125, DoubleArray({
					269.53125, 1.0, 363.28125, 0.07235363857701663, 398.4375, 0.12799743578985023, 433.59375, 0.08737175770435823, 515.625, 0.6206402594994691, 539.0625, 0.3597885911495093, 597.65625, 0.08264700342927812, 656.25, 0.7188440200746729, 785.15625, 0.3795586131241527, 855.46875, 0.011978267525183701, 914.0625, 0.07184674145688824, 1054.6875, 0.22712185759377435, 1171.875, 0.051998250533269966, 1312.5, 0.030940107100424292, 4464.84375, 0.012544763401347499
																	})))
					.biquadEnv(AmpBuilder().src(ConstAmp(60))
							   .ADSR(30, 1, 1, true, 100, Line(), Line(), Line())
							   .add(ConstAmp(60))
							   .build(), ConstAmp(1), FilterPassType::LOWPASS)
					.AHDSR(30, 50, 180, 0.3, false, 630, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::PERCUSSIVE_WOODBLOCK:
				return AmpBuilder()
					.src(mksp<NoiseSrc>())
					.biquadEnvGroup(DSPGroupBuilder::TYPE_CHAIN, {
						{FilterPassType::BELL, NoteFreqAmp * (548.0 / 548.0), 10, 40},
									{FilterPassType::BELL, NoteFreqAmp * (612.0 / 548.0), 10, 40},
									{FilterPassType::BELL, NoteFreqAmp * (825.0 / 548.0), 15, 15},
									{FilterPassType::BELL, NoteFreqAmp * (1216.0 / 548.0), 15, 40},
									{FilterPassType::BELL, NoteFreqAmp * (1337.0 / 548.0), 15, 40},
									{FilterPassType::BELL, NoteFreqAmp * (1727.0 / 548.0), 10, 15},
									{FilterPassType::BELL, NoteFreqAmp * (2166.0 / 548.0), 5, 15},
									{FilterPassType::BELL, NoteFreqAmp * (2417.0 / 548.0), 15, 25},
									{FilterPassType::BELL, NoteFreqAmp * (2923.0 / 548.0), 15, 30},
									{FilterPassType::BELL, NoteFreqAmp * (3408.0 / 548.0), 15, 30},
									{FilterPassType::LOWPASS, NoteFreqAmp * (10000.0 / 548.0), 0.5, 0}
									})
					.mul(0.01)
					.AHDSR(1, 10, 1, 1, false, 1000, Pow(5), Pow(5), Pow(20))
					.build();
			case MIDIFile::Instruments::PERCUSSIVE_TAIKO_DRUM:
				return SimpleDrumSet::tom(sampleRate, 0.5);
			case MIDIFile::Instruments::PERCUSSIVE_MELODIC_TOM:
				return SimpleDrumSet::tom(sampleRate);
			case MIDIFile::Instruments::PERCUSSIVE_SYNTH_DRUM:
				return AmpBuilder()
					.src(mksp<SineWave>())
					.drum(4, 0.5, 0.5, SimpleDrumAmp::MODE_NOTE_RATIO, Pow(-3))
					.ADSR(5, 7, 1, false, 500, Pow(-5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::PERCUSSIVE_REVERSE_CYMBAL:
				return AmpBuilder(mksp<CymbalOsc>(NoteFreqAmp * (1 / 800.0), 0.12, IntArray{305, 444, 558, 630, 794, 824, 1136}))
					.ADSR(2000, 5, 1, false, 2000, Pow(2), Pow(2), Pow(2)).build();
			case MIDIFile::Instruments::EFFECTS_SEASHORE:
				return AmpBuilder().src(mksp<NoiseSrc>())
					.IIR(sampleRate, 1, 103.3, 4060.0)
					.biquadEnv(AmpBuilder().src(ConstAmp(50))
							   .ADSR(500, 0, 1, true, 1500, Pow(5), Pow(5), Pow(3))
							   .add(ConstAmp(70))
							   .build(), ConstAmp(1), FilterPassType::LOWPASS)
					.ADSR(1500, 1500, 0, false, 1500, Pow(3), Pow(3), Pow(3))
					.build();
			case MIDIFile::Instruments::EFFECTS_APPLAUSE:
				return AmpBuilder().src(mksp<NoiseSrc>())
					.IIR(sampleRate, 2, 123.2, 2224.6)
					.ADSR(219, 0, 1, true, 656, Pow(5), Pow(5), Pow(5))
					.build();
			case MIDIFile::Instruments::EFFECTS_GUNSHOT:
				return AmpBuilder().src(AmpBuilder().src(mksp<NoiseSrc>())
										.addMul(mksp<SquareWave>(ConstPhase(51)), 0.1)
										.mean(mksp<TimeEnvelop>(2100, Pow(-4)), 130)
										.build()).ADSR(2, 0, 0, false, 2000, Pow(5), Pow(5), mksp<GraphInterpolator>(std::initializer_list<double>{
					0, 0,
						0.15, 0.00001,
						0.35, 0.001,
						0.65, 0.01,
						0.75, 0.1,
						0.85, 0.3,
						0.964, 1,
						0.965, 0,
						0.974, 1,
						0.975, 0,
						0.984, 1,
						0.985, 0,
						0.994, 1,
						0.995, 0,
						1, 1})).build();
			default:
				return nullptr;
		}
	}
	NoteProcPtr SimpleMIDIInstrument::getDrumSet(s_bank_id bank, u_sample_rate sampleRate){
		return mksp<SimpleDrumSet>();
	}
}