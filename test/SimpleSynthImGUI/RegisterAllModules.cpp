#include "RegisterAllModules.h"
#include "array/SampleProvider.h"
#include "interpolator/BezierInterpolator.h"
#include "interpolator/GraphInterpolator.h"
#include "interpolator/LineInterpolator.h"
#include "interpolator/PowInterpolator.h"
#include "util/Lang.h"
//
#include "dsp/AllPassFilter.h"
#include "dsp/BiquadIIR.h"
#include "dsp/Chorus.h"
#include "dsp/Compressor.h"
#include "dsp/Delayer.h"
#include "dsp/DSPChain.h"
#include "dsp/DSPParallel.h"
#include "dsp/EnvelopDetector.h"
#include "dsp/FeedbackCombFilter.h"
#include "dsp/FeedforwardCombFilter.h"
#include "dsp/FIR.h"
#include "dsp/FDNReverb.h"
#include "dsp/FDNReverb2.h"
#include "dsp/Freeverb.h"
#include "dsp/HRIR.h"
#include "dsp/IIR.h"
#include "dsp/Integrator.h"
#include "dsp/Limiter.h"
#include "dsp/LowPassFeedbackCombFilter.h"
#include "dsp/NoiseGate.h"
#include "dsp/Phaser.h"
#include "dsp/SchroederReverb.h"
#include "dsp/UnitDelay.h"

