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
void registerAllNoteProcessor(Lang & lang, MenuRegister & reg){
	// 复合模块（Composed）分类 - 原有翻译键已补充，此处保持一致
	std::string composedsn=lang.getc("register_module.category.notesrc.composed");
	std::string composed="Composed";
	reg.registerModule(lang, composed, composedsn, "AmpAdder", "register_module.notesrc.name.add", [](){return std::make_shared<AmpAdder>();});
	reg.registerModule(lang, composed, composedsn, "AmpMultiplier", "register_module.notesrc.name.mul", [](){return std::make_shared<AmpMultiplier>();});
	reg.registerModule(lang, composed, composedsn, "FreqModAmp", "register_module.notesrc.name.freq_mod", [](){return std::make_shared<FreqModAmp>();});
	reg.registerModule(lang, composed, composedsn, "Matrix6x6Modulation", "register_module.notesrc.name.matrix_6x6_modulation", [](){return std::make_shared<Matrix6x6Modulation>();}, Matrix6x6ModulationRenderFunc, false);
	reg.registerModule(lang, composed, composedsn, "NoteDSP", "register_module.notesrc.name.note_dsp", [](){return std::make_shared<NoteDSP>();});
	reg.registerModule(lang, composed, composedsn, "PostProcessDSP", "register_module.notesrc.name.post_process_dsp", [](){return std::make_shared<PostProcessDSP>();});
	reg.registerModule(lang, composed, composedsn, "AmpWithCC", "register_module.notesrc.name.cc", [](){return std::make_shared<AmpWithCC>();});
	reg.registerModule(lang, composed, composedsn, "MultiKeyTrigger", "register_module.notesrc.name.multi_key_trigger", [](){return std::make_shared<MultiKeyTrigger>();});
	reg.registerModule(lang, composed, composedsn, "NoteShift", "register_module.notesrc.name.note_shift", [](){return std::make_shared<NoteShift>();});
	reg.registerModule(lang, composed, composedsn, "NoteVelocityMix", "register_module.notesrc.name.note_velocity_mix", [](){return std::make_shared<NoteVelocityMix>();});
	reg.registerModule(lang, composed, composedsn, "AmpQuantization", "register_module.notesrc.name.quantization", [](){return std::make_shared<AmpQuantization>();});
	reg.registerModule(lang, composed, composedsn, "AutoMod", "register_module.notesrc.name.automod", [](){return std::make_shared<AutoMod>();});
	reg.registerModule(lang, composed, composedsn, "SimpleDetuner", "register_module.notesrc.name.simpledetuner", [](){return std::make_shared<SimpleDetuner>();});
	reg.registerModule(lang, composed, composedsn, "SoftSync", "register_module.notesrc.name.softsync", [](){return std::make_shared<SoftSync>();});
	reg.registerModule(lang, composed, composedsn, "HardSync", "register_module.notesrc.name.hardsync", [](){return std::make_shared<HardSync>();});

	// 滤波器（Filter）分类 - 补充 lang 参数和翻译键
	std::string filtersn=lang.getc("register_module.category.notesrc.filter");
	std::string filter="Filter";
	reg.registerModule(lang, filter, filtersn, "BiquadFilterSrc", "register_module.notesrc.name.biquad", [](){return std::make_shared<BiquadFilterSrc>();});
	reg.registerModule(lang, filter, filtersn, "MeanFilterSrc", "register_module.notesrc.name.mean", [](){return std::make_shared<MeanFilterSrc>();});

	// 鼓点（Drum）分类 - 补充 lang 参数和翻译键
	std::string drumsn=lang.getc("register_module.category.notesrc.drum");
	std::string drum="Drum";
	reg.registerModule(lang, drum, drumsn, "SimpleDrumAmp", "register_module.notesrc.name.simple_drum_amp", [](){return std::make_shared<SimpleDrumAmp>();});

	// 噪声源（Noise）分类 - 补充 lang 参数和翻译键
	std::string noisesn=lang.getc("register_module.category.notesrc.noise");
	std::string noise="Noise";
	reg.registerModule(lang, noise, noisesn, "NoiseSrc", "register_module.notesrc.name.noise", [](){return std::make_shared<NoiseSrc>();});
	reg.registerModule(lang, noise, noisesn, "LFSRNoise", "register_module.notesrc.name.lfsr", [](){return std::make_shared<LFSRNoise>();});

	// 正弦波（Sine）分类 - 补充 lang 参数和翻译键
	std::string sinesn=lang.getc("register_module.category.notesrc.sine");
	std::string sine="Sine";
	reg.registerModule(lang, sine, sinesn, "SineWave", "register_module.notesrc.name.sine_wave", [](){return std::make_shared<SineWave>();});

	// 脉冲波（Pulse）分类 - 补充 lang 参数和翻译键
	std::string pulsesn=lang.getc("register_module.category.notesrc.pulse");
	std::string pulse="Pulse";
	reg.registerModule(lang, pulse, pulsesn, "CymbalOsc", "register_module.notesrc.name.cymbal", [](){return std::make_shared<CymbalOsc>();});
	reg.registerModule(lang, pulse, pulsesn, "VVVF", "register_module.notesrc.name.vvvf", [](){return std::make_shared<VVVF>();});
	reg.registerModule(lang, pulse, pulsesn, "TriWave", "register_module.notesrc.name.tri_wave", [](){return std::make_shared<TriWave>();});
	reg.registerModule(lang, pulse, pulsesn, "SawWave", "register_module.notesrc.name.saw_wave", [](){return std::make_shared<SawWave>();});
	reg.registerModule(lang, pulse, pulsesn, "SquareWave", "register_module.notesrc.name.square_wave", [](){return std::make_shared<SquareWave>();});
	reg.registerModule(lang, pulse, pulsesn, "Pulse", "register_module.notesrc.name.pulse", [](){return std::make_shared<Pulse>();});

	// 物理建模（Physic）分类 - 补充 lang 参数和翻译键，统一参数顺序
	std::string physicsn=lang.getc("register_module.category.notesrc.physic");
	std::string physic="Physic";
	reg.registerModule(lang, physic, physicsn, "KarplusStrongSrc", "register_module.notesrc.name.karplus_strong", [](){return std::make_shared<KarplusStrongSrc>();});
	reg.registerModule(lang, physic, physicsn, "BowedString", "register_module.notesrc.name.bowed_string", [](){return std::make_shared<BowedString>();});
	reg.registerModule(lang, physic, physicsn, "Sitar", "register_module.notesrc.name.sitar", [](){return std::make_shared<Sitar>();});
	reg.registerModule(lang, physic, physicsn, "Sakura", "register_module.notesrc.name.sakura", [](){return std::make_shared<Sakura>();}, sakuraRenderFunc);
	reg.registerModule(lang, physic, physicsn, "SakuraExciter", "register_module.notesrc.name.sakura_exciter", [](){return std::make_shared<SakuraExciter>();});
	reg.registerModule(lang, physic, physicsn, "PianoSrc", "register_module.notesrc.name.piano_src", [](){return std::make_shared<PianoSrc>();}, pianoRenderFunc);

	// 采样器（Sampler）分类 - 补充未完善模块的翻译键，统一参数顺序
	std::string samplersn=lang.getc("register_module.category.notesrc.sampler");
	std::string sampler="Sampler";
	reg.registerModule(lang, sampler, samplersn, "WaveSampler", "register_module.notesrc.name.wave_sampler", [](){return std::make_shared<WaveSampler>();});
	reg.registerModule(lang, sampler, samplersn, "SampleArrayProvider", "register_module.notesrc.name.sample_data", [](){return std::make_shared<SampleArrayProvider>();}, sampleDataRenderFunc);

	// 包络器（Envelop）分类 - 补充 lang 参数和翻译键
	std::string envelopsn=lang.getc("register_module.category.notesrc.envelop");
	std::string envelop="Envelop";
	reg.registerModule(lang, envelop, envelopsn, "EnvelopMultiplier", "register_module.notesrc.name.envelop_multiplier", [](){return std::make_shared<EnvelopMultiplier>();});
	reg.registerModule(lang, envelop, envelopsn, "AHDSREnvelop", "register_module.notesrc.name.ahdsr", [](){return std::make_shared<AHDSREnvelop>();});
	reg.registerModule(lang, envelop, envelopsn, "GraphEnvelop", "register_module.notesrc.name.graph", [](){return std::make_shared<GraphEnvelop>();});
	reg.registerModule(lang, envelop, envelopsn, "TimeEnvelop", "register_module.notesrc.name.time", [](){return std::make_shared<TimeEnvelop>();});
	reg.registerModule(lang, envelop, envelopsn, "MultiStageEnvelope", "register_module.notesrc.name.multi_stage_envelope", [](){return std::make_shared<MultiStageEnvelope>();}, MultiStageEnvRenderFunc);

	// 非线性处理（NonLinear）分类 - 补充 lang 参数和翻译键
	std::string nonlinearsn=lang.getc("register_module.category.notesrc.nonlinear");
	std::string nonlinear="NonLinear";
	reg.registerModule(lang, nonlinear, nonlinearsn, "ArctanDistortion", "register_module.notesrc.name.arctan", [](){return std::make_shared<ArctanDistortion>();});
	reg.registerModule(lang, nonlinear, nonlinearsn, "ClampAmp", "register_module.notesrc.name.clamp", [](){return std::make_shared<ClampAmp>();});
	reg.registerModule(lang, nonlinear, nonlinearsn, "ClampWithVelocityAmp", "register_module.notesrc.name.clamp_vel", [](){return std::make_shared<ClampWithVelocityAmp>();});
	reg.registerModule(lang, nonlinear, nonlinearsn, "SoftClipAmp", "register_module.notesrc.name.soft_clip_amp", [](){return std::make_shared<SoftClipAmp>();});
	reg.registerModule(lang, nonlinear, nonlinearsn, "TapeSaturationDistortion", "register_module.notesrc.name.tape_saturation_distortion", [](){return std::make_shared<TapeSaturationDistortion>();});
	reg.registerModule(lang, nonlinear, nonlinearsn, "FoldbackDistortion", "register_module.notesrc.name.foldback_distortion", [](){return std::make_shared<FoldbackDistortion>();});
	// 常量模块（Const）分类 - 补充 lang 参数和翻译键
	std::string constcsn=lang.getc("register_module.category.notesrc.const");
	std::string constc="Const";
	reg.registerModule(lang, constc, constcsn, "ConstAmp", "register_module.notesrc.name.const_amp", [](){return std::make_shared<_ConstAmp>();});
	reg.registerModule(lang, constc, constcsn, "NoteIDAmp", "register_module.notesrc.name.note_id_amp", [](){return std::make_shared<_NoteIDAmp>();});
	reg.registerModule(lang, constc, constcsn, "NoteVelAmp", "register_module.notesrc.name.note_id_amp", [](){return std::make_shared<_NoteVelAmp>();});
	reg.registerModule(lang, constc, constcsn, "NoteFreqAmp", "register_module.notesrc.name.note_id_amp", [](){return std::make_shared<_NoteFreqAmp>();});
}

