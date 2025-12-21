#include "BanksHammer.h"
#include "PianoModel.h"
#include "StulovHammer.h"
#include "dsp/BiquadIIR.h"
#include "dsp/IIR.h"
#include "dsp/IIRUtil.h"
#include "synth/generators/physic/dwg/DigitalWaveGuide.h"
#include "util/Util.h"
#include "lang/Math.h"
using namespace yzrilyzr_dsp;
using namespace yzrilyzr_util;
using namespace yzrilyzr_lang;
using namespace yzrilyzr_array;
namespace yzrilyzr_simplesynth{
	/**
 * 初始化钢琴键的物理参数和模型组件
 * @param key 要初始化的钢琴键对象引用
 * @param param 钢琴键的参数配置
 */
	void PianoModel::PianoKeyInitialize(PianoKey & key, PianoKeyParameters & param){
		// 基础频率 (A0 = 27.5 Hz)
		u_sample baseFrequency=27.5;
		// 弦的失谐因子数组 (用于多弦钢琴键)
		u_sample detuneOffset[3]={0, 1, -1};
		// 保存力度权重参数
		key.weight=param.weight;
		// MIDI音符编号
		u_sample midiNoteNumber=param.noteID;
		// 计算线性映射值，用于后续的sigmoid函数
		u_sample sigmoidInputL=Util::linearMap(21.0, 108.0, param.ampLl, param.ampLr, midiNoteNumber);
		u_sample sigmoidInputR=Util::linearMap(21.0, 108.0, param.amprl, param.amprr, midiNoteNumber);
		// 计算弦长 (米) - 使用sigmoid函数映射
		u_sample stringLength=Util::sigmoid(sigmoidInputL, param.minL, param.maxL, param.ampLl, param.ampLr);
		// 计算弦半径 (米) - 使用sigmoid函数映射并转换单位
		u_sample stringRadius=Util::sigmoid(sigmoidInputR, param.minr, param.maxr, param.amprl, param.amprr) * 0.001;
		// 钢的密度 (kg/m³)
		u_sample stringDensity=7850.0 * param.mult_density_string;
		// 频率比例 - 用于计算锤子和琴弦参数
		u_sample frequencyRatio=Util::clamp01(log(param.frequency / baseFrequency) / log(4192.0 / baseFrequency));
		// 计算锤子质量 (kg)
		u_sample hammerMass=(0.06 - 0.058 * pow(frequencyRatio, 0.1)) * param.mult_mass_hammer;
		// 计算锤子滞后系数
		u_sample hammerHysteresis=0.00001 * frequencyRatio * param.mult_hysteresis_hammer;
		// 计算刚度指数
		u_sample stiffnessExponent=(2.0 + 1.0 * frequencyRatio) * param.mult_stiffness_exponent_hammer;
		// 计算锤子力度系数
		u_sample hammerForce=40.0 / pow(0.0007, stiffnessExponent) * param.mult_force_hammer;
		// 计算琴桥和声锤的阻抗
		key.ZBridge=4000.0 * param.mult_impedance_bridge;
		key.ZHammer=1.0 * param.mult_impedance_hammer;
		// 计算弦芯半径 (米)
		u_sample coreRadius=Math::min(stringRadius, static_cast<u_sample>(0.0006)) * param.mult_radius_core_string;
		// 钢的杨氏模量 (Pa)
		u_sample youngsModulus=2e11 * param.mult_modulus_string;
		// 锤子击打位置比例
		u_sample hammerPosition=param.position_hammer;
		// 根据频率确定弦的数量
		int stringCount;
		if(param.frequency < (48.9994278))
			stringCount=1;
		else if(param.frequency < (87.3070602))
			stringCount=2;
		else
			stringCount=3;

		// 初始化滤波器系数
		u_sample lossFilterC1=0.25;
		u_sample lossFilterC3=5.85 * param.mult_loss_filter;
		// 计算线密度 (kg/m)
		u_sample linearMassDensity=Math::PI * stringRadius * stringRadius * stringDensity;
		// 计算弦张力 (N)
		u_sample stringTension=(2.0 * stringLength * param.frequency) * (2.0 * stringLength * param.frequency) * linearMassDensity;
		// 计算弦的特征阻抗 (kg/s)
		key.Z=sqrt(1.0 * stringTension * linearMassDensity);
		// 计算色散因子 (用于模拟高频衰减)
		u_sample dispersionFactor=(Math::PI * Math::PI * Math::PI) * youngsModulus * coreRadius * coreRadius * coreRadius * coreRadius / (4.0 * stringLength * stringLength * stringTension);
		// 创建新的弦数组
		key.string.clear();
		// 初始化每根弦的数字波导模型
		for(u_index k=0; k < stringCount; k++){
			u_sp<PianoDwgs> pianoDWGs=mksp<PianoDwgs>();
			key.string.push_back(pianoDWGs);
			// 初始化弦的物理模型，应用失谐因子
			Piano_initString(*pianoDWGs,
							 param.frequency + param.frequency * detuneOffset[k] * param.detune,
							 param.sampleRate,
							 hammerPosition,
							 lossFilterC1,
							 lossFilterC3,
							 dispersionFactor,
							 key.Z,
							 key.ZBridge + (stringCount - 1) * key.Z,
							 key.ZHammer);
		}
		// 根据配置选择并初始化锤子模型
		switch(param.hammer_type){
			default:
			case 1:
				key.hammer=mksp<StulovHammer>();
				break;
			case 2:
				key.hammer=mksp<BanksHammer>();
				break;
		}
		// 初始化锤子的物理参数
		key.hammer->init(param.sampleRate, hammerMass, hammerForce, stiffnessExponent, key.Z, hammerHysteresis);
	}
	void PianoModel::Piano_initString(PianoDwgs & pianoDWGs, u_sample freq, u_sample sampleRate, u_sample hammerPos, u_sample c1, u_sample c3, u_sample dispersionFactor, u_sample Z, u_sample Zb, u_sample Zh){
		std::vector<u_sp<DSP>> dispersion;
		u_sample dispersionDelay=0;
		int dispersionOrder=0;
		if(freq > 400)dispersionOrder=1;
		else dispersionOrder=4;
		for(u_index m=0;m < dispersionOrder;m++){
			u_sample targetDelay=IIRUtil::calculateThiranDelay(dispersionFactor, freq, dispersionOrder);
			targetDelay=Math::max(targetDelay, static_cast<u_sample>(1.0));
			dispersionDelay+=targetDelay;
			u_sp<BiquadIIR> f=mksp<BiquadIIR>();
			IIRUtil::designThiranFilter(f->aCoeff, f->bCoeff, targetDelay, 2);
			dispersion.push_back(f);
		}
		u_sp<IIR> lowpass=IIRUtil::newC1C3IIRFilter(freq, c1, c3);
		u_sample lowpassDelay=IIRUtil::groupDelay(*lowpass, freq, sampleRate);
		u_sample totalDelay=sampleRate / freq;
		u_sample del1=hammerPos * 0.5 * totalDelay;
		u_sample del2=0.5 * (totalDelay * (1.0 - hammerPos)) - dispersionDelay;
		u_sample del3=1;
		if(del1 < 2) del1=1;
		if(del2 < 2) del2=1;
		u_sample D=totalDelay - (del1 + del1 + del2 + del3 + dispersionDelay + lowpassDelay);
		/*

		 pin - 0-|--1 -- 2 bridge - soundboard
			   |
			   3
			 hammer

		  The hammer is imparted with an initial velocity.
		  The hammer velocity becomes an input load using the impedances at the junction.
		  The 0 & 1 strings carry the velocity wave.
		  The 0 string is terminated at an infinite impedance pin.
		  The 1 string interacts with the soundboard by inducing a load
		  There are no additional ingoing waves into the strings from either the hammer or the bridge

		*/
		DigitalWaveGuide & dwg1=pianoDWGs.dwgs[0];
		DigitalWaveGuide & dwg2=pianoDWGs.dwgs[1];
		DigitalWaveGuide & dwg3=pianoDWGs.dwgs[2];
		DigitalWaveGuide & dwg4=pianoDWGs.dwgs[3];
		dwg1.init(Z, del1,del1);
		dwg2.init(Z, del2, del3);
		dwg3.init(Zb, 0, 0);
		dwg4.init(Zh, 0, 0);
		dwg2.setDispersion(dispersion, lowpass, D);
		DigitalWaveGuide::connectLeftRight(dwg1, dwg2);
		DigitalWaveGuide::connectLeftRight(dwg2, dwg3);
		DigitalWaveGuide::connectLeftRight(dwg1, dwg4);
		DigitalWaveGuide::connectLeftLeft(dwg2, dwg4);
		dwg1.initAlphaCoefficients();
		dwg2.initAlphaCoefficients();
		dwg3.initAlphaCoefficients();
		dwg4.initAlphaCoefficients();
	}
	u_sample PianoModel::PianoKeyGo(PianoKey & key){
		u_sample Zx2=2 * key.Z;
		std::vector<u_sp<PianoDwgs>> & keyString=key.string;
		u_sample keyStringCount=key.string.size();
		u_sample facZ=Zx2 / (key.Z * keyStringCount + key.ZBridge);
		int k;
		u_sample hammerLoad=0;
		if(key.loadState < 2){
			u_sample vstring=0.0f;
			for(k=0;k < keyStringCount;k++){
				DigitalWaveGuide * waveGuides=keyString[k]->dwgs;
				vstring+=waveGuides[1].leftNode.signals[0] + waveGuides[0].rightNode.signals[1];
			}
			hammerLoad=key.hammer->load(vstring / keyStringCount) / (Zx2);
			if(hammerLoad == 0 && key.loadState == 0){
				key.loadState=1;
			} else if(hammerLoad == 0 && key.loadState == 1){
				key.loadState=2;
			}
		}
		u_sample stringLoad=0;
		for(k=0;k < keyStringCount;k++){
			DigitalWaveGuide * waveGuides=keyString[k]->dwgs;
			waveGuides[3].leftNode.load=hammerLoad;
			waveGuides[0].processDelay();
			waveGuides[1].processDelay();
			stringLoad+=waveGuides[1].rightNode.signals[1];
		}
		stringLoad*=facZ;
		u_sample output=0.0f;
		for(k=0;k < keyStringCount;k++){
			DigitalWaveGuide * waveGuides=keyString[k]->dwgs;
			waveGuides[2].leftNode.load=stringLoad;
			waveGuides[0].calculateLoad();
			waveGuides[1].calculateLoad();
			waveGuides[2].calculateLoad();
			waveGuides[0].updateSignals();
			waveGuides[1].updateSignals();
			waveGuides[2].updateSignals();
			output+=waveGuides[2].leftNode.signals[1];
		}
		return output;
	}
	void PianoModel::PianoKeyTrigger(PianoKey & key, s_note_vel v){
		for(u_index i=0; i < key.string.size();i++){
			PianoDwgs & dwgs=*key.string[i];
			DigitalWaveGuide & dwg=dwgs.dwgs[1];
			dwg.damper=1;
		}
		key.hammer->trigger(v * key.weight);
	}
	void PianoModel::PianoKeyDamper(PianoKey & key){
		for(u_index i=0; i < key.string.size();i++){
			PianoDwgs & dwgs=*key.string[i];
			DigitalWaveGuide & dwg=dwgs.dwgs[1];
			dwg.damper=Util::clamp01(0.05 - key.offTimePassed) * 20;
		}
	}
}