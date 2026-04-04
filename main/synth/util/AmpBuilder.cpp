#include "AmpBuilder.h"
#include "AmplitudeSources.h"
#include "dsp/DSP.h"
#include "dsp/IIR.h"
#include "dsp/BiquadIIR.h"
#include "dsp/IIRUtil.h"
#include "lang/Exception.h"
#include "synth/dsp/NoteDSP.h"
#include "synth/dsp/PostProcessDSP.h"
#include "synth/enveloper/AHDSREnvelop.h"
#include "synth/enveloper/EnvelopMultiplier.h"
#include "synth/enveloper/Enveloper.h"
#include "synth/enveloper/GraphEnvelop.h"
#include "synth/envfilter/BiquadEnvFilterSrc.h"
#include "synth/envfilter/MeanFilterSrc.h"
#include "synth/modulation/AutoMod.h"
#include "synth/modulation/NoteVelocityMix.h"
#include "synth/modulation/PhaseModAmp.h"
#include "synth/modulation/SimpleDrumAmp.h"
#include "synth/nonlinear/AmpQuantization.h"
#include "synth/nonlinear/ArctanDistortion.h"
#include "synth/nonlinear/ClampAmp.h"
#include "synth/nonlinear/ClampWithVelocityAmp.h"
#include "synth/operator/AmpAdder.h"
#include "synth/performance/AmpWithCC.h"
#include "synth/performance/MultiKeyTrigger.h"
#include "synth/performance/NoteShift.h"
#include "synth/performance/SimpleDetuner.h"
#include "synth/physic/KarplusStrongSrc.h"
#include "synth/util/EnvUtil.h"
#include "synth/util/AmplitudeSources.h"