//
#include "synth/composed/AmpAdder.h"
#include "synth/composed/AutoMod.h"
#include "synth/composed/Matrix6x6Modulation.h"
#include "synth/composed/FreqModAmp.h"
#include "synth/composed/NoteDSP.h"
#include "synth/composed/PostProcessDSP.h"
#include "synth/composed/AmpMultiplier.h"
#include "synth/composed/AmpWithCC.h"
#include "synth/composed/AmpQuantization.h"
#include "synth/composed/MultiKeyTrigger.h"
#include "synth/composed/NoteVelocityMix.h"
#include "synth/composed/NoteShift.h"
#include "synth/composed/SimpleDetuner.h"
#include "synth/composed/SoftSync.h"
#include "synth/composed/HardSync.h"
//
#include "synth/filters/BiquadFilterSrc.h"
#include "synth/filters/MeanFilterSrc.h"
//
#include "synth/generators/drum/SimpleDrumAmp.h"
//
#include "synth/generators/noise/NoiseSrc.h"
#include "synth/generators/noise/LFSRNoise.h"
//
#include "synth/generators/sine/SineWave.h"
//
#include "synth/generators/pulse/CymbalOsc.h"
#include "synth/generators/pulse/TriWave.h"
#include "synth/generators/pulse/SawWave.h"
#include "synth/generators/pulse/SquareWave.h"
#include "synth/generators/pulse/Pulse.h"
#include "synth/generators/pulse/VVVF.h"
//
#include "synth/generators/physic/KarplusStrongSrc.h"
#include "synth/generators/physic/BowedString.h"
#include "synth/generators/physic/Sitar.h"
#include "synth/generators/physic/SakuraExciter.h"
#include "synth/generators/physic/Sakura.h"
#include "synth/generators/physic/PianoSrc.h"
//
#include "synth/generators/sampler/WaveSampler.h"
//
#include "synth/envelopers/AHDSREnvelop.h"
#include "synth/envelopers/TimeEnvelop.h"
#include "synth/envelopers/GraphEnvelop.h"
#include "synth/envelopers/EnvelopMultiplier.h"
#include "synth/envelopers/MultiStageEnvelope.h"
//
#include "synth/nonlinear/ArctanDistortion.h"
#include "synth/nonlinear/ClampAmp.h"
#include "synth/nonlinear/ClampWithVelocityAmp.h"
#include "synth/nonlinear/SoftClipAmp.h"
#include "synth/nonlinear/TapeSaturationDistortion.h"
#include "synth/nonlinear/FoldbackDistortion.h"
//
#include "synth/source/AmplitudeSources.h"
#include "imgui.h"
#include "dsp/IIRUtil.h"
#include "SimpleSynthWindow.h"
#include "sakura/SakuraRender.h"
#include "piano/PianoRender.h"
#include "sample/SampleRender.h"
#include "dsprender/DSPRender.h"
#include "interpolatorrender/InterpolatorRender.h"
#include "notesrcrender/NoteSrcRender.h"
using namespace yzrilyzr_simplesynth;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_array;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
void registerAllNoteProcessor(Lang & lang, MenuRegister & reg){
	// 复合模块（Composed）分类 - 原有翻译键已补充，此处保持一致
	String composedsn=lang.get("register_module.category.notesrc.composed");
	String composed="Composed";
	reg.registerModule(lang, composed, composedsn, "AmpAdder", "register_module.notesrc.name.add", [](){return mksp<AmpAdder>();});
	reg.registerModule(lang, composed, composedsn, "AmpMultiplier", "register_module.notesrc.name.mul", [](){return mksp<AmpMultiplier>();});
	reg.registerModule(lang, composed, composedsn, "FreqModAmp", "register_module.notesrc.name.freq_mod", [](){return mksp<FreqModAmp>();});
	reg.registerModule(lang, composed, composedsn, "Matrix6x6Modulation", "register_module.notesrc.name.matrix_6x6_modulation", [](){return mksp<Matrix6x6Modulation>();}, Matrix6x6ModulationRenderFunc, false);
	reg.registerModule(lang, composed, composedsn, "NoteDSP", "register_module.notesrc.name.note_dsp", [](){return mksp<NoteDSP>();});
	reg.registerModule(lang, composed, composedsn, "PostProcessDSP", "register_module.notesrc.name.post_process_dsp", [](){return mksp<PostProcessDSP>();});
	reg.registerModule(lang, composed, composedsn, "AmpWithCC", "register_module.notesrc.name.cc", [](){return mksp<AmpWithCC>();});
	reg.registerModule(lang, composed, composedsn, "MultiKeyTrigger", "register_module.notesrc.name.multi_key_trigger", [](){return mksp<MultiKeyTrigger>();});
	reg.registerModule(lang, composed, composedsn, "NoteShift", "register_module.notesrc.name.note_shift", [](){return mksp<NoteShift>();});
	reg.registerModule(lang, composed, composedsn, "NoteVelocityMix", "register_module.notesrc.name.note_velocity_mix", [](){return mksp<NoteVelocityMix>();});
	reg.registerModule(lang, composed, composedsn, "AmpQuantization", "register_module.notesrc.name.quantization", [](){return mksp<AmpQuantization>();});
	reg.registerModule(lang, composed, composedsn, "AutoMod", "register_module.notesrc.name.automod", [](){return mksp<AutoMod>();});
	reg.registerModule(lang, composed, composedsn, "SimpleDetuner", "register_module.notesrc.name.simpledetuner", [](){return mksp<SimpleDetuner>();});
	reg.registerModule(lang, composed, composedsn, "SoftSync", "register_module.notesrc.name.softsync", [](){return mksp<SoftSync>();});
	reg.registerModule(lang, composed, composedsn, "HardSync", "register_module.notesrc.name.hardsync", [](){return mksp<HardSync>();});

	// 滤波器（Filter）分类 - 补充 lang 参数和翻译键
	String filtersn=lang.get("register_module.category.notesrc.filter");
	String filter="Filter";
	reg.registerModule(lang, filter, filtersn, "BiquadFilterSrc", "register_module.notesrc.name.biquad", [](){return mksp<BiquadFilterSrc>();});
	reg.registerModule(lang, filter, filtersn, "MeanFilterSrc", "register_module.notesrc.name.mean", [](){return mksp<MeanFilterSrc>();});

	// 鼓点（Drum）分类 - 补充 lang 参数和翻译键
	String drumsn=lang.get("register_module.category.notesrc.drum");
	String drum="Drum";
	reg.registerModule(lang, drum, drumsn, "SimpleDrumAmp", "register_module.notesrc.name.simple_drum_amp", [](){return mksp<SimpleDrumAmp>();});

	// 噪声源（Noise）分类 - 补充 lang 参数和翻译键
	String noisesn=lang.get("register_module.category.notesrc.noise");
	String noise="Noise";
	reg.registerModule(lang, noise, noisesn, "NoiseSrc", "register_module.notesrc.name.noise", [](){return mksp<NoiseSrc>();});
	reg.registerModule(lang, noise, noisesn, "LFSRNoise", "register_module.notesrc.name.lfsr", [](){return mksp<LFSRNoise>();});

	// 正弦波（Sine）分类 - 补充 lang 参数和翻译键
	String sinesn=lang.get("register_module.category.notesrc.sine");
	String sine="Sine";
	reg.registerModule(lang, sine, sinesn, "SineWave", "register_module.notesrc.name.sine_wave", [](){return mksp<SineWave>();});

	// 脉冲波（Pulse）分类 - 补充 lang 参数和翻译键
	String pulsesn=lang.get("register_module.category.notesrc.pulse");
	String pulse="Pulse";
	reg.registerModule(lang, pulse, pulsesn, "CymbalOsc", "register_module.notesrc.name.cymbal", [](){return mksp<CymbalOsc>();});
	reg.registerModule(lang, pulse, pulsesn, "VVVF", "register_module.notesrc.name.vvvf", [](){return mksp<VVVF>();});
	reg.registerModule(lang, pulse, pulsesn, "TriWave", "register_module.notesrc.name.tri_wave", [](){return mksp<TriWave>();});
	reg.registerModule(lang, pulse, pulsesn, "SawWave", "register_module.notesrc.name.saw_wave", [](){return mksp<SawWave>();});
	reg.registerModule(lang, pulse, pulsesn, "SquareWave", "register_module.notesrc.name.square_wave", [](){return mksp<SquareWave>();});
	reg.registerModule(lang, pulse, pulsesn, "Pulse", "register_module.notesrc.name.pulse", [](){return mksp<Pulse>();});

	// 物理建模（Physic）分类 - 补充 lang 参数和翻译键，统一参数顺序
	String physicsn=lang.get("register_module.category.notesrc.physic");
	String physic="Physic";
	reg.registerModule(lang, physic, physicsn, "KarplusStrongSrc", "register_module.notesrc.name.karplus_strong", [](){return mksp<KarplusStrongSrc>();});
	reg.registerModule(lang, physic, physicsn, "BowedString", "register_module.notesrc.name.bowed_string", [](){return mksp<BowedString>();});
	reg.registerModule(lang, physic, physicsn, "Sitar", "register_module.notesrc.name.sitar", [](){return mksp<Sitar>();});
	reg.registerModule(lang, physic, physicsn, "Sakura", "register_module.notesrc.name.sakura", [](){return mksp<Sakura>();}, sakuraRenderFunc);
	reg.registerModule(lang, physic, physicsn, "SakuraExciter", "register_module.notesrc.name.sakura_exciter", [](){return mksp<SakuraExciter>();});
	reg.registerModule(lang, physic, physicsn, "PianoSrc", "register_module.notesrc.name.piano_src", [](){return mksp<PianoSrc>();}, pianoRenderFunc);

	// 采样器（Sampler）分类 - 补充未完善模块的翻译键，统一参数顺序
	String samplersn=lang.get("register_module.category.notesrc.sampler");
	String sampler="Sampler";
	reg.registerModule(lang, sampler, samplersn, "WaveSampler", "register_module.notesrc.name.wave_sampler", [](){return mksp<WaveSampler>();});
	reg.registerModule(lang, sampler, samplersn, "SampleArrayProvider", "register_module.notesrc.name.sample_data", [](){return mksp<SampleArrayProvider>();}, sampleDataRenderFunc);

	// 包络器（Envelop）分类 - 补充 lang 参数和翻译键
	String envelopsn=lang.get("register_module.category.notesrc.envelop");
	String envelop="Envelop";
	reg.registerModule(lang, envelop, envelopsn, "EnvelopMultiplier", "register_module.notesrc.name.envelop_multiplier", [](){return mksp<EnvelopMultiplier>();});
	reg.registerModule(lang, envelop, envelopsn, "AHDSREnvelop", "register_module.notesrc.name.ahdsr", [](){return mksp<AHDSREnvelop>();});
	reg.registerModule(lang, envelop, envelopsn, "GraphEnvelop", "register_module.notesrc.name.graph", [](){return mksp<GraphEnvelop>();});
	reg.registerModule(lang, envelop, envelopsn, "TimeEnvelop", "register_module.notesrc.name.time", [](){return mksp<TimeEnvelop>();});
	reg.registerModule(lang, envelop, envelopsn, "MultiStageEnvelope", "register_module.notesrc.name.multi_stage_envelope", [](){return mksp<MultiStageEnvelope>();}, MultiStageEnvRenderFunc);

	// 非线性处理（NonLinear）分类 - 补充 lang 参数和翻译键
	String nonlinearsn=lang.get("register_module.category.notesrc.nonlinear");
	String nonlinear="NonLinear";
	reg.registerModule(lang, nonlinear, nonlinearsn, "ArctanDistortion", "register_module.notesrc.name.arctan", [](){return mksp<ArctanDistortion>();});
	reg.registerModule(lang, nonlinear, nonlinearsn, "ClampAmp", "register_module.notesrc.name.clamp", [](){return mksp<ClampAmp>();});
	reg.registerModule(lang, nonlinear, nonlinearsn, "ClampWithVelocityAmp", "register_module.notesrc.name.clamp_vel", [](){return mksp<ClampWithVelocityAmp>();});
	reg.registerModule(lang, nonlinear, nonlinearsn, "SoftClipAmp", "register_module.notesrc.name.soft_clip_amp", [](){return mksp<SoftClipAmp>();});
	reg.registerModule(lang, nonlinear, nonlinearsn, "TapeSaturationDistortion", "register_module.notesrc.name.tape_saturation_distortion", [](){return mksp<TapeSaturationDistortion>();});
	reg.registerModule(lang, nonlinear, nonlinearsn, "FoldbackDistortion", "register_module.notesrc.name.foldback_distortion", [](){return mksp<FoldbackDistortion>();});
	// 常量模块（Const）分类 - 补充 lang 参数和翻译键
	String constcsn=lang.get("register_module.category.notesrc.const");
	String constc="Const";
	reg.registerModule(lang, constc, constcsn, "ConstAmp", "register_module.notesrc.name.const_amp", [](){return mksp<_ConstAmp>();});
	reg.registerModule(lang, constc, constcsn, "NoteIDAmp", "register_module.notesrc.name.note_id_amp", [](){return mksp<_NoteIDAmp>();});
	reg.registerModule(lang, constc, constcsn, "NoteVelAmp", "register_module.notesrc.name.note_vel_amp", [](){return mksp<_NoteVelAmp>();});
	reg.registerModule(lang, constc, constcsn, "NoteFreqAmp", "register_module.notesrc.name.note_freq_amp", [](){return mksp<_NoteFreqAmp>();});
}

