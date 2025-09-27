#include "AmpBuilder.h"
#include "AmplitudeSources.h"
#include "dsp/DSP.h"
#include "dsp/IIR.h"
#include "dsp/IIRUtil.h"
#include "lang/Exception.h"
#include "synth/composed/AmpAdder.h"
#include "synth/composed/AmpQuantization.h"
#include "synth/composed/AmpWithCC.h"
#include "synth/composed/AutoMod.h"
#include "synth/composed/MultiKeyTrigger.h"
#include "synth/composed/NoteDSP.h"
#include "synth/composed/NoteShift.h"
#include "synth/composed/NoteVelocityMix.h"
#include "synth/composed/PostProcessDSP.h"
#include "synth/composed/SimpleDetuner.h"
#include "synth/envelopers/AHDSREnvelop.h"
#include "synth/envelopers/EnvelopMultiplier.h"
#include "synth/envelopers/Enveloper.h"
#include "synth/envelopers/GraphEnvelop.h"
#include "synth/filters/BiquadFilterSrc.h"
#include "synth/filters/MeanFilterSrc.h"
#include "synth/generators/drum/SimpleDrumAmp.h"
#include "synth/generators/physic/KarplusStrongSrc.h"
#include "synth/nonlinear/ArctanDistortion.h"
#include "synth/nonlinear/ClampAmp.h"
#include "synth/nonlinear/ClampWithVelocityAmp.h"

