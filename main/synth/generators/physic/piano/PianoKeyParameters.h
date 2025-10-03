#pragma once
#include "SimpleSynth.h"

namespace yzrilyzr_simplesynth{
	EBCLASS(PianoKeyParameters){
	public:
	s_note_id noteID=0;
	u_freq frequency=0.0;
	u_sample_rate sampleRate=0;
	u_sample outputVolume=0.0;//输出音量
	u_sample weight=0.0;
	u_sample minr=0.0;
	u_sample maxr=0.0;
	u_sample amprl=0.0;
	u_sample amprr=0.0;
	u_sample minL=0.0;
	u_sample maxL=0.0;
	u_sample ampLl=0.0;
	u_sample ampLr=0.0;
	u_sample mult_radius_core_string;//核心弦的半径
	u_sample mult_density_string;//弦密度
	u_sample mult_modulus_string;// 弦杨氏模量
	u_sample mult_impedance_bridge;//琴桥阻抗
	u_sample mult_impedance_hammer=0.0;//琴锤阻抗
	u_sample mult_mass_hammer=0.0;//琴锤质量
	u_sample mult_force_hammer=0.0;//锤击力度
	u_sample mult_hysteresis_hammer=0.0;//琴锤滞留
	u_sample mult_stiffness_exponent_hammer=0.0;//琴锤刚度
	u_sample position_hammer=0.0;//琴锤位置
	u_sample mult_loss_filter=0.0;
	u_sample detune=0.0;// 弦之间的音高差
	/**
	 * 1 {@link  StulovHammer}
	 * 2 {@link  BanksHammer}
	 */
	int hammer_type=0;
	};
}