void registerAllDSP(Lang & lang, MenuRegister & reg){
	String filtersn=lang.get("register_module.category.dsp.filter");
	String filter="Filter";
	String delaysn=lang.get("register_module.category.dsp.delay");
	String delay="Delay";
	String dynamicsn=lang.get("register_module.category.dsp.dynamic");
	String dynamic="Dynamic";
	String cate="";

	reg.registerModule(lang, filter, filtersn, "AllPassFilter", "register_module.dsp.name.all_pass_filter", [](){return mksp<AllPassFilter>();});
	reg.registerModule(lang, filter, filtersn, "BiquadIIR", "register_module.dsp.name.biquad", [](){return mksp<BiquadIIR>();});
	reg.registerModule(lang, delay, delaysn, "Chorus", "register_module.dsp.name.chorus", [](){return mksp<Chorus>();});
	reg.registerModule(lang, dynamic, dynamicsn, "Compressor", "register_module.dsp.name.compressor", [](){return mksp<Compressor>();}, compressorRenderFunc);
	reg.registerModule(lang, delay, delaysn, "Delayer", "register_module.dsp.name.delayer", [](){return mksp<Delayer>();});
	reg.registerModule(lang, cate, cate, "DSPChain", "register_module.dsp.name.dsp_chain", [](){return mksp<DSPChain>();});
	reg.registerModule(lang, cate, cate, "DSPParallel", "register_module.dsp.name.dsp_parallel", [](){return mksp<DSPParallel>();});
	reg.registerModule(lang, dynamic, dynamicsn, "EnvelopDetector", "register_module.dsp.name.envelop_detector", [](){return mksp<EnvelopDetector>();}, envelopDetectorRenderFunc);
	reg.registerModule(lang, filter, filtersn, "FeedbackCombFilter", "register_module.dsp.name.feedback_comb_filter", [](){return mksp<FeedbackCombFilter>();});
	reg.registerModule(lang, filter, filtersn, "FeedforwardCombFilter", "register_module.dsp.name.feedforward_comb_filter", [](){return mksp<FeedforwardCombFilter>();});
	reg.registerModule(lang, filter, filtersn, "FIR", "register_module.dsp.name.fir", [](){return mksp<FIR>();});
	reg.registerModule(lang, delay, delaysn, "FDNReverb", "register_module.dsp.name.fdnreverb", [](){return mksp<FDNReverb>();});
	reg.registerModule(lang, delay, delaysn, "FDNReverb2", "register_module.dsp.name.fdnreverb2", [](){return mksp<FDNReverb2>();});
	reg.registerModule(lang, delay, delaysn, "Freeverb", "register_module.dsp.name.freeverb", [](){return mksp<Freeverb>();});
	reg.registerModule(lang, cate, cate, "HRIR", "register_module.dsp.name.hrir", [](){return mksp<HRIR>();});
	reg.registerModule(lang, filter, filtersn, "IIR", "register_module.dsp.name.iir", [](){return mksp<IIR>();});
	reg.registerModule(lang, filter, filtersn, "Integrator", "register_module.dsp.name.integrator", [](){return mksp<Integrator>();});
	reg.registerModule(lang, dynamic, dynamicsn, "Limiter", "register_module.dsp.name.limiter", [](){return mksp<Limiter>();});
	reg.registerModule(lang, filter, filtersn, "LowPassFeedbackCombFilter", "register_module.dsp.name.low_pass_feedback_comb_filter", [](){return mksp<LowPassFeedbackCombFilter>();});
	reg.registerModule(lang, dynamic, dynamicsn, "NoiseGate", "register_module.dsp.name.noise_gate", [](){return mksp<NoiseGate>();});
	reg.registerModule(lang, delay, delaysn, "Phaser", "register_module.dsp.name.phaser", [](){return mksp<Phaser>();});
	reg.registerModule(lang, delay, delaysn, "SchroederReverb", "register_module.dsp.name.schroeder_reverb", [](){return mksp<SchroederReverb>();});
	reg.registerModule(lang, delay, delaysn, "UnitDelay", "register_module.dsp.name.unit_delay", [](){return mksp<UnitDelay>();});
}
void registerAllInterpolator(Lang & lang, MenuRegister & reg){
	String cate="";
	reg.registerModule(lang, cate, cate, "PowInterpolator", "register_module.interpolator.name.pow", [](){return mksp<PowInterpolator>();});
	reg.registerModule(lang, cate, cate, "GraphInterpolator", "register_module.interpolator.name.graph", [](){return mksp<GraphInterpolator>();}, graphInterpRenderFunc);
	//reg.registerModule("", "Bezier", [](){return mksp<BezierInterpolator>();});
	reg.registerModule(lang, cate, cate, "LineInterpolator", "register_module.interpolator.name.linear", [](){return mksp<LineInterpolator>();});
}
void registerAllPhaseSrc(Lang & lang, MenuRegister & reg){
	String cate="";
	reg.registerModule(lang, cate, cate, "NotePhase", "register_module.phasesrc.name.note", [](){return mksp<_NotePhase>();});
	reg.registerModule(lang, cate, cate, "AddPhase", "register_module.phasesrc.name.add", [](){return mksp<_AddPhase>();});
	reg.registerModule(lang, cate, cate, "MulPhase", "register_module.phasesrc.name.mul", [](){return mksp<_MulPhase>();});
	reg.registerModule(lang, cate, cate, "ConstPhase", "register_module.phasesrc.name.const", [](){return mksp<_ConstPhase>();});
}