void registerAllDSP(Lang & lang, MenuRegister & reg){
	std::string filtersn=lang.getc("register_module.category.dsp.filter");
	std::string filter="Filter";
	std::string delaysn=lang.getc("register_module.category.dsp.delay");
	std::string delay="Delay";
	std::string dynamicsn=lang.getc("register_module.category.dsp.dynamic");
	std::string dynamic="Dynamic";
	std::string cate="";

	reg.registerModule(lang, filter, filtersn, "AllPassFilter", "register_module.dsp.name.all_pass_filter", [](){return std::make_shared<AllPassFilter>();});
	reg.registerModule(lang, filter, filtersn, "BiquadIIR", "register_module.dsp.name.biquad", [](){return std::make_shared<BiquadIIR>();});
	reg.registerModule(lang, delay, delaysn, "Chorus", "register_module.dsp.name.chorus", [](){return std::make_shared<Chorus>();});
	reg.registerModule(lang, dynamic, dynamicsn, "Compressor", "register_module.dsp.name.compressor", [](){return std::make_shared<Compressor>();}, compressorRenderFunc);
	reg.registerModule(lang, delay, delaysn, "Delayer", "register_module.dsp.name.delayer", [](){return std::make_shared<Delayer>();});
	reg.registerModule(lang, cate, cate, "DSPChain", "register_module.dsp.name.dsp_chain", [](){return std::make_shared<DSPChain>();});
	reg.registerModule(lang, cate, cate, "DSPParallel", "register_module.dsp.name.dsp_parallel", [](){return std::make_shared<DSPParallel>();});
	reg.registerModule(lang, dynamic, dynamicsn, "EnvelopDetector", "register_module.dsp.name.envelop_detector", [](){return std::make_shared<EnvelopDetector>();}, envelopDetectorRenderFunc);
	reg.registerModule(lang, filter, filtersn, "FeedbackCombFilter", "register_module.dsp.name.feedback_comb_filter", [](){return std::make_shared<FeedbackCombFilter>();});
	reg.registerModule(lang, filter, filtersn, "FeedforwardCombFilter", "register_module.dsp.name.feedforward_comb_filter", [](){return std::make_shared<FeedforwardCombFilter>();});
	reg.registerModule(lang, filter, filtersn, "FIR", "register_module.dsp.name.fir", [](){return std::make_shared<FIR>();});
	reg.registerModule(lang, delay, delaysn, "FDNReverb", "register_module.dsp.name.fdnreverb", [](){return std::make_shared<FDNReverb>();});
	reg.registerModule(lang, delay, delaysn, "FDNReverb2", "register_module.dsp.name.fdnreverb2", [](){return std::make_shared<FDNReverb2>();});
	reg.registerModule(lang, delay, delaysn, "Freeverb", "register_module.dsp.name.freeverb", [](){return std::make_shared<Freeverb>();});
	reg.registerModule(lang, cate, cate, "HRIR", "register_module.dsp.name.hrir", [](){return std::make_shared<HRIR>();});
	reg.registerModule(lang, filter, filtersn, "IIR", "register_module.dsp.name.iir", [](){return std::make_shared<IIR>();});
	reg.registerModule(lang, filter, filtersn, "Integrator", "register_module.dsp.name.integrator", [](){return std::make_shared<Integrator>();});
	reg.registerModule(lang, dynamic, dynamicsn, "Limiter", "register_module.dsp.name.limiter", [](){return std::make_shared<Limiter>();});
	reg.registerModule(lang, filter, filtersn, "LowPassFeedbackCombFilter", "register_module.dsp.name.low_pass_feedback_comb_filter", [](){return std::make_shared<LowPassFeedbackCombFilter>();});
	reg.registerModule(lang, dynamic, dynamicsn, "NoiseGate", "register_module.dsp.name.noise_gate", [](){return std::make_shared<NoiseGate>();});
	reg.registerModule(lang, delay, delaysn, "Phaser", "register_module.dsp.name.phaser", [](){return std::make_shared<Phaser>();});
	reg.registerModule(lang, delay, delaysn, "SchroederReverb", "register_module.dsp.name.schroeder_reverb", [](){return std::make_shared<SchroederReverb>();});
	reg.registerModule(lang, delay, delaysn, "UnitDelay", "register_module.dsp.name.unit_delay", [](){return std::make_shared<UnitDelay>();});
}
void registerAllInterpolator(Lang & lang, MenuRegister & reg){
	std::string cate="";
	reg.registerModule(lang, cate, cate, "PowInterpolator", "register_module.interpolator.name.pow", [](){return std::make_shared<PowInterpolator>();});
	reg.registerModule(lang, cate, cate, "GraphInterpolator", "register_module.interpolator.name.graph", [](){return std::make_shared<GraphInterpolator>();}, graphInterpRenderFunc);
	//reg.registerModule("", "Bezier", [](){return std::make_shared<BezierInterpolator>();});
	reg.registerModule(lang, cate, cate, "LineInterpolator", "register_module.interpolator.name.linear", [](){return std::make_shared<LineInterpolator>();});
}
void registerAllPhaseSrc(Lang & lang, MenuRegister & reg){
	std::string cate="";
	reg.registerModule(lang, cate, cate, "NotePhase", "register_module.phasesrc.name.note", [](){return std::make_shared<_NotePhase>();});
	reg.registerModule(lang, cate, cate, "AddPhase", "register_module.phasesrc.name.add", [](){return std::make_shared<_AddPhase>();});
	reg.registerModule(lang, cate, cate, "MulPhase", "register_module.phasesrc.name.mul", [](){return std::make_shared<_MulPhase>();});
	reg.registerModule(lang, cate, cate, "ConstPhase", "register_module.phasesrc.name.const", [](){return std::make_shared<_ConstPhase>();});
}