using namespace yzrilyzr_util;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_array;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	AmpBuilder & AmpBuilder::am(NoteProcPtr amSrc, double amp, u_freq Hz){
		return mul(AmpBuilder(amSrc).freqSrc(Hz).mul(amp).add(1).build());
	}
	NoteProcPtr AmpBuilder::build(){
		return _src;
	}
	/**
	 * this += add
	 * 音符时长为 this->_src和add两者 中的最长
	 */
	AmpBuilder & AmpBuilder::add(double add1){
		return add(ConstAmp(add1));
	}
	/**
	 * this *= mul
	 * 音符时长为 this->_src和mul两者 中的最短
	 */
	AmpBuilder & AmpBuilder::mul(double mul1){
		return mul(ConstAmp(mul1));
	}
	AmpBuilder & AmpBuilder::velMix(s_note_vel ovrd, u_normal_01 mix){
		_src=mksp<NoteVelocityMix>(_src, ovrd, mix);
		return *this;
	}
	AmpBuilder & AmpBuilder::velMix(u_normal_01 mix){
		return velMix(1, mix);
	}
	AmpBuilder & AmpBuilder::freqSrc(double src){
		return freqSrc(ConstPhase(src));
	}
	/**
	 * this = src
	 */
	AmpBuilder & AmpBuilder::src(NoteProcPtr src){
		this->_src=src;
		return *this;
	}
	/**
	 * this *= mul
	 * 音符时长为 this->_src和mul两者 中的最短
	 */
	AmpBuilder & AmpBuilder::mul(NoteProcPtr mul1){
		if(U_INSTANCE_OF_PTR(Enveloper, this->_src)){
			this->_src=mksp<EnvelopMultiplier>(this->_src, mul1);
		} else if(U_INSTANCE_OF_PTR(Enveloper, mul1)){
			this->_src=mksp<EnvelopMultiplier>(mul1, this->_src);
		} else{
			this->_src=mksp<AmpMultiplier>(this->_src, mul1);
		}
		return *this;
	}
	AmpBuilder & AmpBuilder::multyKey(const IntArray & noteShift, const DoubleArray & velocityMul){
		this->_src=mksp<MultiKeyTrigger>(_src, noteShift, velocityMul);
		return *this;
	}
	AmpBuilder & AmpBuilder::autoMod(u_normal_01 modFreqDepth, u_normal_01 modAmpDepth, u_freq modRate, u_time modDelay, NoteProcPtr modShape){
		this->_src=mksp<AutoMod>(this->_src, modFreqDepth, modAmpDepth, modRate, modDelay, modShape);
		return *this;
	}
	/**
	 * this += add
	 * 音符时长为 this->_src和add两者 中的最长
	 */
	AmpBuilder & AmpBuilder::add(NoteProcPtr add){
		if(this->_src == nullptr){
			this->_src=add;
			return *this;
		}
		this->_src=mksp<AmpAdder>(this->_src, add);
		return *this;
	}
	AmpBuilder & AmpBuilder::freqSrc(u_sp<PhaseSrc> src){
		if(auto pmsrc=U_INSTANCE_OF_PTR(PhaseModAmp, _src)){
			pmsrc->setPhaseSource(src);
		} else{
			_src=mksp<PhaseModAmp>(_src, src);
		}
		return *this;
	}
	AmpBuilder & AmpBuilder::cc(const IntArray & cc){
		this->_src=mksp<AmpWithCC>(this->_src, cc);
		return *this;
	}
	NoteProcPtr cali(int samples){
		//return new ChannelVolumeCali(build(), samples);
		return nullptr;
	}
	/**
	 * this += src1 * mul
	 * 音符时长为 this->_src和mul两者 中的最短
	 */
	AmpBuilder & AmpBuilder::addMul(NoteProcPtr src1, double mul){
		this->_src=mksp<AmpAdder>(this->_src, mksp<AmpMultiplier>(src1, ConstAmp(mul)));
		return *this;
	}
	AmpBuilder & AmpBuilder::noteDSP(DSPPtr dsp){
		this->_src=mksp<NoteDSP>(this->_src, dsp);
		return *this;
	}
	AmpBuilder & AmpBuilder::postDSP(DSPPtr dsp){
		this->_src=mksp<PostProcessDSP>(this->_src, dsp);
		return *this;
	}
	/**
	 * this *= ( amp * (amSrc.freq=Hz) )
	 */
	AmpBuilder & AmpBuilder::rm(NoteProcPtr amSrc, double amp, u_freq Hz){
		return mul(AmpBuilder(amSrc).freqSrc(Hz).mul(amp).build());
	}
	AmpBuilder & AmpBuilder::src(double src){
		this->_src=ConstAmp(src);
		return *this;
	}
	/**
	 * this = clamp(src,x)
	 * 硬截幅
	 *
	 * @param clamp >= 0
	 */
	AmpBuilder & AmpBuilder::clamp(u_sample clamp1){
		return clamp(1, clamp1);
	}
	/**
	 * this = clamp(src,x*inputGain)
	 * 硬截幅
	 *
	 * @param clamp >= 0
	 */
	AmpBuilder & AmpBuilder::clamp(u_sample inputGain, u_sample clamp1){
		return clamp(inputGain, clamp1, 1);
	}
	AmpBuilder & AmpBuilder::clamp(u_sample inputGain, u_sample clamp1, u_sample outputGain){
		this->_src=mksp<ClampAmp>(_src, inputGain, clamp1, outputGain);
		return *this;
	}
	/**
	 * this = clampV(src,x*inputGain)
	 * 硬截幅，根据note的velocity进行截幅
	 *
	 * @param clamp >= 0
	 */
	AmpBuilder & AmpBuilder::clampV(u_sample inputGain, u_sample clamp){
		return clampV(inputGain, clamp, 1);
	}
	AmpBuilder & AmpBuilder::clampV(u_sample inputGain, u_sample clamp, u_sample outputGain){
		this->_src=mksp<ClampWithVelocityAmp>(_src, inputGain, clamp, outputGain);
		return *this;
	}
	AmpBuilder & AmpBuilder::arctanDistortion(u_sample inputGain, double alpha, u_sample outputGain){
		this->_src=mksp<ArctanDistortion>(_src, inputGain, alpha, outputGain);
		return *this;
	}
	/**
	 * this = IIR(src,x)
	 * IIR滤波
	 *
	 * @param n  阶数
	 * @param f1 Hz开始
	 * @param f2 Hz结束（高通低通填0即可）
	 */
	AmpBuilder & AmpBuilder::iir(yzrilyzr_dsp::ButterworthParams & params){
		auto iir=mksp<yzrilyzr_dsp::IIR>();
		params.fcf1=IIRUtil::limitFreq(params.sampleRate, params.fcf1);
		params.fcf2=IIRUtil::limitFreq(params.sampleRate, params.fcf2);
		IIRUtil::design_butterworth(iir->aCoeff, iir->bCoeff, params);
		iir->init(params.sampleRate);
		this->_src=mksp<NoteDSP>(_src, iir);
		return *this;
	}
	AmpBuilder & AmpBuilder::quantization(int32_t q){
		this->_src=mksp<AmpQuantization>(_src, q);
		return *this;
	}
	AmpBuilder & AmpBuilder::noteShift(int8_t shift){
		this->_src=mksp<NoteShift>(this->_src, shift);
		return *this;
	}
	/**
	 * this *= ADSR
	 * 音符时长仅由ADSR参数决定
	 */
	AmpBuilder &
		AmpBuilder::ADSR(u_time_ms attackTime, u_time_ms decayTime, u_normal_01 decayToVolume, bool canSustain, u_time_ms releaseTime, u_sp<Interpolator> aCurve, u_sp<Interpolator> dCurve,
						 u_sp<Interpolator> rCurve){
		return DAHDSR(0, attackTime, 1, decayTime, decayToVolume, canSustain, releaseTime, 100, aCurve, dCurve, rCurve);
	}
	AmpBuilder &
		AmpBuilder::AR(u_time_ms attackTime, u_time_ms releaseTime, u_sp<Interpolator> aCurve,
					   u_sp<Interpolator> rCurve){
		return DAHDSR(0, attackTime, 0.1, 0.1, 1, false, releaseTime, 100, aCurve, Line(), rCurve);
	}
	AmpBuilder &
		AmpBuilder::AR(u_time_ms attackTime, u_time_ms releaseTime, u_time_ms forceReleaseTime, u_sp<Interpolator> aCurve,
					   u_sp<Interpolator> rCurve){
		return DAHDSR(0, attackTime, 0.1, 0.1, 1, false, releaseTime, forceReleaseTime, aCurve, Line(), rCurve);
	}
	AmpBuilder & AmpBuilder::ADSR(u_time_ms attackTime, u_time_ms decayTime, u_normal_01 decayToVolume, bool canSustain, u_time_ms releaseTime, u_time_ms forceReleaseTime, u_sp<Interpolator> aCurve,
								  u_sp<Interpolator> dCurve, u_sp<Interpolator> rCurve){
		return  DAHDSR(0, attackTime, 1, decayTime, decayToVolume, canSustain, releaseTime, forceReleaseTime, aCurve, dCurve, rCurve);
	}
	AmpBuilder & AmpBuilder::AHDSR(u_time_ms attackTime, u_time_ms holdTime, u_time_ms decayTime, u_normal_01 decayToVolume, bool canSustain, u_time_ms releaseTime, u_sp<Interpolator> aCurve,
								   u_sp<Interpolator> dCurve, u_sp<Interpolator> rCurve){
		return DAHDSR(0, attackTime, holdTime, decayTime, decayToVolume, canSustain, releaseTime, 100, aCurve, dCurve, rCurve);
	}
	AmpBuilder &
		AmpBuilder::AHDSR(u_time_ms attackTime, u_time_ms holdTime, u_time_ms decayTime, u_normal_01 decayToVolume, bool canSustain, u_time_ms releaseTime, u_time_ms forceReleaseTime, u_sp<Interpolator> aCurve,
						  u_sp<Interpolator> dCurve, u_sp<Interpolator> rCurve){
		return DAHDSR(0, attackTime, holdTime, decayTime, decayToVolume, canSustain, releaseTime, forceReleaseTime, aCurve, dCurve, rCurve);
	}
	AmpBuilder & AmpBuilder::DAHDSR(u_time_ms delayTime, u_time_ms attackTime, u_time_ms holdTime, u_time_ms decayTime, u_normal_01 decayToVolume, bool canSustain, u_time_ms releaseTime, u_sp<Interpolator> aCurve,
									u_sp<Interpolator> dCurve, u_sp<Interpolator> rCurve){
		return DAHDSR(delayTime, attackTime, holdTime, decayTime, decayToVolume, canSustain, releaseTime, 100, aCurve, dCurve, rCurve);
	}
	AmpBuilder &
		AmpBuilder::DAHDSR(u_time_ms delayTime, u_time_ms attackTime, u_time_ms holdTime, u_time_ms decayTime, u_normal_01 decayToVolume, bool canSustain, u_time_ms releaseTime, u_time_ms forceReleaseTime, u_sp<Interpolator> aCurve,
						   u_sp<Interpolator> dCurve, u_sp<Interpolator> rCurve){
		return mul(mksp<AHDSREnvelop>(delayTime, attackTime, holdTime, decayTime, decayToVolume, canSustain, releaseTime, forceReleaseTime, aCurve, dCurve, rCurve));
	}
	AmpBuilder & AmpBuilder::GraphEnv(int32_t sustainPointIndex, const DoubleArray & pointValues){
		return mul(mksp<GraphEnvelop>(sustainPointIndex, pointValues));
	}
	AmpBuilder & AmpBuilder::MultiStageEnv(const std::vector<MSEPoint> & points){
		return mul(mksp<MultiStageEnvelope>(points));
	}
	AmpBuilder & AmpBuilder::pm(NoteProcPtr pmSrc, double amp, double noteRatio){
		if(!U_INSTANCE_OF_PTR(PhaseModAmp, _src)){
			_src=mksp<PhaseModAmp>(_src, NotePhase);
		}
		auto pmptr=spsc<PhaseModAmp>(this->_src);
		pmSrc=AmpBuilder(pmSrc).freqSrc(pmptr->getPhaseSource() * noteRatio).build();
		pmptr->pm(pmSrc, amp);
		return *this;
	}
	AmpBuilder & AmpBuilder::pm(NoteProcPtr pmSrc, double amp){
		if(!U_INSTANCE_OF_PTR(PhaseModAmp, _src)){
			_src=mksp<PhaseModAmp>(_src, NotePhase);
		}
		spsc<PhaseModAmp>(this->_src)->pm(pmSrc, amp);
		return *this;
	}
	AmpBuilder & AmpBuilder::lpm(NoteProcPtr pmSrc, double amp, u_freq lpmHz){
		if(!U_INSTANCE_OF_PTR(PhaseModAmp, _src)){
			_src=mksp<PhaseModAmp>(_src, NotePhase);
		}
		pmSrc=AmpBuilder(pmSrc).freqSrc(lpmHz).build();
		spsc<PhaseModAmp>(this->_src)->lpm(pmSrc, amp);
		return *this;
	}
	AmpBuilder & AmpBuilder::lpm(NoteProcPtr lpmSrc, double amp){
		if(!U_INSTANCE_OF_PTR(PhaseModAmp, _src)){
			_src=mksp<PhaseModAmp>(_src, NotePhase);
		}
		spsc<PhaseModAmp>(this->_src)->lpm(lpmSrc, amp);
		return *this;
	}
	AmpBuilder & AmpBuilder::ks(u_normal_01 alpha){
		this->_src=KarplusStrongBuilder().burst(_src).alpha(alpha).build();
		return *this;
	}
	AmpBuilder & AmpBuilder::ks(u_freq freq, u_normal_01 alpha){
		this->_src=KarplusStrongBuilder().constFreq(freq).alpha(alpha).burst(_src).build();
		return *this;
	}
	AmpBuilder & AmpBuilder::ks(u_sp<PhaseSrc> freqSrc, u_normal_01 alpha){
		this->_src=KarplusStrongBuilder().alpha(alpha).burst(_src).build();
		this->freqSrc(freqSrc);
		return *this;
	}
	AmpBuilder & AmpBuilder::drum(u_freq startFreq, u_freq endFreq, u_time duration){
		this->_src=mksp<SimpleDrumAmp>(this->_src, startFreq, endFreq, duration);
		return *this;
	}
	AmpBuilder & AmpBuilder::drum(u_freq startFreq, u_freq endFreq, u_time duration, int mode, u_sp<Interpolator> interpolator){
		this->_src=mksp<SimpleDrumAmp>(this->_src, startFreq, endFreq, duration, mode, interpolator);
		return *this;
	}
	AmpBuilder & AmpBuilder::biquadEnv(NoteProcPtr freqEnv, NoteProcPtr qEnv, FilterPassType type){
		this->_src=mksp<BiquadEnvFilterSrc>(this->_src, freqEnv, qEnv, type);
		return *this;
	}
	AmpBuilder & AmpBuilder::biquadEnvID(double idOffset, double q, FilterPassType type){
		return biquadEnv(AmpBuilder(NoteIDAmp).add(idOffset).build(), ConstAmp(q), type);
	}
	AmpBuilder & AmpBuilder::biquadEnvVel(s_note_id idMin, s_note_id idMax, double q, FilterPassType type){
		return biquadEnv(AmpBuilder(ConstAmp(idMax - idMin)).mul(NoteVelAmp).add(ConstAmp(idMin)).build(), ConstAmp(q), type);
	}
	AmpBuilder & AmpBuilder::biquad(yzrilyzr_dsp::RBJParams & params){
		auto iir=mksp<yzrilyzr_dsp::IIR>();
		params.f0=IIRUtil::limitFreq(params.sampleRate, params.f0);
		IIRUtil::RBJ_biquad(*iir, params);
		iir->init(params.sampleRate);
		this->_src=mksp<NoteDSP>(_src, iir);
		return *this;
	}
	AmpBuilder & AmpBuilder::biquadEnvGroup(int type, std::vector<BiquadEnvFilterGroupConfig> filters){
		this->_src=mksp<BiquadEnvFilterGroup>(_src, type, filters);
		return *this;
	}

	AmpBuilder & AmpBuilder::mean(NoteProcPtr pEnv, double pMul){
		this->_src=mksp<MeanFilterSrc>(this->_src, pEnv, pMul);
		return *this;
	}
	AmpBuilder & AmpBuilder::detune(int32_t count, s_note_id offset){
		this->_src=mksp<SimpleDetuner>(_src, count, offset);
		return *this;
	}
	NoteProcPtr AmpBuilder::parse(const String & str){
		/*String[] str1 = str.split("\n");
		for(String s: str1){
			ASTParser.parse(s);
		}
		ASTRoot root = ASTParser.parse(str);
		System.out.println(root);
		return nullptr;*/
		return nullptr;
	}
}