using namespace yzrilyzr_util;
using namespace yzrilyzr_interpolator;
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_array;
using namespace yzrilyzr_lang;
namespace yzrilyzr_simplesynth{
	AmpBuilder & AmpBuilder::am(std::shared_ptr<Osc> amSrc, double amp, u_freq Hz){
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
		_src= std::make_shared<NoteVelocityMix>(_src, ovrd, mix);
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
		if(std::dynamic_pointer_cast<Enveloper>(this->_src)) this->_src=std::make_shared<EnvelopMultiplier>(this->_src, mul1);
		else if(std::dynamic_pointer_cast<Enveloper>(mul1)) this->_src=std::make_shared<EnvelopMultiplier>(mul1, this->_src);
		else this->_src=std::make_shared<AmpMultiplier>(this->_src, mul1);
		return *this;
	}
	AmpBuilder & AmpBuilder::multyKey(std::shared_ptr<IntArray> noteShift, std::shared_ptr<yzrilyzr_array::DoubleArray> velocityMul){
		this->_src=std::make_shared<MultiKeyTrigger>(_src, noteShift, velocityMul);
		return *this;
	}
	AmpBuilder & AmpBuilder::autoMod(u_normal_01 modFreqDepth, u_normal_01 modAmpDepth, u_freq modRate, u_time modDelay, NoteProcPtr modShape){
		this->_src=std::make_shared<AutoMod>(this->_src, modFreqDepth, modAmpDepth, modRate, modDelay, modShape);
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
		this->_src=std::make_shared<AmpAdder>(this->_src, add);
		return *this;
	}
	AmpBuilder & AmpBuilder::freqSrc(std::shared_ptr<PhaseSrc> src){
		if(!(std::dynamic_pointer_cast<Osc>(this->_src)))
			throw Exception("this->_src not FreqBasedGenerator");
		std::dynamic_pointer_cast<Osc>(this->_src)->setPhaseSource(src);
		return *this;
	}
	AmpBuilder & AmpBuilder::cc(std::shared_ptr<IntArray> cc){
		this->_src=std::make_shared<AmpWithCC>(this->_src, cc);
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
		this->_src=std::make_shared<AmpAdder>(this->_src, std::make_shared<AmpMultiplier>(src1, ConstAmp(mul)));
		return *this;
	}
	AmpBuilder & AmpBuilder::noteDSP(yzrilyzr_dsp::DSPPtr dsp){
		this->_src=std::make_shared<NoteDSP>(this->_src, dsp);
		return *this;
	}
	AmpBuilder & AmpBuilder::postDSP(yzrilyzr_dsp::DSPPtr dsp){
		this->_src=std::make_shared<PostProcessDSP>(this->_src, dsp);
		return *this;
	}
	/**
	 * this *= ( amp * (amSrc.freq=Hz) )
	 */
	AmpBuilder & AmpBuilder::rm(std::shared_ptr<Osc> amSrc, double amp, u_freq Hz){
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
		this->_src=std::make_shared<ClampAmp>(_src, inputGain, clamp1, outputGain);
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
		this->_src=std::make_shared<ClampWithVelocityAmp>(_src, inputGain, clamp, outputGain);
		return *this;
	}
	AmpBuilder & AmpBuilder::arctanDistortion(u_sample inputGain, double alpha, u_sample outputGain){
		this->_src=std::make_shared<ArctanDistortion>(_src, inputGain, alpha, outputGain);
		return *this;
	}
	/**
	 * this = IIR(src,x)
	 * IIR滤波(带通)
	 *
	 * @param n  阶数(建议先测试，否则容易振荡)
	 * @param f1 Hz开始
	 * @param f2 Hz结束（高通低通填0即可）
	 */
	AmpBuilder & AmpBuilder::IIR(u_sample_rate sampleRate, uint16_t n, u_freq f1, u_freq f2){
		return IIR(sampleRate, FilterPassType::BANDPASS, n, f1, f2);
	}
	/**
	 * this = IIR(src,x)
	 * IIR滤波
	 *
	 * @param n  阶数
	 * @param f1 Hz开始
	 * @param f2 Hz结束（高通低通填0即可）
	 */
	AmpBuilder & AmpBuilder::IIR(u_sample_rate sampleRate, FilterPassType type, uint16_t n, u_freq f1, u_freq f2){
		this->_src=std::make_shared<NoteDSP>(_src, IIRUtil::newButterworthIIRFilter(sampleRate, type, n, IIRUtil::limitFreq(sampleRate, f1), IIRUtil::limitFreq(sampleRate, f2)));
		return *this;
	}
	AmpBuilder & AmpBuilder::quantization(int32_t q){
		this->_src=std::make_shared<AmpQuantization>(_src, q);
		return *this;
	}
	AmpBuilder & AmpBuilder::noteShift(int8_t shift){
		this->_src=std::make_shared<NoteShift>(this->_src, shift);
		return *this;
	}
	/**
	 * this *= ADSR
	 * 音符时长仅由ADSR参数决定
	 */
	AmpBuilder &
		AmpBuilder::ADSR(u_time_ms attackTime, u_time_ms decayTime, u_normal_01 decayToVolume, bool canSustain, u_time_ms releaseTime, std::shared_ptr<Interpolator> aCurve, std::shared_ptr<Interpolator> dCurve,
						 std::shared_ptr<Interpolator> rCurve){
		return DAHDSR(0, attackTime, 1, decayTime, decayToVolume, canSustain, releaseTime, 100, aCurve, dCurve, rCurve);
	}
	AmpBuilder & AmpBuilder::ADSR(u_time_ms attackTime, u_time_ms decayTime, u_normal_01 decayToVolume, bool canSustain, u_time_ms releaseTime, u_time_ms forceReleaseTime, std::shared_ptr<Interpolator> aCurve,
								  std::shared_ptr<Interpolator> dCurve, std::shared_ptr<Interpolator> rCurve){
		return  DAHDSR(0, attackTime, 1, decayTime, decayToVolume, canSustain, releaseTime, forceReleaseTime, aCurve, dCurve, rCurve);
	}
	AmpBuilder & AmpBuilder::AHDSR(u_time_ms attackTime, u_time_ms holdTime, u_time_ms decayTime, u_normal_01 decayToVolume, bool canSustain, u_time_ms releaseTime, std::shared_ptr<Interpolator> aCurve,
								   std::shared_ptr<Interpolator> dCurve, std::shared_ptr<Interpolator> rCurve){
		return DAHDSR(0, attackTime, holdTime, decayTime, decayToVolume, canSustain, releaseTime, 100, aCurve, dCurve, rCurve);
	}
	AmpBuilder &
		AmpBuilder::AHDSR(u_time_ms attackTime, u_time_ms holdTime, u_time_ms decayTime, u_normal_01 decayToVolume, bool canSustain, u_time_ms releaseTime, u_time_ms forceReleaseTime, std::shared_ptr<Interpolator> aCurve,
						  std::shared_ptr<Interpolator> dCurve, std::shared_ptr<Interpolator> rCurve){
		return DAHDSR(0, attackTime, holdTime, decayTime, decayToVolume, canSustain, releaseTime, forceReleaseTime, aCurve, dCurve, rCurve);
	}
	AmpBuilder & AmpBuilder::DAHDSR(u_time_ms delayTime, u_time_ms attackTime, u_time_ms holdTime, u_time_ms decayTime, u_normal_01 decayToVolume, bool canSustain, u_time_ms releaseTime, std::shared_ptr<Interpolator> aCurve,
									std::shared_ptr<Interpolator> dCurve, std::shared_ptr<Interpolator> rCurve){
		return DAHDSR(delayTime, attackTime, holdTime, decayTime, decayToVolume, canSustain, releaseTime, 100, aCurve, dCurve, rCurve);
	}
	AmpBuilder &
		AmpBuilder::DAHDSR(u_time_ms delayTime, u_time_ms attackTime, u_time_ms holdTime, u_time_ms decayTime, u_normal_01 decayToVolume, bool canSustain, u_time_ms releaseTime, u_time_ms forceReleaseTime, std::shared_ptr<Interpolator> aCurve,
						   std::shared_ptr<Interpolator> dCurve, std::shared_ptr<Interpolator> rCurve){
		return mul(std::make_shared<AHDSREnvelop>(delayTime, attackTime, holdTime, decayTime, decayToVolume, canSustain, releaseTime, forceReleaseTime, aCurve, dCurve, rCurve));
	}
	AmpBuilder & AmpBuilder::GraphEnv(int32_t sustainPointIndex, DoubleArray * pointValues){
		return mul(std::make_shared<GraphEnvelop>(sustainPointIndex, pointValues));
	}
	AmpBuilder & AmpBuilder::MultiStageEnv(const std::vector<MSEPoint> &points){
		return mul(std::make_shared<MultiStageEnvelope>(points));
	}
	AmpBuilder & AmpBuilder::pm(std::shared_ptr<Osc> pmSrc, double amp, double noteRatio){
		std::dynamic_pointer_cast<Osc>(this->_src)->pm(pmSrc, amp, noteRatio);
		return *this;
	}
	AmpBuilder & AmpBuilder::pm(NoteProcPtr pmSrc, double amp){
		std::dynamic_pointer_cast<Osc>(this->_src)->pm(pmSrc, amp);
		return *this;
	}
	AmpBuilder & AmpBuilder::lpm(std::shared_ptr<Osc> pmSrc, double amp, u_freq lpmHz){
		std::dynamic_pointer_cast<Osc>(this->_src)->lpm(pmSrc, amp, lpmHz);
		return *this;
	}
	AmpBuilder & AmpBuilder::lpm(NoteProcPtr lpmSrc, double amp){
		std::dynamic_pointer_cast<Osc>(this->_src)->lpm(lpmSrc, amp);
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
	AmpBuilder & AmpBuilder::ks(std::shared_ptr<PhaseSrc> freqSrc, u_normal_01 alpha){
		this->_src=KarplusStrongBuilder().freqSrc(freqSrc).alpha(alpha).burst(_src).build();
		return *this;
	}
	AmpBuilder & AmpBuilder::drum(u_freq startFreq, u_freq endFreq, u_time duration){
		this->_src=std::make_shared<SimpleDrumAmp>(this->_src, startFreq, endFreq, duration);
		return *this;
	}
	AmpBuilder & AmpBuilder::drum(u_freq startFreq, u_freq endFreq, u_time duration, int mode, std::shared_ptr<Interpolator> interpolator){
		this->_src=std::make_shared<SimpleDrumAmp>(this->_src, startFreq, endFreq, duration, mode, interpolator);
		return *this;
	}
	AmpBuilder & AmpBuilder::biquadEnv(NoteProcPtr freqEnv, NoteProcPtr qEnv, FilterPassType type){
		this->_src=std::make_shared<BiquadFilterSrc>(this->_src, freqEnv, qEnv, type);
		return *this;
	}
	AmpBuilder & AmpBuilder::biquadEnvID(double idOffset, double q, FilterPassType type){
		return biquadEnv(AmpBuilder(NoteIDAmp).add(idOffset).build(), ConstAmp(q), type);
	}
	AmpBuilder & AmpBuilder::biquadEnvVel(s_note_id idMin, s_note_id idMax, double q, yzrilyzr_dsp::FilterPassType type){
		return biquadEnv(AmpBuilder(ConstAmp(idMax - idMin)).mul(NoteVelAmp).add(ConstAmp(idMin)).build(), ConstAmp(q), type);
	}
	AmpBuilder & AmpBuilder::biquad(u_sample_rate sampleRate, FilterPassType type, u_freq f1, double q, double gain){
		this->_src=std::make_shared<NoteDSP>(_src, IIRUtil::newBiquadIIRFilter(IIRUtil::limitFreq(sampleRate, f1), sampleRate, q, gain, type));
		return *this;
	}
	AmpBuilder & AmpBuilder::biquadEnvGroup(int type, std::vector<BiquadEnvFilterGroupConfig> filters){
		this->_src=std::make_shared<BiquadEnvFilterGroup>(_src, type, filters);
		return *this;
	}

	AmpBuilder & AmpBuilder::mean(NoteProcPtr pEnv, double pMul){
		this->_src=std::make_shared<MeanFilterSrc>(this->_src, pEnv, pMul);
		return *this;
	}
	AmpBuilder & AmpBuilder::detune(int32_t count, s_note_id offset){
		this->_src=std::make_shared<SimpleDetuner>(_src, count, offset);
		return *this;
	}
	NoteProcPtr AmpBuilder::parse(const std::string & str){
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